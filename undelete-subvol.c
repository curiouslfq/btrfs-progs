// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Fujitsu.  All rights reserved.
 */

#include "ctree.h"
#include "disk-io.h"
#include "transaction.h"
#include "messages.h"

/*
 * Determines whether the subvolume is intact, according to the drop_progress
 * of the root item specified by @subvol_id.
 *
 * Return true if the subvolume is intact, otherwise return false.
 */
static bool is_subvol_intact(struct btrfs_fs_info *fs_info, u64 subvol_id)
{
	struct btrfs_key key;
	struct btrfs_root *root;

	key.objectid = subvol_id;
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = -1;

	root = btrfs_read_fs_root(fs_info, &key);
	if (IS_ERR(root))
		return false;

	/* the subvolume is intact if the objectid of drop_progress == 0 */
	return !btrfs_disk_key_objectid(&root->root_item.drop_progress);
}

/*
 * Clear BTRFS_ROOT_SUBVOL_DEAD flag.
 *
 * @subvol_id	specify the root_item which will be modified.
 *
 * Return 0 if no error occurred.
 */
static int recover_dead_root(struct btrfs_trans_handle *trans, u64 subvol_id)
{
	struct btrfs_fs_info *fs_info = trans->fs_info;
	struct btrfs_root *root = fs_info->tree_root;
	struct btrfs_key key;
	struct btrfs_path path;
	struct extent_buffer *leaf;
	struct btrfs_root_item root_item;
	u64 root_flags;
	u64 offset;
	int ret;

	key.objectid = subvol_id;
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = 0;

	btrfs_init_path(&path);
	ret = btrfs_search_slot(trans, root, &key, &path, 0, 0);
	if (ret) {
		error("couldn't find ROOT_ITEM for %llu failed: %d",
				subvol_id, ret);
		goto out;
	}

	leaf = path.nodes[0];

	offset = btrfs_item_ptr_offset(leaf, path.slots[0]);
	read_extent_buffer(leaf, &root_item, offset, sizeof(root_item));

	/* Clear BTRFS_ROOT_SUBVOL_DEAD */
	root_flags = btrfs_root_flags(&root_item);
	btrfs_set_root_flags(&root_item,
			     root_flags & ~BTRFS_ROOT_SUBVOL_DEAD);

	write_extent_buffer(leaf, &root_item, offset, sizeof(root_item));
	btrfs_mark_buffer_dirty(leaf);

out:
	btrfs_release_path(&path);
	return ret;
}

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

/*
 * Recover a subvolume specified by subvol_id, and link it to the lost+found
 * directory.
 *
 * @subvol_id: specify the subvolume which will be linked, and also be the part
 * of the subvolume name.
 *
 * Return 0 if no error occurred.
 */
static int link_subvol_to_lostfound(struct btrfs_fs_info *fs_info,
				    u64 subvol_id)
{
	struct btrfs_trans_handle *trans;
	struct btrfs_root *root = fs_info->tree_root;
	struct btrfs_root *fs_root = fs_info->fs_root;
	char buf[BTRFS_NAME_LEN + 1] = {0}; /* 1 for snprintf null */
	char *dir_name = "lost+found";
	u64 lost_found_ino = 0;
	u32 mode = 0700;
	int ret;

	/*
	 * For link subvolume to lost+found,
	 * 2 for parent(256)'s dir_index and dir_item
	 * 2 for lost+found dir's inode_item and inode_ref
	 * 2 for lost+found dir's dir_index and dir_item for the subvolume
	 * 2 for the subvolume's root_ref and root_backref
	 */
	trans = btrfs_start_transaction(fs_root, 8);
	if (IS_ERR(trans)) {
		error("unable to start transaction");
		ret = PTR_ERR(trans);
		goto out;
	}

	/* Create lost+found directory */
	ret = btrfs_mkdir(trans, fs_root, dir_name, strlen(dir_name),
			  BTRFS_FIRST_FREE_OBJECTID, &lost_found_ino,
			  mode);
	if (ret < 0) {
		error("failed to create '%s' dir: %d", dir_name, ret);
		goto out;
	}

	/* Link the subvolume to lost+found directory */
	snprintf(buf, BTRFS_NAME_LEN + 1, "sub%llu", subvol_id);
	ret = btrfs_link_subvol(trans, fs_root, buf, subvol_id, lost_found_ino,
				false);
	if (ret) {
		error("failed to link the subvol %llu: %d", subvol_id, ret);
		goto out;
	}

	/* Clear root flags BTRFS_ROOT_SUBVOL_DEAD */
	ret = recover_dead_root(trans, subvol_id);
	if (ret)
		goto out;

	/* Delete the orphan item after undeletion is completed. */
	ret = btrfs_del_orphan_item(trans, root, subvol_id);
	if (ret) {
		error("failed to delete the orphan_item for %llu: %d",
				subvol_id, ret);
		goto out;
	}

	ret = btrfs_commit_transaction(trans, fs_root);
	if (ret) {
		error("transaction commit failed: %d", ret);
		goto out;
	}

out:
	return ret;
}

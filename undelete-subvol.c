// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Fujitsu.  All rights reserved.
 */

#include "ctree.h"
#include "disk-io.h"

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

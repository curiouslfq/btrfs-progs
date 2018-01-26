/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 Fujitsu.  All rights reserved.
 */

#ifndef __BTRFS_UNDELETE_SUBVOLUME_H__
#define __BTRFS_UNDELETE_SUBVOLUME_H__

int btrfs_undelete_subvols(struct btrfs_fs_info *fs_info, u64 subvol_id);

#endif

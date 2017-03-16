/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Fujitsu.  All rights reserved.
 */

#ifndef __BTRFS_DEDUPE__
#define __BTRFS_DEDUPE__

/*
 * Dedup storage backend
 * On disk is persist storage but overhead is large
 * In memory is fast but will lose all its hash on umount
 */
#define BTRFS_DEDUPE_BACKEND_INMEMORY		0
#define BTRFS_DEDUPE_BACKEND_LAST		1

/* Dedup block size limit and default value */
#define BTRFS_DEDUPE_BLOCKSIZE_MAX	SZ_8M
#define BTRFS_DEDUPE_BLOCKSIZE_MIN	SZ_16K
#define BTRFS_DEDUPE_BLOCKSIZE_DEFAULT	SZ_128K

/* Default dedupe limit on number of hash */
#define BTRFS_DEDUPE_LIMIT_NR_DEFAULT	SZ_32K

/* Hash algorithm, only support SHA256 yet */
#define BTRFS_DEDUPE_HASH_SHA256		0

#endif

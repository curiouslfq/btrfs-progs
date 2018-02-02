#!/bin/bash
# Test undelete-subvol can recover the intact subvolume, and can skip the
# incomplete subvolume.
#
# For intact_subvolume.img it has 1 orphan root and it can be recovered.
# For subvolume_in_drop_progress.raw.xz it has 2 orphan roots while only
# one can be recovered.

source "$TOP/tests/common"

check_prereq btrfs
check_prereq btrfs-image

setup_root_helper

check_image()
{
	TEST_DEV="$1"

	run_check_stdout "$TOP/btrfs" rescue undelete-subvol "$TEST_DEV" \
		| grep -q "Recovered 1 deleted subvols" \
		|| _fail "failed to undelete subvol $image" >&2

	run_check "$TOP/btrfs" check "$TEST_DEV"

	# check whether the recovered image can be mounted normally
	run_check_mount_test_dev

	run_check_stdout $SUDO_HELPER "$TOP/btrfs" subvolume list "$TEST_MNT" \
		| grep -q "lost+found" \
		|| _fail "failed to find the recovered subvol $image" >&2

	run_check_umount_test_dev "$TEST_MNT"

	rm "$TEST_DEV"
}

check_all_images

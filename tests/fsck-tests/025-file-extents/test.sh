#!/bin/bash
# Confirm btrfs check can check file extents without causing false alert

source "$TOP/tests/common"

check_prereq btrfs
check_prereq mkfs.btrfs
check_global_prereq dd
check_global_prereq fallocate

setup_root_helper
prepare_test_dev 128M

# Do some write into a large prealloc range
# Lowmem mode can report missing csum due to wrong csum range
test_paritical_write_into_prealloc()
{
	run_check $SUDO_HELPER "$TOP/mkfs.btrfs" -f "$TEST_DEV"
	run_check_mount_test_dev

	run_check $SUDO_HELPER fallocate -l 128K "$TEST_MNT/file"
	sync
	run_check $SUDO_HELPER dd conv=notrunc if=/dev/zero of="$TEST_MNT/file" bs=1K count=64
	run_check_umount_test_dev
	run_check "$TOP/btrfs" check "$TEST_DEV"
}

# Inline compressed file extent
# Lowmem mode can cause silent error without any error message
# due to too restrict check on inline extent size
test_compressed_inline_extent()
{
	run_check $SUDO_HELPER "$TOP/mkfs.btrfs" -f "$TEST_DEV"
	run_check_mount_test_dev -o compress=lzo,max_inline=2048

	run_check $SUDO_HELPER dd conv=notrunc if=/dev/null of="$TEST_MNT/file" bs=1K count=1
	run_check_umount_test_dev
	run_check "$TOP/btrfs" check "$TEST_DEV"
}

test_paritical_write_into_prealloc
test_compressed_inline_extent

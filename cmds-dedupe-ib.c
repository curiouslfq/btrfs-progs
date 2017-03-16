// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Fujitsu.  All rights reserved.
 */

#include <getopt.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "ctree.h"
#include "ioctl.h"

#include "commands.h"
#include "utils.h"
#include "kerncompat.h"
#include "dedupe-ib.h"

static const char * const dedupe_ib_cmd_group_usage[] = {
	"btrfs dedupe-inband <command> [options] <path>",
	NULL
};

static const char dedupe_ib_cmd_group_info[] =
"manage inband(write time) de-duplication";

const struct cmd_group dedupe_ib_cmd_group = {
	dedupe_ib_cmd_group_usage, dedupe_ib_cmd_group_info, {
		NULL_CMD_STRUCT
	}
};

int cmd_dedupe_ib(int argc, char **argv)
{
	return handle_command_group(&dedupe_ib_cmd_group, argc, argv);
}

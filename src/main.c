/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/sys_log.h>

#include "dloader/dl_channel.h"
#include "dloader/dl_command.h"
#include "dloader/dl_packet.h"

#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN	"UNISOC"

void main(void)
{
	int ret;
	struct dl_ch *ch;
	printk("UNISOC fdl.\n");

	ch = dl_channel_init();
	if (ch == NULL) {
		printk("Init channel failed.\n");
		return;
	}

	dl_packet_init(ch);
	
	ret = dl_cmd_init();
	if (ret) {
		printk("Init command failed.\n");
		return;
	}

	printk("start download...\n");
}

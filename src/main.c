/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <uart.h>

#include "dloader/dl_channel.h"
#include "dloader/dl_cmd_common.h"

#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN	"UNISOC"

void main(void)
{
	int ret;

    ret = dl_channel_init();
	if(ret) {
		FDL_PRINT("Init channel failed.\n");
		return;
	}

	do_download();

	while(1) {}
}

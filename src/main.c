/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <uart.h>

#include "dloader/dl_channel.h"

#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN	"UNISOC"

#define UART_0			"UART_0"
struct device *uart_fdl_dev;

extern int do_download();

void main(void)
{
	struct FDL_ChannelHandler *dl_Channel;
	printk("Unisoc fdl\n");
	uart_fdl_dev = device_get_binding(UART_0);
	if (uart_fdl_dev == NULL) {
		printk("uart_fdl_dev is NULL\n");
	}

	dl_Channel = FDL_ChannelGet();
	dl_Channel->priv = uart_fdl_dev;

	do_download();

	while(1) {}
}

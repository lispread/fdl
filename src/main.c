/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>

#include <gpio.h>
#include <watchdog.h>
#include <device.h>
#include <console.h>
#include <flash.h>
#include <shell/shell.h>
#include <string.h>
#include <misc/util.h>
#include <uart.h>
#include <stdlib.h>
#include <logging/sys_log.h>

#include "uwp_hal.h"
#include "dloader/dl_channel.h"

#ifdef SYS_LOG_DOMAIN
#undef SYS_LOG_DOMAIN
#endif
#define SYS_LOG_DOMAIN	"UNISOC"

#define UART_0			"UART_0"
struct device *uart_fdl_dev;

extern int do_download();
static int sprd_read (struct FDL_ChannelHandler  *channel, const unsigned char *buf, unsigned int len)
{
	struct device *priv = (struct device*)channel->priv;

	return uart_fifo_read(priv,buf,len);
}

static char sprd_getChar (struct FDL_ChannelHandler  *channel)
{
    char ch;

    struct device *priv = (struct device*)channel->priv;
	uart_poll_in(priv,&ch);

    return ch;
}

static int sprd_getSingleChar (struct FDL_ChannelHandler  *channel)
{
    char ch;

    struct device *priv = (struct device*)channel->priv;
	uart_poll_in(priv,&ch);

    return ch;
}
static int sprd_write (struct FDL_ChannelHandler  *channel, const unsigned char *buf, unsigned int len)
{
	int i;
	struct device *priv = (struct device*)channel->priv;
	printf(" %x",0xaa);
	for (i=0;i<len;i++) {
    	printf(" %x",buf[i]);
	}
	printf(" %x\n",0x55);
	return uart_fifo_fill(priv,buf,len);
}

static int sprd_putChar (struct FDL_ChannelHandler  *channel, const unsigned char ch)
{
    struct device *priv = (struct device*)channel->priv;

	uart_poll_out(priv,ch);

    return 0;
}

FDL_ChannelHandler_T  gUart0Channel = {
    .Open = NULL,
    .Read = NULL,
    .GetChar = sprd_getChar,
    .GetSingleChar = sprd_getSingleChar,
    .Write = sprd_write,
    .PutChar = sprd_putChar,
    .SetBaudrate =NULL,
    .DisableHDLC = NULL,
    .Close = NULL,
    .priv = NULL,
};

void main(void)
{
	printk("UNISOC fdl.\n");
	uart_fdl_dev = device_get_binding(UART_0);
	if (uart_fdl_dev == NULL) {
		SYS_LOG_WRN("uart_fdl_dev is NULL %x\n",uart_fdl_dev);
	}
	gUart0Channel.priv = uart_fdl_dev;

	do_download();
}

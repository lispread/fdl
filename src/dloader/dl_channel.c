/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/sys_log.h>
#include <string.h>
#include <uart.h>

#include "dl_channel.h"
#include "dl_packet.h"

#define BOOT_FLAG_USB                   (0x5A)
#define BOOT_FLAG_UART1                 (0x6A)
#define BOOT_FLAG_UART0                 (0x7A)

#define UART_0	"UART_0"
struct device *uart_dev;

/******************************************************************************/
//  Description:    find a useable channel
//  Global resource dependence:
//  Author:         junqiang.wang
//  Note:
/******************************************************************************/

static int uart_write(struct dl_ch  *channel, const unsigned char *buf, unsigned int len)
{
	int i;
	struct device *priv = (struct device*)channel->priv;
	printk("uart write: %x",0xaa);
	for (i=0; i<len; i++) {
		printk(" %x",buf[i]);
	}
	printk(" %x\n",0x55);
	return uart_fifo_fill(priv,buf,len);
}

static int uart_put_char(struct dl_ch  *channel, const unsigned char ch)
{
	struct device *priv = (struct device*)channel->priv;
 
	printk("uart put: %c(0x%02x)\n", ch, ch);
	uart_poll_out(priv,ch);

	return 0;
}
static struct dl_ch uart_channel = {
	.write = uart_write,
	.put_char = uart_put_char,
};

#define MAX_BUF_LEN	(0x4000)
static u8_t buf[MAX_BUF_LEN];
static struct dl_pkt recv_pkt;

void uart_rx_handle(struct dl_ch *ch)
{
	struct device *dev = (struct device *)ch->priv;

	static u32_t offset = 0;
	bool pkt_complete = false;
	u32_t len;
	u32_t i;

	len = uart_fifo_read(dev, &buf[offset], MAX_BUF_LEN);
	offset += len;

	for (i = 1; i < offset; i++) {
		if (buf[i] == 0x7e) {
			pkt_complete = true;
			break;
		}
	}

	if (!pkt_complete) return;

	printk("uart read %d bytes: \n", offset);
	if (offset < 32) {
		for (i = 0; i < offset; i++) {
			printk("%02x ", buf[i]);
			if ((i > 0) && (i % 16 == 0)) printk("\n");
		}
		printk("\n");
	}

	/* remove 0x7E */
	offset -= 2;

	recv_pkt.data_size = offset;
	memcpy(&recv_pkt.body, buf + 1, offset);

	dl_pkt_handler(&recv_pkt);

	offset = 0;
}

static void uart_callback(void *user_data)
{
	struct dl_ch *uart_ch = (struct dl_ch *)user_data;
	struct device *dev = (struct device *)uart_ch->priv;

	uart_irq_update(dev);

	if (uart_irq_rx_ready(dev)) {
		uart_rx_handle(uart_ch);
	}
}

struct dl_ch *dl_channel_get(void)
{
	return &uart_channel;
}

struct dl_ch *dl_channel_init(void)
{
	struct dl_ch *uart_ch = &uart_channel;

	uart_dev = device_get_binding(UART_0);
	if (uart_dev == NULL) {
		SYS_LOG_WRN("uart_dev is NULL %x\n",uart_dev);
		return NULL;
	}

	uart_ch->priv = uart_dev;

	uart_irq_callback_user_data_set(uart_dev, uart_callback,
			(void *)uart_ch);
	uart_irq_rx_enable(uart_dev);

	return uart_ch;
}

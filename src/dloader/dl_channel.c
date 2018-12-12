#include <zephyr.h>
#include <string.h>
#include <misc/util.h>
#include <uart.h>
#include <stdlib.h>
#include "dl_channel.h"

#define BOOT_FLAG_USB                   (0x5A)
#define BOOT_FLAG_UART1                 (0x6A)
#define BOOT_FLAG_UART0                 (0x7A)
#define UART_0			"UART_0"

/******************************************************************************/
//  Description:    find a useable channel
//  Global resource dependence:
//  Author:         
//  Note:
/******************************************************************************/
struct device *uart_fdl_dev;

static int sprd_read (struct FDL_ChannelHandler  *channel, unsigned char *buf, unsigned int len)
{
	struct device *priv = (struct device*)channel->priv;

	return uart_fifo_read(priv,buf,len);
}

static char sprd_getChar (struct FDL_ChannelHandler  *channel)
{
    char ch;
    struct device *priv = (struct device*)channel->priv;
	
	while(uart_poll_in(priv,&ch) != 0){
	};

    return ch;
}

static int sprd_getSingleChar (struct FDL_ChannelHandler  *channel)
{
    char ch;
    struct device *priv = (struct device*)channel->priv;
	
	while(uart_poll_in(priv,&ch) != 0){
	};

    return ch;
}
static int sprd_write (struct FDL_ChannelHandler  *channel, const unsigned char *buf, unsigned int len)
{
	struct device *priv = (struct device*)channel->priv;

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
    .Read = sprd_read,
    .GetChar = sprd_getChar,
    .GetSingleChar = sprd_getSingleChar,
    .Write = sprd_write,
    .PutChar = sprd_putChar,
    .SetBaudrate =NULL,
    .DisableHDLC = NULL,
    .Close = NULL,
    .priv = NULL,
};
struct FDL_ChannelHandler *FDL_ChannelGet()
{
    return &gUart0Channel;
}

int dl_channel_init()
{
	struct FDL_ChannelHandler *dl_Channel;
	printk("Unisoc uart\n");
	uart_fdl_dev = device_get_binding(UART_0);
	if (uart_fdl_dev == NULL) {
		printk("uart_fdl_dev is NULL\n");
        return -1;
	}

	dl_Channel = FDL_ChannelGet();
	dl_Channel->priv = uart_fdl_dev;

    return 0;
}

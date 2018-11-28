#include <zephyr.h>
#include <logging/sys_log.h>
#include "dl_channel.h"

#define BOOT_FLAG_USB                   (0x5A)
#define BOOT_FLAG_UART1                 (0x6A)
#define BOOT_FLAG_UART0                 (0x7A)


/******************************************************************************/
//  Description:    find a useable channel
//  Global resource dependence:
//  Author:         junqiang.wang
//  Note:
/******************************************************************************/
extern struct FDL_ChannelHandler gUart0Channel, gUart1Channel;
struct FDL_ChannelHandler gUSBChannel;

struct FDL_ChannelHandler *FDL_ChannelGet()
{
    unsigned int bootMode = 0;

    struct FDL_ChannelHandler *channel;
	bootMode = BOOT_FLAG_UART0;
#ifdef CONFIG_UART_DOWNLOAD
	bootMode = BOOT_FLAG_UART1;
#endif
//	__udelay(100000000);

    switch (bootMode)
    {
        case BOOT_FLAG_UART1:
            channel = &gUart1Channel;
            channel -> Open(channel, 115200);
            break;
        case BOOT_FLAG_UART0:
            channel = &gUart0Channel;
            //channel -> Open(channel, 115200);
            //channel -> SetBaudrate(channel, 115200);
            break;
    }
    return channel;
}
struct FDL_ChannelHandler *FDL_ChannelPrintGet()
{
	return NULL;
}
struct FDL_ChannelHandler *FDL_USBChannel()
{
	return &gUSBChannel;
}


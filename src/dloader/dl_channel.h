/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _DL_CHANNEL_H_
#define _DL_CHANNEL_H_

struct dl_ch
{
    int (*open) (struct dl_ch *channel, unsigned int  baudrate);
    int (*read) (struct dl_ch *channel, const unsigned char *buf, unsigned int  len);
    char (*get_char) (struct dl_ch *channel);
    int (*get_sigle_char) (struct dl_ch *channel);
    int (*write) (struct dl_ch *channel, const unsigned char *buf, unsigned int  len);
    int (*put_char) (struct dl_ch *channel, const unsigned char ch);
    int (*set_baudrate) (struct dl_ch *channel, unsigned int  baudrate);
    int (*disable_hdlc) (struct dl_ch *channel, int  disabled);
    int (*close) (struct dl_ch *channel);
    void *priv;
};

struct dl_ch *dl_channel_get(void);
struct dl_ch *dl_channel_init(void);
//struct dl_ch *FDL_USBChannel(void);

#endif

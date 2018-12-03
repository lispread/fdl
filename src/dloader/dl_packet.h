/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _DL_PACKET_H_
#define _DL_PACKET_H_

#define MAX_PKT_SIZE    0xA00 /* Just data field of a packet excluding header and checksum */
#define PACKET_HEADER_SIZE   4   // (type + size)

#define PACKET_MAX_NUM    3

typedef enum
{
    PKT_NONE = 0,
    PKT_HEAD,
    PKT_GATHER,
    PKT_RECV,
    PKT_ERROR
} pkt_flag_s;

struct pkt_body {
    unsigned short  type;
    unsigned short  size;
    unsigned char   content[MAX_PKT_SIZE];
};

struct dl_pkt {
    struct dl_pkt *next;
    int	pkt_state;
    int	data_size;
    int	ack_flag;
    struct pkt_body body;
};

struct dl_cmd {
	u32_t type;
	int (*handle)(struct dl_pkt *pkt, void *arg);
};

struct dl_ch;

struct dl_pkt *dl_pkt_alloc(void);
void dl_packet_free (struct dl_pkt *pkt);
void dl_packet_send (struct dl_pkt *pkt);
void dl_send_ack (u32_t cmd);
int dl_pkt_handler(u8_t *buf, u32_t len);
void dl_packet_init(struct dl_ch *ch);

#endif  // PACKET_H


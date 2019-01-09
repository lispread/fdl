/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authors: Justin Wang <justin.wang@spreadtrum.com>
 */

#ifndef PACKET_H
#define PACKET_H

#include "dl_cmd_common.h"

#define MAX_PKT_SIZE    0x4000 /* Just data field of a packet excluding header and checksum */
//#define MAX_PKT_SIZE    0x10000 /* Just data field of a packet excluding header and checksum */
#define PACKET_HEADER_SIZE   4   // (type + size)

#define PACKET_MAX_NUM    3

#define	FDL_DEFAULT_HDLC_FORMAT	0
#define	FDL_NONE_HDLC_FORMAT	1

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
#ifndef FPGA_TRACE_DOWNLOAD
    unsigned char   content[ MAX_PKT_SIZE ];
#else
    char*    content;
#endif
};

typedef struct dl_packet {
    struct dl_packet *next;
    int	pkt_state;
    int	data_size;
    int	ack_flag;
    struct pkt_body body;
}dl_packet_t;

struct dl_cmd {
	enum dl_cmd_type type;
	int (*handle)(struct dl_packet *pkt, void *arg);
};

dl_packet_t *FDL_MallocPacket (void);
void dl_packet_init (void);
struct dl_packet* dl_get_packet (void);
void dl_free_packet (struct dl_packet *pkt);
void dl_send_packet (struct dl_packet *pkt);
void dl_send_ack (dl_cmd_type_t  pkt_type);
void FDL_DisableHDLC (int disabled);
int * FDL_get_DisableHDLC(void);

#endif  // PACKET_H


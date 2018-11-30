/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <logging/sys_log.h>

#include "dl_packet.h"
#include "dl_stdio.h"
#include "dl_crc.h"
#include "dl_channel.h"
#include "dl_command.h"

int act_as_romcode;

int message_format;

#define	FDL_DEFAULT_HDLC_FORMAT	0
#define	FDL_NONE_HDLC_FORMAT	1

struct dl_ch *fdl_ch;
struct dl_ch *gFdlPrintChannel;

static struct dl_pkt  packet[ PACKET_MAX_NUM ];

static struct dl_pkt *packet_free_list;
static struct dl_pkt *packet_completed_list;
static struct dl_pkt *packet_receiving;

void print_addr(uint32_t addr)
{
	unsigned char temp;

	temp = (unsigned char)(addr>>24);
	gFdlPrintChannel->put_char(gFdlPrintChannel,temp);
	temp = (unsigned char)(addr>>16);
	gFdlPrintChannel->put_char(gFdlPrintChannel,temp);
	temp = (unsigned char)(addr>>8);
	gFdlPrintChannel->put_char(gFdlPrintChannel,temp);
	temp = (unsigned char)addr;
	gFdlPrintChannel->put_char(gFdlPrintChannel,temp);

	gFdlPrintChannel->put_char(gFdlPrintChannel,0x77);

}
void dl_packet_init(struct dl_ch *ch)
{
	uint32_t i = 0;

	message_format = FDL_DEFAULT_HDLC_FORMAT;
	packet_free_list = &packet[0];

	for (i = 0; i < PACKET_MAX_NUM; i++) {
		packet[i].next   = &packet[i+1];
	}
	packet[PACKET_MAX_NUM-1].next = NULL;

	packet_completed_list = NULL;
	packet_receiving      = NULL;
	fdl_ch = ch;
	printk("fdl_ch: %p\n",fdl_ch);
}

struct dl_pkt *dl_pkt_alloc (void)
{
	struct dl_pkt   *tmp_ptr = NULL;

	if (NULL != packet_free_list) {
		tmp_ptr = packet_free_list;
		packet_free_list = tmp_ptr->next;

		tmp_ptr->next       = NULL;
		tmp_ptr->pkt_state  = PKT_NONE;
		tmp_ptr->ack_flag   = 0;
		tmp_ptr->data_size  = 0;
	}

	return tmp_ptr;
}

void dl_packet_free(struct dl_pkt *ptr)
{
	ptr->next = packet_free_list;
	packet_free_list = ptr;
}

void dl_pkt_write (const void *buf, int len)
{
	fdl_ch->write (fdl_ch, buf, len);
}

uint32_t  FDL_DataProcess (struct dl_pkt *pkt_src, struct dl_pkt *pkt_dest)
{
	unsigned short  crc, size;
	int32_t           write_len;
	int32_t           send_len;
	int32_t           i;
	uint8_t           curval;

	uint8_t          *des_ptr = NULL;
	uint8_t          *src_ptr = NULL;

	size = pkt_src->body.size;
	write_len = size + sizeof (unsigned short) + PACKET_HEADER_SIZE;
	src_ptr = (uint8_t *) &pkt_src->body;

	pkt_src->body.size = EndianConv_16 (pkt_src->body.size);
	pkt_src->body.type = EndianConv_16 (pkt_src->body.type);

	/*src CRC calculation*/
	if (1 == act_as_romcode) {
		crc = crc_16_l_calc((char *)&pkt_src->body,
				size + PACKET_HEADER_SIZE);
	} else {
		crc = frm_chk((const unsigned short *)&pkt_src->body,
				size + PACKET_HEADER_SIZE);
		crc = EndianConv_16 (crc);
	}

	pkt_src->body.content[ size ] = (crc >> 8) & 0xFF;
	pkt_src->body.content[ size+1 ] = (crc)    & 0xFF;

	/*******************************************
	 *    des data preparation
	 ********************************************/

	des_ptr = (uint8_t *) &pkt_dest->body;
	/*head flag*/
	* (des_ptr++) = HDLC_FLAG;
	send_len = 1;

	if (message_format == FDL_NONE_HDLC_FORMAT) {
		memcpy(des_ptr,src_ptr,write_len);
		send_len += write_len;
		des_ptr += write_len;
	} else {
		/*middle part process*/
		for (i = 0; i < write_len; i++) {
			curval = * (src_ptr + i);

			if ( (HDLC_FLAG == curval) || (HDLC_ESCAPE == curval)) {
				* (des_ptr++) = HDLC_ESCAPE;
				* (des_ptr++) = ~HDLC_ESCAPE_MASK & curval;
				send_len++;
			} else {
				* (des_ptr++) = curval;
			}

			send_len++;
		}
	}
	/*end flag*/
	* (des_ptr++) = HDLC_FLAG;
	send_len++;

	return send_len;
}
/******************************************************************************
 * FDL_SendPacket
 ******************************************************************************/
void dl_packet_send(struct dl_pkt *pkt)
{
	int32_t           send_len;   /*length after encode*/
	struct dl_pkt       *tmp_pkt = NULL;

	// send a ACK packet to notify PC that we are ready.
	tmp_pkt = dl_pkt_alloc();

	if (NULL == tmp_pkt) {

		dl_packet_free (packet_receiving);
		tmp_pkt = dl_pkt_alloc();
	}

	send_len = FDL_DataProcess (pkt, tmp_pkt);

	dl_pkt_write ( (char *) (& (tmp_pkt->body)), send_len);

	dl_packet_free (tmp_pkt);
}

void dl_send_ack(u32_t cmd)
{

	unsigned int ack_packet_src[8];
	unsigned int ack_packet_dst[8];
	struct dl_pkt *pkt = (struct dl_pkt *) ack_packet_src;

	int32_t           send_len;   /*length after encode*/
	struct dl_pkt       *tmp_pkt = NULL;

	pkt->body.type = cmd;
	pkt->body.size = 0;

	tmp_pkt = (struct dl_pkt *) ack_packet_dst;

	send_len = FDL_DataProcess (pkt, tmp_pkt);
	//printk("pkt->body.type=%d\n" , pkt->body.type);
	//printk("before send_len = %d\n" , send_len);
	dl_pkt_write ( (char *) (& (tmp_pkt->body)), send_len);
	//printk("send_len = %d" , send_len);

}

struct dl_cmd dl_cmds[] = {
	{BSL_CMD_CONNECT, dl_cmd_write_connect},
	{BSL_CMD_START_DATA, dl_cmd_write_start},
	{BSL_CMD_MIDST_DATA, dl_cmd_write_midst},
	{BSL_CMD_END_DATA, dl_cmd_write_end},
	{0, NULL}
};

void dl_pkt_handler(struct dl_pkt *pkt)
{
	u32_t i;
	u32_t cmds_cnt = sizeof(dl_cmds) / sizeof(struct dl_cmd);
	struct dl_cmd *cmd;

	pkt->body.type = EndianConv_16(pkt->body.type);
	pkt->body.size = EndianConv_16(pkt->body.size);

	printk("pkt type: %d.\n", pkt->body.type);
	for (i = 0, cmd = dl_cmds; i < cmds_cnt; i++, cmd++) {
		if ((pkt->body.type == cmd->type) && (cmd->handle != NULL)) {
			cmd->handle(pkt, NULL);
			return;
		}
	}

	/* cannot found the cmd in cmdlist */
	dl_send_ack(BSL_UNSUPPORTED_CMD);
}

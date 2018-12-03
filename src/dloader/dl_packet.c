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

struct dl_ch *fdl_ch;
struct dl_ch *gFdlPrintChannel;

static struct dl_pkt  packet[PACKET_MAX_NUM];

static struct dl_pkt *packet_free_list;

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

	packet_free_list = &packet[0];

	for (i = 0; i < PACKET_MAX_NUM; i++) {
		packet[i].next   = &packet[i+1];
	}
	packet[PACKET_MAX_NUM-1].next = NULL;

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

void dl_packet_send(struct dl_pkt *pkt)
{
	u32_t send_len;
	u8_t c = 0x7E;
	u16_t crc;
	u16_t *crc_ptr;

	u16_t size = pkt->body.size;
	send_len = PACKET_HEADER_SIZE + size + sizeof(u16_t);

	pkt->body.size = EndianConv_16(size);
	pkt->body.type = EndianConv_16(pkt->body.type);

	crc = frm_chk((const u16_t *)&pkt->body,
			size + PACKET_HEADER_SIZE);

	crc_ptr = (u16_t *)(pkt->body.content + size);
	*crc_ptr = crc;

	dl_pkt_write(&c, 1);
	dl_pkt_write((u8_t *)&(pkt->body), send_len);
	dl_pkt_write(&c, 1);

	dl_packet_free(pkt);
}

void dl_send_ack(u32_t cmd)
{
	struct dl_pkt *pkt;

	pkt = dl_pkt_alloc();
	if (pkt == NULL) {
		printk("Alloc packet failed!\n");
		dl_cmd_reply(OPERATE_SYSTEM_ERROR);
		return;
	}

	pkt->body.type = cmd;
	pkt->body.size = 0;

	dl_packet_send(pkt);
}

struct dl_cmd dl_cmds[] = {
	{BSL_CMD_CONNECT, dl_cmd_connect},
	{BSL_CMD_START_DATA, dl_cmd_start},
	{BSL_CMD_MIDST_DATA, dl_cmd_midst},
	{BSL_CMD_END_DATA, dl_cmd_end},
	{0, NULL}
};

int dl_pkt_handler(u8_t *buf, u32_t len)
{
	u32_t i;
	u32_t cmds_cnt = sizeof(dl_cmds) / sizeof(struct dl_cmd);
	struct dl_cmd *cmd;
	struct dl_pkt *pkt;

	pkt = dl_pkt_alloc();
	if (pkt == NULL) {
		printk("Alloc packet failed!\n");
		dl_send_ack(BSL_REP_OPERATION_FAILED);
		return -1;
	}

	memcpy(&(pkt->body), buf, len);
	pkt->data_size = len;

	pkt->body.type = EndianConv_16(pkt->body.type);
	pkt->body.size = EndianConv_16(pkt->body.size);

	printk("pkt type: %d.\n", pkt->body.type);
	for (i = 0, cmd = dl_cmds; i < cmds_cnt; i++, cmd++) {
		if ((pkt->body.type == cmd->type) && (cmd->handle != NULL)) {
			cmd->handle(pkt, NULL);
			dl_packet_free(pkt);
			return 0;
		}
	}

	dl_packet_free(pkt);
	/* cannot found the cmd in cmdlist */
	return dl_cmd_reply(OPERATE_INVALID_ADDR);
}

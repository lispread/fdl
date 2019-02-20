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
 * Authors:  <@spreadtrum.com>
 */

#include <zephyr.h>
#include <string.h>
#include "dl_packet.h"
#include "dl_cmd_common.h"
#include "dl_cmd_proc.h"

typedef struct _DA_INFO_T_
{
	uint32_t   dwVersion;
	bool    bDisableHDLC; //0: Enable hdl; 1:Disable hdl
}DA_INFO_T;

struct dl_cmd dl_cmds[] = {
	{BSL_CMD_CONNECT, dl_cmd_write_connect},
	{BSL_CMD_START_DATA, dl_cmd_write_start},
	{BSL_CMD_MIDST_DATA, dl_cmd_write_midst},
	{BSL_CMD_END_DATA, dl_cmd_write_end},
	{0, NULL}
};

void dl_cmd_handler(void)
{
	u32_t i;
	u32_t cmds_cnt = sizeof(dl_cmds) / sizeof(struct dl_cmd);
	struct dl_cmd *cmd;
	struct dl_packet *pkt;
	FDL_PRINT("%s:enter\n", __FUNCTION__);
	for (;;) {
		pkt = dl_get_packet();
		//FDL_PRINT("%x %x\n ", pkt->body.type,pkt->body.size);
		pkt->body.type = (pkt->body.type >> 8 | pkt->body.type << 8);
		pkt->body.size = (pkt->body.size >> 8 | pkt->body.size << 8);
		for (i = 0, cmd = dl_cmds; i < cmds_cnt; i++, cmd++) {
			if ((pkt->body.type == cmd->type) && (cmd->handle != NULL)) {
				cmd->handle(pkt, NULL);
				dl_free_packet(pkt);
				break;
			}
		}
		/* cannot found the cmd in cmdlist */
		if (NULL == cmd->handle){
			dl_send_ack(BSL_UNSUPPORTED_CMD);
			dl_free_packet(pkt);
		}
	}
}

const char fdl_version_string[] = {"SPRD3"};

int do_download()
{
	dl_packet_t ack_packet;
	DA_INFO_T Da_Info;

	FDL_PRINT("%s:enter\n", __FUNCTION__);
	dl_packet_init ();

	/* uart download doesn't supoort disable hdlc, so need check it */
	if (FDL_get_DisableHDLC() == NULL) {
	        ack_packet.body.type = BSL_REP_VER;
		ack_packet.body.size = sizeof(fdl_version_string);
		memcpy((u8_t *)ack_packet.body.content, fdl_version_string, sizeof(fdl_version_string));
		dl_send_packet(&ack_packet); }
	else {
		Da_Info.dwVersion = 1;
		Da_Info.bDisableHDLC = 1;
		ack_packet.body.type = BSL_INCOMPATIBLE_PARTITION;
		memcpy((u8_t *)ack_packet.body.content, (u8_t *)&Da_Info,sizeof(Da_Info));
		ack_packet.body.size = sizeof(Da_Info);
		dl_send_packet(&ack_packet);
	}

	/* enter command handler */
	dl_cmd_handler();
    return 0;
}


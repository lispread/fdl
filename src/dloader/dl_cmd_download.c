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

static struct dl_cmd *cmdlist = NULL;
static int index=0;

typedef struct _DA_INFO_T_
{
	uint32_t   dwVersion;
	bool    bDisableHDLC; //0: Enable hdl; 1:Disable hdl
}DA_INFO_T;


void dl_cmd_register(enum dl_cmd_type type,
	int (*handle)(struct dl_packet *pkt, void *arg),struct dl_cmd *data)
{
	struct dl_cmd *cmd = (void *)0;

	cmd = &data[index];

	index++;
	if (cmd) {
		cmd->type = type;
		cmd->handle = (void *)handle;
		cmd->next = cmdlist;
		cmdlist = cmd;
	}
	return;
}
void dl_cmd_handler(void)
{
	struct dl_cmd *cmd;
	struct dl_packet *pkt;
	printk("%s:enter\n", __FUNCTION__);
	for (;;) {
		pkt = dl_get_packet();
		//printk("%x %x\n ", pkt->body.type,pkt->body.size);
		pkt->body.type = (pkt->body.type >> 8 | pkt->body.type << 8);
		pkt->body.size = (pkt->body.size >> 8 | pkt->body.size << 8);
		for (cmd = cmdlist; cmd; cmd = cmd->next) {
			if(cmd->type != pkt->body.type)
				continue;

			cmd->handle((void *)pkt,NULL);
			dl_free_packet(pkt);

		/*
		 * in for(;;) loop, there should be no other log except handle func,
		 * otherwise common_raw_write() will be called all the time and will
		 * slow the download speed
		 */
			//write_log();
			break;
		}
		/* cannot found the cmd in cmdlist */
		if (NULL == cmd){
			dl_send_ack(BSL_UNSUPPORTED_CMD);
			dl_free_packet(pkt);
		}
	}
}

const char fdl_version_string[] = {"SPRD3"};

int do_download()
{
	struct dl_cmd cmd_store[5];
	dl_packet_t ack_packet;
	DA_INFO_T Da_Info;

	printk("%s:enter\n", __FUNCTION__);
	dl_packet_init ();

	/* register all cmd process functions */
	dl_cmd_register(BSL_CMD_CONNECT, dl_cmd_write_connect,cmd_store);
	dl_cmd_register(BSL_CMD_START_DATA, dl_cmd_write_start,cmd_store);
	dl_cmd_register(BSL_CMD_MIDST_DATA, dl_cmd_write_midst,cmd_store);
	dl_cmd_register(BSL_CMD_END_DATA, dl_cmd_write_end,cmd_store);

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


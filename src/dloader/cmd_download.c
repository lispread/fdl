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

#include <zephyr.h>
#include <logging/sys_log.h>
#include "packet.h"
#include "dl_cmd_def.h"
#include "dl_cmd_proc.h"

//#include <chipram_env.h>

#ifdef CONFIG_SOC_IWHALE2
#include <asm/arch/chip_id.h>
#endif


#ifdef CONFIG_USB_ENUM_IN_UBOOT
static int dl_times=0;
#endif
static struct dl_cmd *cmdlist = NULL;
//static struct dl_cmd cmd_store[5];
static int index=0;
extern int sprd_clean_rtc(void);
extern void usb_init (uint32_t autodl_mode);
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
	//cmd = (struct dl_cmd *)k_malloc(sizeof(struct dl_cmd));
	index++;
	printf("%s:malloc %d to %p mem\n", __FUNCTION__ , sizeof(struct dl_cmd), cmd);
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
	printf("%s:enter\n", __FUNCTION__);
	for (;;) {
		//printf("befor get_pkt\n");
		pkt = dl_get_packet();
		//printf("%x %x\n ", pkt->body.type,pkt->body.size);
		pkt->body.type = (pkt->body.type >> 8 | pkt->body.type << 8);
		pkt->body.size = (pkt->body.size >> 8 | pkt->body.size << 8);
		for (cmd = cmdlist; cmd; cmd = cmd->next) {
			if(cmd->type != pkt->body.type)
				continue;
			//printf("do packet %x %x %x\n ", cmd,cmd->handle,cmd->next);
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
		//printf("cmd %x %x\n ", pkt->body.type,pkt->body.size);
		/* cannot found the cmd in cmdlist */
		if (NULL == cmd){
			dl_send_ack(BSL_UNSUPPORTED_CMD);
			dl_free_packet(pkt);
		}
	}
}

#ifdef CONFIG_DL_POWER_CONTROL
extern void dl_power_control(void);
#endif

const char fdl_version_string[] = {"SPRD3"};

int do_download()
{
	struct dl_cmd cmd_store[5];
	dl_packet_t ack_packet;
	DA_INFO_T Da_Info;

	printf("%s:enter\n", __FUNCTION__);
	dl_packet_init ();
#ifdef CONFIG_NAND_BOOT
	fdl_ubi_dev_init();
#endif
	//sprd_clean_rtc();

#ifdef CONFIG_DL_POWER_CONTROL
	dl_power_control();
#endif
printf("%s:reg\n", __FUNCTION__);
	/* register all cmd process functions */
	dl_cmd_register(BSL_CMD_CONNECT, dl_cmd_write_connect,cmd_store);
	dl_cmd_register(BSL_CMD_START_DATA, dl_cmd_write_start,cmd_store);
	dl_cmd_register(BSL_CMD_MIDST_DATA, dl_cmd_write_midst,cmd_store);
	dl_cmd_register(BSL_CMD_END_DATA, dl_cmd_write_end,cmd_store);
	/*dl_cmd_register(BSL_CMD_READ_FLASH_START, dl_cmd_read_start);
	dl_cmd_register(BSL_CMD_READ_FLASH_MIDST, dl_cmd_read_midst);
	dl_cmd_register(BSL_CMD_READ_FLASH_END, dl_cmd_read_end);
	dl_cmd_register(BSL_ERASE_FLASH, dl_cmd_erase);
	dl_cmd_register(BSL_REPARTITION, dl_cmd_repartition);
	dl_cmd_register(BSL_CMD_NORMAL_RESET, dl_cmd_reboot);
	dl_cmd_register(BSL_CMD_POWER_DOWN_TYPE, dl_powerdown_device);
	//dl_cmd_register(BSL_CMD_READ_CHIP_TYPE, dl_cmd_mcu_read_chiptype);
	dl_cmd_register(BSL_CMD_READ_MCP_TYPE, dl_cmd_read_mcptype);
	dl_cmd_register(BSL_CMD_CHECK_ROOTFLAG, dl_cmd_check_rootflag);
#ifdef CONFIG_SPRD_UID
	dl_cmd_register(BSL_CMD_READ_UID, dl_cmd_get_chip_uid);
#else
#ifdef CONFIG_X86
	dl_cmd_register(BSL_CMD_READ_UID, dl_cmd_get_uid_x86);
#else
	dl_cmd_register(BSL_CMD_READ_UID, dl_cmd_get_uid);
#endif
#endif
	dl_cmd_register(BSL_CMD_END_PROCESS, dl_cmd_end_process);
	dl_cmd_register(BSL_CMD_READ_REFINFO, dl_cmd_read_ref_info);
	dl_cmd_register(BSL_CMD_DIS_HDLC, dl_cmd_disable_hdlc);
	dl_cmd_register(BSL_CMD_WRITE_DATETIME, dl_cmd_write_datetime);*/
	//usb_init(0);
printf("%s:register\n", __FUNCTION__);
	/* uart download doesn't supoort disable hdlc, so need check it */
	if (FDL_get_DisableHDLC() == NULL) {
	        ack_packet.body.type = BSL_REP_VER;
			//ack_packet.body.bytes = 0;
			//ack_packet.body.by = 0;
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
printf("%s:start\n", __FUNCTION__);
	/* enter command handler */
	dl_cmd_handler();
        return 0;
}


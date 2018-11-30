/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _DL_COMMAND_H_
#define _DL_COMMADN_H_
#include "dl_packet.h"

enum dl_cmd_type {
    BSL_PKT_TYPE_MIN = 0,                       /* the bottom of the DL packet type range */
    BSL_CMD_TYPE_MIN = BSL_PKT_TYPE_MIN,        /* 0x0 */

    /* Link Control */
    BSL_CMD_CONNECT = BSL_CMD_TYPE_MIN,         /* 0x0 */
    /* Data Download */
    /* the start flag of the data downloading */
    BSL_CMD_START_DATA,                         /* 0x1 */
    /* the midst flag of the data downloading */
    BSL_CMD_MIDST_DATA,                         /* 0x2 */
    /* the end flag of the data downloading */
    BSL_CMD_END_DATA,                           /* 0x3 */
    /* Execute from a certain address */
    BSL_CMD_EXEC_DATA,                          /* 0x4 */
    BSL_CMD_NORMAL_RESET,                       /* 0x5 */
    BSL_CMD_READ_FLASH,                         /* 0x6 */
    BSL_CMD_READ_CHIP_TYPE,                     /* 0x7 */
    BSL_CMD_LOOKUP_NVITEM,                      /* 0x8 */
    BSL_SET_BAUDRATE,                           /* 0x9 */
    BSL_ERASE_FLASH,                            /* 0xA */
    BSL_REPARTITION,                            /* 0xB */
    BSL_CMD_READ_MCP_TYPE=0xD,                 /* 0xD */
    BSL_CMD_READ_FLASH_START =0x10,/*0x10*/
    BSL_CMD_READ_FLASH_MIDST,		/*0x11*/
    BSL_CMD_READ_FLASH_END,		/*0x12*/
    BSL_CMD_OFF_CHG = 0x13,                     /* 0x13*/
    BSL_CMD_POWER_DOWN_TYPE = 0x17,             /* 0x17*/
    BSL_CMD_CHECK_ROOTFLAG = 0x19,               /* 0x19*/
    BSL_CMD_READ_UID = 0x1A,               /*0x1A*/
    BSL_CMD_TYPE_MAX,
    BSL_CMD_READ_REFINFO = 0x20,
    BSL_CMD_DIS_HDLC = 0x21,
    BSL_CMD_WRITE_DATETIME = 0x22,

    BSL_CMD_END_PROCESS = 0x7F,
    /* Start of the Command can be transmited by phone*/
    BSL_REP_TYPE_MIN = 0x80,

    /* The operation acknowledge */
    BSL_REP_ACK = BSL_REP_TYPE_MIN,         /* 0x80 */
    BSL_REP_VER,                            /* 0x81 */

    /* the operation not acknowledge */
    /* system  */
    BSL_REP_INVALID_CMD,                    /* 0x82 */
    BSL_REP_UNKNOW_CMD,                     /* 0x83 */
    BSL_REP_OPERATION_FAILED,               /* 0x84 */

    /* Link Control*/
    BSL_REP_NOT_SUPPORT_BAUDRATE,           /* 0x85 */

    /* Data Download */
    BSL_REP_DOWN_NOT_START,                 /* 0x86 */
    BSL_REP_DOWN_MULTI_START,               /* 0x87 */
    BSL_REP_DOWN_EARLY_END,                 /* 0x88 */
    BSL_REP_DOWN_DEST_ERROR,                /* 0x89 */
    BSL_REP_DOWN_SIZE_ERROR,                /* 0x8A */
    BSL_REP_VERIFY_ERROR,                   /* 0x8B */
    BSL_REP_NOT_VERIFY,                     /* 0x8C */

    /* Phone Internal Error */
    BSL_PHONE_NOT_ENOUGH_MEMORY,            /* 0x8D */
    BSL_PHONE_WAIT_INPUT_TIMEOUT,           /* 0x8E */

    /* Phone Internal return value */
    BSL_PHONE_SUCCEED,                      /* 0x8F */
    BSL_PHONE_VALID_BAUDRATE,               /* 0x90 */
    BSL_PHONE_REPEAT_CONTINUE,              /* 0x91 */
    BSL_PHONE_REPEAT_BREAK,                 /* 0x92 */

    BSL_REP_READ_FLASH,                     /* 0x93 */
    BSL_REP_READ_CHIP_TYPE,                 /* 0x94 */
    BSL_REP_LOOKUP_NVITEM,                  /* 0x95 */

    BSL_INCOMPATIBLE_PARTITION,             /* 0x96 */
    BSL_UNKNOWN_DEVICE,                     /* 0x97 */
    BSL_INVALID_DEVICE_SIZE,                /* 0x98 */

    BSL_ILLEGAL_SDRAM,                      /* 0x99 */
    BSL_WRONG_SDRAM_PARAMETER,              /* 0x9a */
    BSL_REP_READ_MCP_TYPE,                  /* 0x9b*/
    BSL_EEROR_CHECKSUM = 0xA0,
    BSL_CHECKSUM_DIFF,
    BSL_WRITE_ERROR,

    /*phone root return value*/
    BSL_PHONE_ROOTFLAG = 0xA7,
    BSL_REP_READ_CHIP_UID = 0xAB,
    BSL_REP_READ_REFINFO=0xB1,
    BSL_UNSUPPORTED_CMD = 0xFE,
    BSL_PKT_TYPE_MAX
};

#define SEND_ERROR_RSP(x)         \
    {                       \
        dl_send_ack(x);        \
        while(1);           \
    }



#define PARTITION_SIZE_LENGTH          (4)
#define MAX_PARTITION_NAME_SIZE   (36)
#define GAP_SIZE_LENGTH   (8)
#define PARTITION_SIZE_LENGTH_V1  (8)
#define REPARTITION_HEADER_MAGIC 0x3A726170
#define BIT64_DATA_LENGTH 0x58
#define BIT64_READ_MIDST_LENGTH 0x0C

#define REPARTITION_UNIT_LENGTH    (MAX_PARTITION_NAME_SIZE *2 + PARTITION_SIZE_LENGTH)
#define REPARTITION_UNIT_LENGTH_V1    (MAX_PARTITION_NAME_SIZE *2 +  PARTITION_SIZE_LENGTH_V1 + GAP_SIZE_LENGTH)
#define REF_INFO_OFF 0XFA000
#define CONFIG_SYS_LOAD_ADDR 0x120000
typedef enum OPERATE_STATUS {
	OPERATE_SUCCESS = 1,
	OPERATE_SYSTEM_ERROR,
	OPERATE_DEVICE_INIT_ERROR,
	OPERATE_INVALID_DEVICE_SIZE,
	OPERATE_INCOMPATIBLE_PART,
	OPERATE_INVALID_ADDR,
	OPERATE_INVALID_SIZE,
	OPERATE_WRITE_ERROR,
	OPERATE_CHECKSUM_DIFF,
	OPERATE_IGNORE
} OPERATE_STATUS;

typedef struct _REPARTITION_TABLE_INFO
{
	unsigned char version;
	unsigned char unit;
	unsigned char table_count;
	unsigned char reserved;
	unsigned int     table_tag;
	unsigned short     table_offset;
	unsigned short     table_size;
} REPARTITION_TABLE_INFO;

int dl_cmd_write_connect(struct dl_pkt * packet, void *arg);
int dl_cmd_write_start(struct dl_pkt * packet, void *arg);
int dl_cmd_write_midst(struct dl_pkt * packet, void *arg);
int dl_cmd_write_end(struct dl_pkt * packet, void *arg);
int dl_cmd_read_start(struct dl_pkt * packet, void *arg);
int dl_cmd_read_midst(struct dl_pkt * packet, void *arg);
int dl_cmd_read_end(struct dl_pkt * packet, void *arg);
int dl_cmd_erase(struct dl_pkt * packet, void *arg);
int dl_cmd_repartition(struct dl_pkt * pakcet, void *arg);
int dl_cmd_reboot (struct dl_pkt *pakcet, void *arg);
int dl_powerdown_device(struct dl_pkt *packet, void *arg);
int dl_cmd_read_mcptype(struct dl_pkt * packet, void *arg);
int dl_cmd_check_rootflag(struct dl_pkt * packet, void *arg);
int dl_cmd_get_uid(struct dl_pkt *packet, void *arg);
int dl_cmd_get_chip_uid(struct dl_pkt *packet, void *arg);
int dl_cmd_get_uid_x86(struct dl_pkt *packet, void *arg);
int dl_cmd_end_process(struct dl_pkt *packet, void *arg);
int dl_cmd_read_ref_info(struct dl_pkt *packet, void *arg);
int dl_cmd_disable_hdlc(struct dl_pkt *packet, void *arg);
int dl_cmd_write_datetime(struct dl_pkt *packet, void *arg);
int dl_cmd_init(void);
#endif /*DL_CMD_PROC_H */

#ifndef _DL_COMMON_
#define _DL_COMMON_

#include <zephyr.h>

/*loader_common.h has the same define*/
#define NV_HEAD_MAGIC	(0x00004e56)
#define NV_VERSION		(101)
#define NV_HEAD_LEN	(512)

#define NORFLASH_ADDRESS 0x2000000
#define BOOTLOADER_HEADER_OFFSET 0x20

#define PART_SAME 1
#define PART_DIFF   0
#define PART_FAIL   -1
#define PART_FATAL_ERR -2
#define _LITTLE_ENDIAN 1

typedef struct _NV_HEADER {
	uint32_t magic;
	uint32_t len;
	uint32_t checksum;
	uint32_t version;
} nv_header_t;

typedef enum dl_cmd_type {
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
}dl_cmd_type_t ;

#define SEND_ERROR_RSP(x)         \
    {                       \
        dl_send_ack(x);        \
        while(1);           \
    }

extern int do_download();
#endif

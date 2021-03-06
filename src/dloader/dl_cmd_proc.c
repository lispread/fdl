#include <zephyr.h>
#include <spi.h>
#include <hal_sfc.h>
#include <flash.h>
#include <string.h>

#include "dl_cmd_proc.h"
#include "dl_crc.h"
#include "dl_cmd_common.h"
#ifdef FDL_DEBUG
#include "mbedtls/md5.h"
#endif

static uint32_t start_addr=0;
static uint32_t recv_image_size=0;

static __inline dl_cmd_type_t convert_operate_status(int err)
{
	switch (err)
	{
		case OPERATE_SUCCESS:
			return BSL_REP_ACK;
		case OPERATE_INVALID_ADDR:
			return BSL_REP_DOWN_DEST_ERROR;
		case OPERATE_INVALID_SIZE:
			return BSL_REP_DOWN_SIZE_ERROR;
		case OPERATE_DEVICE_INIT_ERROR:
			return BSL_UNKNOWN_DEVICE;
		case OPERATE_INVALID_DEVICE_SIZE:
			return BSL_INVALID_DEVICE_SIZE;
		case OPERATE_INCOMPATIBLE_PART:
			return BSL_INCOMPATIBLE_PARTITION;
		case OPERATE_WRITE_ERROR:
			return BSL_WRITE_ERROR;
		case OPERATE_CHECKSUM_DIFF:
			return BSL_CHECKSUM_DIFF;

		default:
		    return BSL_REP_OPERATION_FAILED;
	}
}


static __inline void _send_reply(uint32_t err)
{
	dl_send_ack (convert_operate_status(err));
	return;
}

int _parse_repartition_header(u8_t * data, REPARTITION_TABLE_INFO * info, u8_t ** list)
{
	u8_t *  pointer = data;
	/*magic number must be "par:", otherwise it must be the old version packet(version 0)*/
	if (*(uint32_t*)data != REPARTITION_HEADER_MAGIC) {
		info->version = 0;
		/*default unit in version 0 is MB*/
		info->unit = 0;
		*list = data;
		return 0;
	}

	/*   header format:
	  *	|  magic(4Byte) | Version(1Byte) | Unit(1Byte) | table count(1Byte)|Reserved(1Byte) |
	  *	table tag(4) | table offset(2)| table size(2)|
	  */
	pointer += 4;
	info->version = *pointer;
	pointer += 1;
	info->unit = *pointer;
	pointer += 1;
	info->table_count = *pointer;
	pointer += 1;
	info->reserved = *pointer;
	pointer += 1;
	info->table_tag = *(unsigned int *)pointer;
	pointer += 4;
	info->table_offset = *(unsigned short *)pointer;
	pointer += 2;
	info->table_size = *(unsigned short *)pointer;
	pointer += 2;
	FDL_PRINT("%s: version(%d),unit(%d), table_count(%d), table_tag(%d), table_offset(%d), table_size(%d)\n",
		__FUNCTION__, info->version, info->unit, info->table_count, info->table_tag, info->table_offset,
		info->table_size);
	*list = pointer;
	return 0;
}

int dl_cmd_write_connect(dl_packet_t *packet, void *arg)
{
	FDL_PRINT("dl_cmd_write_connect \n");
	dl_send_ack(BSL_REP_ACK);
	return 0;
}

int dl_cmd_write_start (dl_packet_t *packet, void *arg)
{

    FDL_PRINT("start_addr %x\n",start_addr);
	memcpy(&start_addr, (void *)(packet->body.content), sizeof(start_addr));
	memcpy(&recv_image_size, (void *)(packet->body.content+sizeof(start_addr)), sizeof(start_addr));

	start_addr = EndianConv_32(start_addr);
	recv_image_size = EndianConv_32(recv_image_size);

	_send_reply(OPERATE_SUCCESS);
	return 0;
}


static char *s_iram_address = (char *)CONFIG_SYS_LOAD_ADDR;

int dl_cmd_write_midst(dl_packet_t *packet, void *arg)
{
	memcpy(s_iram_address, (void *)(packet->body.content), packet->body.size);
	s_iram_address += packet->body.size;
	_send_reply(OPERATE_SUCCESS);
	//FDL_PRINT("dl_cmd_write_midst %x ",s_iram_address); //every packet
	return 0;
}

int dl_cmd_write_end (dl_packet_t *packet, void *arg)
{
	int32_t  ret,op_res = OPERATE_SUCCESS;
	int32_t offset,earse_size;
	struct device *dev = device_get_binding(DT_FLASH_DEV_NAME);

#ifdef FDL_DEBUG
    mbedtls_md5_context md5_ctx,md5_ctx_f_one;
    int i;
	uint32_t image_len;
	unsigned char md5sum[16];

	image_len = s_iram_address - CONFIG_SYS_LOAD_ADDR;
#endif
	if (dev == NULL) {
		FDL_PRINT("Can not open device: %s.\n", DT_FLASH_DEV_NAME);
		_send_reply(OPERATE_SYSTEM_ERROR);
		return -1;
	}

	s_iram_address = (char *)CONFIG_SYS_LOAD_ADDR;
#ifdef FDL_DEBUG
    mbedtls_md5_init( &md5_ctx );
	if( ( ret = mbedtls_md5_starts_ret( &md5_ctx ) ) != 0 )
        FDL_PRINT("md5 start: .\n");
    if( ( ret = mbedtls_md5_update_ret( &md5_ctx, s_iram_address, image_len) ) != 0 )
        FDL_PRINT("md5 update: .\n");
 	if( ( ret = mbedtls_md5_finish_ret( &md5_ctx, md5sum ) ) != 0 )
		FDL_PRINT("md5 finish: .\n");

	for(i=0;i<16;i++) {
		FDL_PRINT("%x ",md5sum[i]);
	}
	FDL_PRINT("\n");
#endif

	offset = start_addr - NORFLASH_ADDRESS;

	flash_write_protection_set(dev, false);
	
	earse_size = ROUND_UP(recv_image_size,NORFLASH_SECTOR_SIZE);
	FDL_PRINT("Erase flash address: 0x%x size: 0x%x 0x%x.\n", offset,recv_image_size, earse_size);
	ret = flash_erase(dev, offset, earse_size);
	if (ret) {
		FDL_PRINT("Erase flash failed.\n");
		_send_reply(OPERATE_SYSTEM_ERROR);
		return -1;
	}

	FDL_PRINT("Write flash start...%x\n",offset);
	ret = flash_write(dev, offset, s_iram_address, recv_image_size);
	if (ret) {
		FDL_PRINT("wirte flash failed.\n");
		_send_reply(OPERATE_SYSTEM_ERROR);
		return -1;
	}
	FDL_PRINT("Write success.\n");
	flash_write_protection_set(dev, true);

	_send_reply(op_res);
#ifdef FDL_DEBUG
	mbedtls_md5_init( &md5_ctx_f_one );
	flash_read(dev,offset,s_iram_address, image_len);
	if( ( ret = mbedtls_md5_starts_ret( &md5_ctx_f_one ) ) != 0 )
        FDL_PRINT("md5 start: .\n");
	if( ( ret = mbedtls_md5_update_ret( &md5_ctx_f_one, s_iram_address, image_len ) ) != 0 )
        FDL_PRINT("md5 update: .\n");
 	if( ( ret = mbedtls_md5_finish_ret( &md5_ctx_f_one, md5sum ) ) != 0 )
		FDL_PRINT("md5 finish: .\n");

	for(i=0;i<16;i++) {
		FDL_PRINT("%x ",md5sum[i]);
	}
	FDL_PRINT("\n");
#endif

	return 0;
}

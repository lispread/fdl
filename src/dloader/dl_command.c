/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <string.h>
#include <logging/sys_log.h>

#include "dl_stdio.h"
#include "dl_command.h"

/*static root_stat_t root_stat;*/
extern void print_addr(uint32_t addr);
const char fdl_ver_str[] = {"SPRD3"};


#if 0
static void _decode_packet_data(struct dl_pkt *packet, u8_t * partition_name, uint64_t * size, uint32_t * checksum)
{
	uint32_t  i = 0;
	uint16_t *data = (uint16_t *) (packet->body.content);

	if (BIT64_DATA_LENGTH == packet->body.size)
		*size = *(uint64_t *) (data + MAX_PARTITION_NAME_SIZE);
	else
		*size = (uint64_t)*(uint32_t *) (data + MAX_PARTITION_NAME_SIZE);
	debugf("packet->body.size:0x%x, image size:0x%llx\n", packet->body.size, *size);
	if (NULL != checksum)
		*checksum = *(uint32_t *) (data + MAX_PARTITION_NAME_SIZE + 2);

	while (('\0' != *(data+i)) && (i < (MAX_PARTITION_NAME_SIZE-1))){
		partition_name[i] = *(data+i) & 0xFF;
		i++;
	}
	partition_name[i] = '\0';

	return;
}
#endif

static __inline u32_t convert_operate_status(int err)
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
	printk("%s: version(%d),unit(%d), table_count(%d), table_tag(%d), table_offset(%d), table_size(%d)\n",
		__FUNCTION__, info->version, info->unit, info->table_count, info->table_tag, info->table_offset,
		info->table_size);
	*list = pointer;
	return 0;
}
struct spi_flash *sf = NULL;
static uint32_t start_addr=0;
static uint32_t partition_size=0;
#define NORFLASH_ADDRESS 0x2000000

int dl_write_sf(uint32_t offset,uint32_t size,char *data)
{
	//u32_t	sector;
	int	ret = 0;

/* 	ret = spi_flash_erase(sf, offset,
		sector * sf->sector_size); */
	//printk("Writing to SPI flash...ret=%d, sec=%d, block=%x erasesize=%d \n",ret,sector,sf->sector_size,sf->erase_size);
	if (ret)
		goto done;

	////printk("Writing to SPI flash...ret=%d, sec=%d, block=%x \n",ret,sector,sf->sector_size);
/* 	ret = spi_flash_write(sf, offset,
		size, data); */
	if (ret)
		goto done;

	ret = 1;

 done:
	return ret;
}

int dl_cmd_write_connect(struct dl_pkt *packet, void *arg)
{
	printk("dl_cmd_write_connect \n");
	dl_send_ack(BSL_REP_ACK);
	return 0;
}

int dl_cmd_write_start (struct dl_pkt *packet, void *arg)
{
	memcpy(&start_addr, (void *)(packet->body.content), sizeof(start_addr));
	memcpy(&partition_size, (void *)(packet->body.content+sizeof(start_addr)), sizeof(start_addr));

	start_addr = EndianConv_32(start_addr);
	partition_size = EndianConv_32(partition_size);

	printk("start_addr %x\n",start_addr);
	printk("partition_size %d\n",partition_size);

	_send_reply(1);

	return 0;
}


static char *s_iram_address = (char *)CONFIG_SYS_LOAD_ADDR;

int dl_cmd_write_midst(struct dl_pkt *packet, void *arg)
{
	memcpy(s_iram_address, (void *)(packet->body.content), packet->body.size);
	s_iram_address += packet->body.size;
	_send_reply(1);
	//printk("dl_cmd_write_midst %x\n",s_iram_address); //every packet
	return 0;
}

int dl_cmd_write_end (struct dl_pkt *packet, void *arg)
{
	//int32_t op_res = 1;
	//int32_t offset;
	s_iram_address = (char *)CONFIG_SYS_LOAD_ADDR;
	printk("dl_cmd_write_end %p\n",s_iram_address);
#if 0
	offset = start_addr - NORFLASH_ADDRESS;
	print_addr(offset);
	op_res = dl_write_sf(offset,partition_size,(char *)CONFIG_SYS_LOAD_ADDR);
	if(sf){
	    /* spi_flash_free(sf); */
	    sf = NULL;
	}
	_send_reply(op_res);
#endif
	_send_reply(1);

	return 0;
}

int dl_cmd_init(void)
{
	struct dl_pkt *pkt;

	pkt = dl_pkt_alloc();
	if (pkt == NULL) {
		printk("No packet!\n");
		return -1;
	}

	pkt->body.type = BSL_REP_VER;
	pkt->body.size = sizeof(fdl_ver_str);
	memcpy((u8_t *)pkt->body.content, fdl_ver_str, sizeof(fdl_ver_str));
	dl_packet_send(pkt);

	return 0;
}

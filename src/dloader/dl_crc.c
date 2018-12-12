#include <zephyr.h>
#include "dl_cmd_common.h"
#include "dl_crc.h"

#ifdef __cplusplus
extern   "C"
{
#endif

unsigned int crc_16_l_calc (char *buf_ptr,unsigned int len)
{
    unsigned int i;
    unsigned short crc = 0;

    while (len--!=0)
    {
        for (i = CRC_16_L_SEED; i !=0 ; i = i>>1)
        {
            if ( (crc & CRC_16_L_POLYNOMIAL) !=0)
            {
                crc = crc << 1 ;
                crc = crc ^ CRC_16_POLYNOMIAL;
            }
            else
            {
                crc = crc << 1 ;
            }

            if ( (*buf_ptr & i) != 0) /*lint !e737*/
            {
                crc = crc ^ CRC_16_POLYNOMIAL;
            }
        }

        buf_ptr++;
    }

    return (crc);
}

unsigned short frm_chk (const unsigned short *src, int len)
{
    unsigned int sum = 0;

    while (len > 3)
    {
        sum += *src++;
        sum += *src++;
        len -= 4;
    }

    switch (len&0x03)
    {
        case 2:
            sum += *src++;
            break;
        case 3:
            sum += *src++;
            sum += * ( (unsigned char *) src);
            break;
        case 1:
            sum += * ( (unsigned char *) src);
            break;
        default:
            break;
    }

    sum = (sum >> 16) + (sum & 0x0FFFF);
    sum += (sum >> 16);
    return (~sum);
}

unsigned short EndianConv_16 (unsigned short value)
{
#ifdef _LITTLE_ENDIAN
    return (value >> 8 | value << 8);
#else
    return value;
#endif
}

unsigned int EndianConv_32 (unsigned int value)
{
#ifdef _LITTLE_ENDIAN
    unsigned int nTmp = 0;
	nTmp = (value >> 24 | value << 24);

    nTmp |= ( (value >> 8) & 0x0000FF00);
    nTmp |= ( (value << 8) & 0x00FF0000);
    return nTmp;
#else
    return value;
#endif
}
/**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

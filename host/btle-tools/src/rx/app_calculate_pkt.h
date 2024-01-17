/*************************************************************************
	> File Name: app_calculate_pkt.h
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 16:18:01 CST
 ************************************************************************/

#ifndef _APP_CALCULATE_PKT_H
#define _APP_CALCULATE_PKT_H

#include "app_ble_tools.h"

//#define LEN_DEMOD_BUF_PREAMBLE_ACCESS ( (NUM_PREAMBLE_ACCESS_BYTE*8)-8 ) // to get 2^x integer
//#define LEN_DEMOD_BUF_PREAMBLE_ACCESS 32
//#define LEN_DEMOD_BUF_PREAMBLE_ACCESS (NUM_PREAMBLE_ACCESS_BYTE*8)
#define LEN_DEMOD_BUF_ACCESS (NUM_ACCESS_ADDR_BYTE*8) //32 = 2^5

// demodulates and parses a packet
void receiver(IQ_TYPE *rxp_in, int buf_len, int channel_number, uint32_t access_addr, uint32_t crc_init, int verbose_flag, int raw_flag); 

#endif

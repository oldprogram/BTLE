/*************************************************************************
	> File Name: app_ble_tools.h
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 15:01:32 CST
 ************************************************************************/

#ifndef _APP_BLE_TOOLS_H
#define _APP_BLE_TOOLS_H

#include "../common.h"

//----------------------------------some sys stuff----------------------------------

//----------------------------------some basic signal definition----------------------------------
#define SAMPLE_PER_SYMBOL 4 // 4M sampling rate



#define LEN_BUF_IN_SAMPLE (4*4096) //4096 samples = ~1ms for 4Msps; ATTENTION each rx callback get hackrf.c:lib_device->buffer_size samples!!!
#define LEN_BUF (LEN_BUF_IN_SAMPLE*2)
#define LEN_BUF_IN_SYMBOL (LEN_BUF_IN_SAMPLE/SAMPLE_PER_SYMBOL)
//----------------------------------some basic signal definition----------------------------------

//----------------------------------BTLE SPEC related--------------------------------
#define DEFAULT_CHANNEL 37
#define DEFAULT_ACCESS_ADDR (0x8E89BED6)
#define DEFAULT_CRC_INIT (0x555555)
#define MAX_CHANNEL_NUMBER 39
#define MAX_NUM_INFO_BYTE (43)
#define MAX_NUM_PHY_BYTE (47)
//#define MAX_NUM_PHY_SAMPLE ((MAX_NUM_PHY_BYTE*8*SAMPLE_PER_SYMBOL)+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL))
#define MAX_NUM_PHY_SAMPLE (MAX_NUM_PHY_BYTE*8*SAMPLE_PER_SYMBOL)
#define LEN_BUF_MAX_NUM_PHY_SAMPLE (2*MAX_NUM_PHY_SAMPLE)

#define NUM_PREAMBLE_BYTE (1)
#define NUM_ACCESS_ADDR_BYTE (4)
#define NUM_PREAMBLE_ACCESS_BYTE (NUM_PREAMBLE_BYTE+NUM_ACCESS_ADDR_BYTE)
//----------------------------------BTLE SPEC related--------------------------------

typedef enum
{
    LL_RESERVED,
    LL_DATA1,
    LL_DATA2,
    LL_CTRL
} LL_PDU_TYPE;


typedef struct {
    uint8_t Data[40];
} LL_DATA_PDU_PAYLOAD_TYPE;

typedef enum
{
    LL_CONNECTION_UPDATE_REQ = 0,
    LL_CHANNEL_MAP_REQ= 1,
    LL_TERMINATE_IND= 2,
    LL_ENC_REQ= 3,
    LL_ENC_RSP= 4,
    LL_START_ENC_REQ= 5,
    LL_START_ENC_RSP= 6,
    LL_UNKNOWN_RSP= 7,
    LL_FEATURE_REQ= 8,
    LL_FEATURE_RSP= 9,
    LL_PAUSE_ENC_REQ= 10,
    LL_PAUSE_ENC_RSP= 11,
    LL_VERSION_IND= 12,
    LL_REJECT_IND= 13
} LL_CTRL_PDU_PAYLOAD_TYPE;


typedef struct {
    uint8_t Opcode;
    uint8_t WinSize;
    uint16_t WinOffset;
    uint16_t Interval;
    uint16_t Latency;
    uint16_t Timeout;
    uint16_t Instant;
} LL_CTRL_PDU_PAYLOAD_TYPE_0;

typedef struct {
    uint8_t Opcode;
    uint8_t ChM[5];
    uint16_t Instant;
} LL_CTRL_PDU_PAYLOAD_TYPE_1;

typedef struct {
    uint8_t Opcode;
    uint8_t ErrorCode;
} LL_CTRL_PDU_PAYLOAD_TYPE_2_7_13;

typedef struct {
    uint8_t Opcode;
    uint8_t Rand[8];
    uint8_t EDIV[2];
    uint8_t SKDm[8];
    uint8_t IVm[4];
} LL_CTRL_PDU_PAYLOAD_TYPE_3;

typedef struct {
    uint8_t Opcode;
    uint8_t SKDs[8];
    uint8_t IVs[4];
} LL_CTRL_PDU_PAYLOAD_TYPE_4;

typedef struct {
    uint8_t Opcode;
} LL_CTRL_PDU_PAYLOAD_TYPE_5_6_10_11;

typedef struct {
    uint8_t Opcode;
    uint8_t FeatureSet[8];
} LL_CTRL_PDU_PAYLOAD_TYPE_8_9;

typedef struct {
    uint8_t Opcode;
    uint8_t VersNr;
    uint16_t CompId;
    uint16_t SubVersNr;
} LL_CTRL_PDU_PAYLOAD_TYPE_12;

typedef struct {
    uint8_t Opcode;
    uint8_t payload_byte[40];
} LL_CTRL_PDU_PAYLOAD_TYPE_R;

typedef enum
{
    ADV_IND = 0,
    ADV_DIRECT_IND= 1,
    ADV_NONCONN_IND= 2,
    SCAN_REQ= 3,
    SCAN_RSP= 4,
    CONNECT_REQ= 5,
    ADV_SCAN_IND= 6,
    RESERVED0= 7,
    RESERVED1= 8,
    RESERVED2= 9,
    RESERVED3= 10,
    RESERVED4= 11,
    RESERVED5= 12,
    RESERVED6= 13,
    RESERVED7= 14,
    RESERVED8= 15
} ADV_PDU_TYPE;


typedef struct {
    uint8_t AdvA[6];
    uint8_t Data[31];
} ADV_PDU_PAYLOAD_TYPE_0_2_4_6;

typedef struct {
    uint8_t A0[6];
    uint8_t A1[6];
} ADV_PDU_PAYLOAD_TYPE_1_3;

typedef struct {
    uint8_t InitA[6];
    uint8_t AdvA[6];
    uint8_t AA[4];
    uint32_t CRCInit;
    uint8_t WinSize;
    uint16_t WinOffset;
    uint16_t Interval;
    uint16_t Latency;
    uint16_t Timeout;
    uint8_t ChM[5];
    uint8_t Hop;
    uint8_t SCA;
} ADV_PDU_PAYLOAD_TYPE_5;

typedef struct {
    uint8_t payload_byte[40];
} ADV_PDU_PAYLOAD_TYPE_R;


typedef struct {
    int pkt_avaliable;
    int hop;
    int new_chm_flag;
    int interval;
    uint32_t access_addr;
    uint32_t crc_init;
    uint8_t chm[5];
    bool crc_ok;
} RECV_STATUS;



uint64_t get_freq_by_channel_number(int channel_number);
bool chm_is_full_map(uint8_t *chm);//handle freq hop for channel mapping 1FFFFFFFFF
void scramble_byte(uint8_t *byte_in, int num_byte, const uint8_t *scramble_table_byte, uint8_t *byte_out);
void init_pcap_file();
void write_packet_to_file(FILE *fh, int packet_len, uint8_t *packet, uint8_t channel, uint32_t access_addr);


#endif

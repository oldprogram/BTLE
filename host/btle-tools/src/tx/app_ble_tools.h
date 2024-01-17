/*************************************************************************
	> File Name: app_ble_tools.h
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 15:00:14 CST
 ************************************************************************/

#ifndef _APP_BLE_TOOLS_H
#define _APP_BLE_TOOLS_H

#include "../common.h"

#ifdef USE_BLADERF
#define SAMPLE_PER_SYMBOL 4
#else
#define SAMPLE_PER_SYMBOL 4
#endif // USE_BLADERF

//#define AMPLITUDE (110.0)
#define AMPLITUDE (127.0)
#define MOD_IDX (0.5)
//#define LEN_GAUSS_FILTER (11) // pre 8, post 3
#define LEN_GAUSS_FILTER (4) // pre 2, post 2
#define MAX_NUM_INFO_BYTE (43)
#define MAX_NUM_PHY_BYTE (47)
#define MAX_NUM_PHY_SAMPLE ((MAX_NUM_PHY_BYTE*8*SAMPLE_PER_SYMBOL)+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL))

#define MAX_NUM_CHAR_CMD (256)

typedef enum{
    FLAGS,
    LOCAL_NAME08,
    LOCAL_NAME09,
    TXPOWER,
    SERVICE02,
    SERVICE03,
    SERVICE04,
    SERVICE05,
    SERVICE06,
    SERVICE07,
    SERVICE_SOLI14,
    SERVICE_SOLI15,
    SERVICE_DATA,
    MANUF_DATA,
    CONN_INTERVAL,
    SPACE,
    NUM_AD_TYPE
} AD_TYPE;

typedef enum{
    INVALID_TYPE,
    RAW,
    DISCOVERY,
    IBEACON,
    ADV_IND,
    ADV_DIRECT_IND,
    ADV_NONCONN_IND,
    ADV_SCAN_IND,
    SCAN_REQ,
    SCAN_RSP,
    CONNECT_REQ,
    LL_DATA,
    LL_CONNECTION_UPDATE_REQ,
    LL_CHANNEL_MAP_REQ,
    LL_TERMINATE_IND,
    LL_ENC_REQ,
    LL_ENC_RSP,
    LL_START_ENC_REQ,
    LL_START_ENC_RSP,
    LL_UNKNOWN_RSP,
    LL_FEATURE_REQ,
    LL_FEATURE_RSP,
    LL_PAUSE_ENC_REQ,
    LL_PAUSE_ENC_RSP,
    LL_VERSION_IND,
    LL_REJECT_IND,
    NUM_PKT_TYPE
} PKT_TYPE;

typedef struct{
    int channel_number;
    PKT_TYPE pkt_type;

    char cmd_str[MAX_NUM_CHAR_CMD]; // hex string format command input

    int num_info_bit;
    char info_bit[MAX_NUM_PHY_BYTE*8]; // without CRC and whitening

    int num_info_byte;
    uint8_t info_byte[MAX_NUM_PHY_BYTE];

    int num_phy_bit;
    char phy_bit[MAX_NUM_PHY_BYTE*8]; // all bits which will be fed to GFSK modulator

    int num_phy_byte;
    uint8_t phy_byte[MAX_NUM_PHY_BYTE];

    int num_phy_sample;
    char phy_sample[2*MAX_NUM_PHY_SAMPLE]; // GFSK output to D/A (hackrf board)
    int8_t phy_sample1[2*MAX_NUM_PHY_SAMPLE]; // GFSK output to D/A (hackrf board)

    int space; // how many millisecond null signal shouwl be padded after this packet
} PKT_INFO;


void fill_adv_pdu_header_byte(PKT_TYPE pkt_type, int txadd, int rxadd, int payload_len, uint8_t *byte_out);
void fill_adv_pdu_header(PKT_TYPE pkt_type, int txadd, int rxadd, int payload_len, char *bit_out);
void fill_hop_sca(int hop, int sca, char *bit_out);
void fill_data_pdu_header(int llid, int nesn, int sn, int md, int length, char *bit_out);
void get_opcode(PKT_TYPE pkt_type, char *bit_out);
void scramble_byte(uint8_t *byte_in, int num_byte, int channel_number, uint8_t *byte_out);
void scramble(char *bit_in, int num_bit, int channel_number, char *bit_out);
void crc24_and_scramble_byte_to_gen_phy_bit(char *crc_init_hex, PKT_INFO *pkt);
void crc24_and_scramble_to_gen_phy_bit(char *crc_init_hex, PKT_INFO *pkt);

#endif

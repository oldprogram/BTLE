/*************************************************************************
	> File Name: app_ble_tools.c
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 15:00:10 CST
 ************************************************************************/
#include "app_ble_tools.h"

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 蓝牙组包相关 
//---------------------------------------------------------------------------------------
void fill_adv_pdu_header_byte(PKT_TYPE pkt_type, int txadd, int rxadd, int payload_len, uint8_t *byte_out) {
    if (pkt_type == ADV_IND || pkt_type == IBEACON) {
        //bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 0; bit_out[0] = 0;
        byte_out[0] = 0;
    } else if (pkt_type == ADV_DIRECT_IND) {
        //bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 0; bit_out[0] = 1;
        byte_out[0] = 1;
    } else if (pkt_type == ADV_NONCONN_IND || pkt_type == DISCOVERY) {
        //bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 1; bit_out[0] = 0;
        byte_out[0] = 2;
    } else if (pkt_type == SCAN_REQ) {
        //bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 1; bit_out[0] = 1;
        byte_out[0] = 3;
    } else if (pkt_type == SCAN_RSP) {
        //bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 0; bit_out[0] = 0;
        byte_out[0] = 4;
    } else if (pkt_type == CONNECT_REQ) {
        //bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 0; bit_out[0] = 1;
        byte_out[0] = 5;
    } else if (pkt_type == ADV_SCAN_IND) {
        //bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 1; bit_out[0] = 0;
        byte_out[0] = 6;
    } else {
        //bit_out[3] = 1; bit_out[2] = 1; bit_out[1] = 1; bit_out[0] = 1;
        byte_out[0] = 0xF;
        printf("Warning! Reserved TYPE!\n");
    }

    /*bit_out[4] = 0;
      bit_out[5] = 0;

      bit_out[6] = txadd;
      bit_out[7] = rxadd;*/
    byte_out[0] =  byte_out[0] | (txadd << 6);
    byte_out[0] =  byte_out[0] | (rxadd << 7);

    /*bit_out[8] = 0x01&(payload_len>>0);
      bit_out[9] = 0x01&(payload_len>>1);
      bit_out[10] = 0x01&(payload_len>>2);
      bit_out[11] = 0x01&(payload_len>>3);
      bit_out[12] = 0x01&(payload_len>>4);
      bit_out[13] = 0x01&(payload_len>>5);

      bit_out[14] = 0;
      bit_out[15] = 0;*/
    byte_out[1] = payload_len;
}

void fill_adv_pdu_header(PKT_TYPE pkt_type, int txadd, int rxadd, int payload_len, char *bit_out) {
    if (pkt_type == ADV_IND || pkt_type == IBEACON) {
        bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 0; bit_out[0] = 0;
    } else if (pkt_type == ADV_DIRECT_IND) {
        bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 0; bit_out[0] = 1;
    } else if (pkt_type == ADV_NONCONN_IND || pkt_type == DISCOVERY) {
        bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 1; bit_out[0] = 0;
    } else if (pkt_type == SCAN_REQ) {
        bit_out[3] = 0; bit_out[2] = 0; bit_out[1] = 1; bit_out[0] = 1;
    } else if (pkt_type == SCAN_RSP) {
        bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 0; bit_out[0] = 0;
    } else if (pkt_type == CONNECT_REQ) {
        bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 0; bit_out[0] = 1;
    } else if (pkt_type == ADV_SCAN_IND) {
        bit_out[3] = 0; bit_out[2] = 1; bit_out[1] = 1; bit_out[0] = 0;
    } else {
        bit_out[3] = 1; bit_out[2] = 1; bit_out[1] = 1; bit_out[0] = 1;
        printf("Warning! Reserved TYPE!\n");
    }

    bit_out[4] = 0;
    bit_out[5] = 0;

    bit_out[6] = txadd;
    bit_out[7] = rxadd;

    bit_out[8] = 0x01&(payload_len>>0);
    bit_out[9] = 0x01&(payload_len>>1);
    bit_out[10] = 0x01&(payload_len>>2);
    bit_out[11] = 0x01&(payload_len>>3);
    bit_out[12] = 0x01&(payload_len>>4);
    bit_out[13] = 0x01&(payload_len>>5);

    bit_out[14] = 0;
    bit_out[15] = 0;
}

void fill_hop_sca(int hop, int sca, char *bit_out) {
    bit_out[0] = 0x01&(hop>>0);
    bit_out[1] = 0x01&(hop>>1);
    bit_out[2] = 0x01&(hop>>2);
    bit_out[3] = 0x01&(hop>>3);
    bit_out[4] = 0x01&(hop>>4);

    bit_out[5] = 0x01&(sca>>0);
    bit_out[6] = 0x01&(sca>>1);
    bit_out[7] = 0x01&(sca>>2);
}

void fill_data_pdu_header(int llid, int nesn, int sn, int md, int length, char *bit_out) {
    bit_out[0] = 0x01&(llid>>0);
    bit_out[1] = 0x01&(llid>>1);

    bit_out[2] = nesn;

    bit_out[3] = sn;

    bit_out[4] = md;

    bit_out[5] = 0;
    bit_out[6] = 0;
    bit_out[7] = 0;

    bit_out[8] = 0x01&(length>>0);
    bit_out[9] = 0x01&(length>>1);
    bit_out[10] = 0x01&(length>>2);
    bit_out[11] = 0x01&(length>>3);
    bit_out[12] = 0x01&(length>>4);

    bit_out[13] = 0;
    bit_out[14] = 0;
    bit_out[15] = 0;
}


#include "scramble_table_ch37.h"
void scramble_byte(uint8_t *byte_in, int num_byte, int channel_number, uint8_t *byte_out) {
    int i;
    for(i=0; i<num_byte; i++){
        byte_out[i] = byte_in[i]^scramble_table_ch37[i];
    }
}

void get_opcode(PKT_TYPE pkt_type, char *bit_out) {
    if (pkt_type == LL_CONNECTION_UPDATE_REQ) {
        convert_hex_to_bit("00", bit_out);
    } else if (pkt_type == LL_CHANNEL_MAP_REQ) {
        convert_hex_to_bit("01", bit_out);
    } else if (pkt_type == LL_TERMINATE_IND) {
        convert_hex_to_bit("02", bit_out);
    } else if (pkt_type == LL_ENC_REQ) {
        convert_hex_to_bit("03", bit_out);
    } else if (pkt_type == LL_ENC_RSP) {
        convert_hex_to_bit("04", bit_out);
    } else if (pkt_type == LL_START_ENC_REQ) {
        convert_hex_to_bit("05", bit_out);
    } else if (pkt_type == LL_START_ENC_RSP) {
        convert_hex_to_bit("06", bit_out);
    } else if (pkt_type == LL_UNKNOWN_RSP) {
        convert_hex_to_bit("07", bit_out);
    } else if (pkt_type == LL_FEATURE_REQ) {
        convert_hex_to_bit("08", bit_out);
    } else if (pkt_type == LL_FEATURE_RSP) {
        convert_hex_to_bit("09", bit_out);
    } else if (pkt_type == LL_PAUSE_ENC_REQ) {
        convert_hex_to_bit("0A", bit_out);
    } else if (pkt_type == LL_PAUSE_ENC_RSP) {
        convert_hex_to_bit("0B", bit_out);
    } else if (pkt_type == LL_VERSION_IND) {
        convert_hex_to_bit("0C", bit_out);
    } else if (pkt_type == LL_REJECT_IND) {
        convert_hex_to_bit("0D", bit_out);
    } else {
        convert_hex_to_bit("FF", bit_out);
        printf("Warning! Reserved TYPE!\n");
    }
}

void scramble(char *bit_in, int num_bit, int channel_number, char *bit_out) {
    char bit_store[7], bit_store_update[7];
    int i;

    bit_store[0] = 1;
    bit_store[1] = 0x01&(channel_number>>5);
    bit_store[2] = 0x01&(channel_number>>4);
    bit_store[3] = 0x01&(channel_number>>3);
    bit_store[4] = 0x01&(channel_number>>2);
    bit_store[5] = 0x01&(channel_number>>1);
    bit_store[6] = 0x01&(channel_number>>0);

    for (i=0; i<num_bit; i++) {
        bit_out[i] = ( bit_store[6] + bit_in[i] )%2;

        bit_store_update[0] = bit_store[6];

        bit_store_update[1] = bit_store[0];
        bit_store_update[2] = bit_store[1];
        bit_store_update[3] = bit_store[2];

        bit_store_update[4] = (bit_store[3]+bit_store[6])%2;

        bit_store_update[5] = bit_store[4];
        bit_store_update[6] = bit_store[5];

        memcpy(bit_store, bit_store_update, 7);
    }
}

void crc24_and_scramble_byte_to_gen_phy_bit(char *crc_init_hex, PKT_INFO *pkt) {
    crc24(pkt->info_bit+5*8, pkt->num_info_bit-5*8, crc_init_hex, pkt->info_bit+pkt->num_info_bit);

    int crc24_checksum = crc24_byte(pkt->info_byte+5, pkt->num_info_byte-5, 0xAAAAAA); // 0x555555 --> 0xaaaaaa
    (pkt->info_byte+pkt->num_info_byte)[0] = crc24_checksum & 0xFF;
    (pkt->info_byte+pkt->num_info_byte)[1] = (crc24_checksum>>8) & 0xFF;
    (pkt->info_byte+pkt->num_info_byte)[2] = (crc24_checksum>>16) & 0xFF;

    printf("after crc24\n");
    disp_bit_in_hex(pkt->info_bit, pkt->num_info_bit + 3*8);
    disp_hex(pkt->info_byte, pkt->num_info_byte + 3);

    scramble(pkt->info_bit+5*8, pkt->num_info_bit-5*8+24, pkt->channel_number, pkt->phy_bit+5*8);
    memcpy(pkt->phy_bit, pkt->info_bit, 5*8);
    pkt->num_phy_bit = pkt->num_info_bit + 24;

    scramble_byte(pkt->info_byte+5, pkt->num_info_byte-5+3, pkt->channel_number, pkt->phy_byte+5);
    memcpy(pkt->phy_byte, pkt->info_byte, 5);
    pkt->num_phy_byte = pkt->num_info_byte + 3;

    printf("after scramble %d %d\n", pkt->num_phy_bit , pkt->num_phy_byte);
    disp_bit_in_hex(pkt->phy_bit, pkt->num_phy_bit);
    disp_hex(pkt->phy_byte, pkt->num_phy_byte);
}

void crc24_and_scramble_to_gen_phy_bit(char *crc_init_hex, PKT_INFO *pkt) {
    crc24(pkt->info_bit+5*8, pkt->num_info_bit-5*8, crc_init_hex, pkt->info_bit+pkt->num_info_bit);

    printf("after crc24\n");
    disp_bit_in_hex(pkt->info_bit, pkt->num_info_bit + 3*8);

    scramble(pkt->info_bit+5*8, pkt->num_info_bit-5*8+24, pkt->channel_number, pkt->phy_bit+5*8);
    memcpy(pkt->phy_bit, pkt->info_bit, 5*8);
    pkt->num_phy_bit = pkt->num_info_bit + 24;

    printf("after scramble %d %d\n", pkt->num_phy_bit , pkt->num_phy_byte);
    disp_bit_in_hex(pkt->phy_bit, pkt->num_phy_bit);
}



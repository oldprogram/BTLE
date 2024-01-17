/*************************************************************************
	> File Name: app_ble_tools.c
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 15:01:28 CST
 ************************************************************************/
#include "app_ble_tools.h"


uint64_t get_freq_by_channel_number(int channel_number) {
    uint64_t freq_hz;
    if ( channel_number == 37 ) {
        freq_hz = 2402000000ull;
    } else if (channel_number == 38) {
        freq_hz = 2426000000ull;
    } else if (channel_number == 39) {
        freq_hz = 2480000000ull;
    } else if (channel_number >=0 && channel_number <= 10 ) {
        freq_hz = 2404000000ull + channel_number*2000000ull;
    } else if (channel_number >=11 && channel_number <= 36 ) {
        freq_hz = 2428000000ull + (channel_number-11)*2000000ull;
    } else {
        freq_hz = 0xffffffffffffffff;
    }
    return(freq_hz);
}

//handle freq hop for channel mapping 1FFFFFFFFF
bool chm_is_full_map(uint8_t *chm) {
    if ( (chm[0] == 0x1F) && (chm[1] == 0xFF) && (chm[2] == 0xFF) && (chm[3] == 0xFF) && (chm[4] == 0xFF) ) {
        return(true);
    }
    return(false);
}

void scramble_byte(uint8_t *byte_in, int num_byte, const uint8_t *scramble_table_byte, uint8_t *byte_out) {
    int i;
    for(i=0; i<num_byte; i++){
        byte_out[i] = byte_in[i]^scramble_table_byte[i];
    }
}

/* File handling for pcap + BTLE, don't use btbb as it's too buggy and slow */
// TCPDUMP_MAGIC PCAP_VERSION_MAJOR PCAP_VERSION_MINOR thiszone sigfigs snaplen linktype (DLT_BLUETOOTH_LE_LL_WITH_PHDR)
// 0xa1b2c3d4 \x00\x02 \x00\x04 \x00\x00\x00\x00 \x00\x00\x00\x00 \x00\x00\x05\xDC \x00\x00\x01\x00
const char* PCAP_HDR_TCPDUMP = "\xA1\xB2\xC3\xD4\x00\x02\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\xDC\x00\x00\x01\x00";
const int PCAP_HDR_TCPDUMP_LEN = 24;
//const char* PCAP_FILE_NAME = "btle_store.pcap";
char* filename_pcap = NULL;
FILE *fh_pcap_store;

void init_pcap_file() {
    fh_pcap_store = fopen(filename_pcap, "wb");
    fwrite(PCAP_HDR_TCPDUMP, 1, PCAP_HDR_TCPDUMP_LEN, fh_pcap_store);
}

typedef struct {
    int sec;
    int usec;
    int caplen;
    int len;
} pcap_header;

const uint8_t BTLE_HEADER_LEN = 10;

// http://www.whiterocker.com/bt/LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR.html
// num_demod_byte -- LE packet: header + data
void write_packet_to_file(FILE *fh, int packet_len, uint8_t *packet, uint8_t channel, uint32_t access_addr){
    // flags: 0x0001 indicates the LE Packet is de-whitened
    // pcap header: tv_sec tv_usec caplen len
    pcap_header header_pcap;
    //header_pcap.sec = packetcount++;
    header_pcap.caplen = htonl(BTLE_HEADER_LEN + 4 + packet_len);
    header_pcap.len = htonl(BTLE_HEADER_LEN + 4 + packet_len);
    fwrite(&header_pcap, 16, 1, fh_pcap_store);
    // BTLE header: RF_Channel:1 Signal_Power:1 Noise_Power:1 Access_address_off:1 Reference_access_address (receiver):4 flags:2 packet
    uint8_t header_btle[10] = {channel, 0, 0, 0, 0, 0, 0, 0, 1, 0};
    fwrite(header_btle, 1, 10, fh);
    fwrite(&access_addr, 1, 4, fh);
    fwrite(packet, 1, packet_len, fh);
}
void write_dummy_entry(){
    uint8_t pkt[10] = {7,7,7,7,7,7,7,7,7,7};
    write_packet_to_file(fh_pcap_store, 10, pkt, 1, 0xFFFFFFF1);
    write_packet_to_file(fh_pcap_store, 10, pkt, 2, 0xFFFFFFF2);
    write_packet_to_file(fh_pcap_store, 10, pkt, 3, 0xFFFFFFF3);
}



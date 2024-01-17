/*************************************************************************
	> File Name: app_parse_input_rx.h
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 14:01:33 CST
 ************************************************************************/

#ifndef _APP_PARSE_INPUT_RX_H
#define _APP_PARSE_INPUT_RX_H

void usage();
void parse_commandline(
        // Inputs
        int argc,
        char * const argv[],
        // Outputs
        int* chan,
        int* gain,
        int* lnaGain,
        uint8_t* amp,
        uint32_t* access_addr,
        uint32_t* crc_init,
        int* verbose_flag,
        int* raw_flag,
        uint64_t* freq_hz, 
        uint32_t* access_mask, 
        int* hop_flag,
        char** filename_pcap
        );
int receiver_controller(void *rf_dev, int verbose_flag, int *chan, uint32_t *access_addr, uint32_t *crc_init_internal);


#endif

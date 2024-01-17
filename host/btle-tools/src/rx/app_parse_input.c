/*************************************************************************
	> File Name: app_parse_input_rx.c
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 14:01:28 CST
 ************************************************************************/
#include "app_calculate_pkt.h"
#include "app_board.h"

#ifdef USE_BLADERF
#define DEFAULT_GAIN 45
#define MAX_GAIN 60
#else
#define DEFAULT_GAIN 6
#define MAX_GAIN 62
#endif

extern RECV_STATUS receiver_status;

//---------------------------------------------------------------------------------------
// 对内函数调用的函数 
//---------------------------------------------------------------------------------------
void usage() {
    printf("Usage:\n");
    printf("    -h --help\n");
    printf("      Print this help screen\n");
    printf("    -c --chan\n");
    printf("      Channel number. default 37. valid range 0~39\n");
    printf("    -g --gain\n");
    printf("      Rx gain in dB. HACKRF rxvga default %d, valid 0~62. bladeRF default is max rx gain 66dB (valid 0~66)\n", DEFAULT_GAIN);
    printf("    -l --lnaGain\n");
    printf("      LNA gain in dB (HACKRF only). HACKRF lna default %d, valid 0~40.\n", 32);
    printf("    -b --amp\n");
    printf("      Enable amp (HACKRF only). Default off.\n");
    printf("    -a --access\n");
    printf("      Access address. 4 bytes. Hex format (like 89ABCDEF). Default %08x for channel 37 38 39. For other channel you should pick correct value according to sniffed link setup procedure\n", DEFAULT_ACCESS_ADDR);
    printf("    -k --crcinit\n");
    printf("      CRC init value. 3 bytes. Hex format (like 555555). Default %06x for channel 37 38 39. For other channel you should pick correct value according to sniffed link setup procedure\n", DEFAULT_CRC_INIT);
    printf("    -v --verbose\n");
    printf("      Print more information when there is error\n");
    printf("    -r --raw\n");
    printf("      Raw mode. After access addr is detected, print out following raw 42 bytes (without descrambling, parsing)\n");
    printf("    -f --freq_hz\n");
    printf("      This frequency (Hz) will override channel setting (In case someone want to work on freq other than BTLE. More general purpose)\n");
    printf("    -m --access_mask\n");
    printf("      If a bit is 1 in this mask, corresponding bit in access address will be taken into packet existing decision (In case someone want a shorter/sparser unique word to do packet detection. More general purpose)\n");
    printf("    -o --hop\n");
    printf("      This will turn on data channel tracking (frequency hopping) after link setup information is captured in ADV_CONNECT_REQ packet\n");
    printf("    -s --filename\n");
    printf("      Store packets to pcap file.\n");
    printf("\nSee README for detailed information.\n");
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数 
//---------------------------------------------------------------------------------------
// > command line parameters
// Parse the command line arguments and return optional parameters as
// variables.
// Also performs some basic sanity checks on the parameters.
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
        ) {
    printf("BLE sniffer. Xianjun Jiao. putaoshu@msn.com\n\n");

    // Default values
    (*chan) = DEFAULT_CHANNEL;
    (*gain) = DEFAULT_GAIN;
    (*lnaGain) = 32;
    (*amp) = 0;
    (*access_addr) = DEFAULT_ACCESS_ADDR;
    (*crc_init) = 0x555555;
    (*verbose_flag) = 0;
    (*raw_flag) = 0;
    (*freq_hz) = 123;
    (*access_mask) = 0xFFFFFFFF;
    (*hop_flag) = 0;
    (*filename_pcap) = 0;

    while (1) {
        static struct option long_options[] = {
            {"help",         no_argument,       0, 'h'},
            {"chan",   required_argument, 0, 'c'},
            {"gain",         required_argument, 0, 'g'},
            {"lnaGain",         required_argument, 0, 'l'},
            {"amp",         no_argument, 0, 'b'},
            {"access",         required_argument, 0, 'a'},
            {"crcinit",           required_argument, 0, 'k'},
            {"verbose",         no_argument, 0, 'v'},
            {"raw",         no_argument, 0, 'r'},
            {"freq_hz",           required_argument, 0, 'f'},
            {"access_mask",         required_argument, 0, 'm'},
            {"hop",         no_argument, 0, 'o'},
            {"filename",         required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        int c = getopt_long (argc, argv, "hc:g:l:ba:k:vrf:m:os:",
                long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            char * endp;
            case 0:
            // Code should only get here if a long option was given a non-null
            // flag value.
            printf("Check code!\n");
            goto abnormal_quit;
            break;
            case 'v':(*verbose_flag) = 1;break;
            case 'r':(*raw_flag) = 1;break;
            case 'o':(*hop_flag) = 1;break;
            case 'h':goto abnormal_quit;break;
            case 'c':(*chan) = strtol(optarg,&endp,10);break;
            case 'g':(*gain) = strtol(optarg,&endp,10);break;
            case 'l':(*lnaGain) = strtol(optarg,&endp,10);break;
            case 'b':(*amp) = 1;break;
            case 'f':(*freq_hz) = strtol(optarg,&endp,10);break;
            case 'a':(*access_addr) = strtol(optarg,&endp,16);break;
            case 'm':(*access_mask) = strtol(optarg,&endp,16);break;
            case 'k':(*crc_init) = strtol(optarg,&endp,16);break;
            case 's':(*filename_pcap) = (char*)optarg;break;
            case '?':goto abnormal_quit;/* getopt_long already printed an error message. */
            default:goto abnormal_quit;
        }
    }

    if ( (*chan)<0 || (*chan)>MAX_CHANNEL_NUMBER ) {
        printf("channel number must be within 0~%d!\n", MAX_CHANNEL_NUMBER);
        goto abnormal_quit;
    }

    if ( (*gain)<0 || (*gain)>MAX_GAIN ) {
        printf("rx gain must be within 0~%d!\n", MAX_GAIN);
        goto abnormal_quit;
    }

    if ( (*lnaGain)<0 || (*lnaGain)>40 ) {
        printf("lna gain must be within 0~%d!\n", 40);
        goto abnormal_quit;
    }

    // Error if extra arguments are found on the command line
    if (optind < argc) {
        printf("Error: unknown/extra arguments specified on command line!\n");
        goto abnormal_quit;
    }

    return;

abnormal_quit:
    usage();
    exit(-1);
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数 
//---------------------------------------------------------------------------------------
// state machine
int receiver_controller(void *rf_dev, int verbose_flag, int *chan, uint32_t *access_addr, uint32_t *crc_init_internal) {
    const int guard_us = 7000;
    const int guard_us1 = 4000;
    static int hop_chan = 0;
    static int state = 0;
    static int interval_us, target_us, target_us1, hop;
    static struct timeval time_run, time_mark;
    uint64_t freq_hz;

    switch(state) {
        case 0: // wait for track
            if ( receiver_status.crc_ok && receiver_status.hop!=-1 ) { //start track unless you ctrl+c

                if ( !chm_is_full_map(receiver_status.chm) ) {
                    printf("Hop: Not full ChnMap 1FFFFFFFFF! (%02x%02x%02x%02x%02x) Stay in ADV Chn\n", receiver_status.chm[0], receiver_status.chm[1], receiver_status.chm[2], receiver_status.chm[3], receiver_status.chm[4]);
                    receiver_status.hop = -1;
                    return(0);
                }

                printf("Hop: track start ...\n");

                hop = receiver_status.hop;
                interval_us = receiver_status.interval*1250;
                target_us = interval_us - guard_us;
                target_us1 = interval_us - guard_us1;

                hop_chan = ((hop_chan + hop)%37);
                (*chan) = hop_chan;
                freq_hz = get_freq_by_channel_number( hop_chan );

                if( board_set_freq(rf_dev, freq_hz) != 0 ) {
                    return(-1);
                }

                (*crc_init_internal) = crc_init_reorder(receiver_status.crc_init);
                (*access_addr) = receiver_status.access_addr;

                printf("Hop: next ch %d freq %ldMHz access %08x crcInit %06x\n", hop_chan, freq_hz/1000000, receiver_status.access_addr, receiver_status.crc_init);

                state = 1;
                printf("Hop: next state %d\n", state);
            }
            receiver_status.crc_ok = false;

            break;

        case 1: // wait for the 1st packet in data channel
            if ( receiver_status.crc_ok ) {// we capture the 1st data channel packet
                gettimeofday(&time_mark, NULL);
                printf("Hop: 1st data pdu\n");
                state = 2;
                printf("Hop: next state %d\n", state);
            }
            receiver_status.crc_ok = false;

            break;

        case 2: // wait for time is up. let hop to next chan
            gettimeofday(&time_run, NULL);
            if ( TimevalDiff(&time_run, &time_mark)>target_us ) {// time is up. let's hop

                gettimeofday(&time_mark, NULL);

                hop_chan = ((hop_chan + hop)%37);
                (*chan) = hop_chan;
                freq_hz = get_freq_by_channel_number( hop_chan );

                if( board_set_freq(rf_dev, freq_hz) != 0 ) {
                    return(-1);
                }

                if (verbose_flag) printf("Hop: next ch %d freq %ldMHz\n", hop_chan, freq_hz/1000000);

                state = 3;
                if (verbose_flag) printf("Hop: next state %d\n", state);
            }
            receiver_status.crc_ok = false;

            break;

        case 3: // wait for the 1st packet in new data channel
            if ( receiver_status.crc_ok ) {// we capture the 1st data channel packet in new data channel
                gettimeofday(&time_mark, NULL);        
                state = 2;
                if (verbose_flag) printf("Hop: next state %d\n", state);
            }

            gettimeofday(&time_run, NULL);
            if ( TimevalDiff(&time_run, &time_mark)>target_us1 ) {
                if (verbose_flag) printf("Hop: skip\n");

                gettimeofday(&time_mark, NULL);

                hop_chan = ((hop_chan + hop)%37);
                (*chan) = hop_chan;
                freq_hz = get_freq_by_channel_number( hop_chan );

                if( board_set_freq(rf_dev, freq_hz) != 0 ) {
                    return(-1);
                }

                if (verbose_flag) printf("Hop: next ch %d freq %ldMHz\n", hop_chan, freq_hz/1000000);

                if (verbose_flag) printf("Hop: next state %d\n", state);
            }

            receiver_status.crc_ok = false;
            break;

        default:
            printf("Hop: unknown state!\n");
            return(-1);
    }

    return(0);
}


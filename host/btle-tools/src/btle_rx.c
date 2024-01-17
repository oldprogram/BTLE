// Bluetooth Low Energy SDR sniffer by Xianjun Jiao (putaoshu@msn.com)
// app_parse_input.c -> app_calculate_pkt.h -> app_ble_tools.h -> common.h -> app_tools.h
//                              app_board.c ->
#include "./rx/app_ble_tools.h"
#include "./rx/app_board.h"
#include "./rx/app_parse_input.h"
#include "./rx/app_calculate_pkt.h"


extern char *board_name;
extern uint8_t access_bit_mask[];
extern RECV_STATUS receiver_status;
volatile IQ_TYPE rx_buf[LEN_BUF + LEN_BUF_MAX_NUM_PHY_SAMPLE];
volatile int rx_buf_offset; // remember to initialize it!
extern volatile bool do_exit;
extern char* filename_pcap;
extern FILE *fh_pcap_store;

int main(int argc, char** argv) {
    uint64_t freq_hz;
    int gain, lnaGain, chan, phase, rx_buf_offset_tmp, verbose_flag, raw_flag, hop_flag;
    uint8_t amp;
    uint32_t access_addr, access_addr_mask, crc_init, crc_init_internal;
    bool run_flag = false;
    void* rf_dev;
    IQ_TYPE *rxp;

    parse_commandline(argc, argv, &chan, &gain, &lnaGain, &amp, &access_addr, &crc_init, &verbose_flag, &raw_flag, &freq_hz, &access_addr_mask, &hop_flag, &filename_pcap);

    if (freq_hz == 123)
        freq_hz = get_freq_by_channel_number(chan);

    uint32_to_bit_array(access_addr_mask, access_bit_mask);

    printf("Cmd line input: chan %d, freq %ldMHz, access addr %08x, crc init %06x raw %d verbose %d rx %ddB (%s) file=%s\n", chan, freq_hz/1000000, access_addr, crc_init, raw_flag, verbose_flag, gain, board_name, filename_pcap);

    if(filename_pcap != NULL) {
        printf("will store packets to: %s\n", filename_pcap);
        init_pcap_file();
    }

    // run cyclic recv in background
    do_exit = false;
    if ( config_run_board(freq_hz, gain, lnaGain, amp, &rf_dev) != 0 ){
        usage();
        if (rf_dev != NULL) {
            goto program_quit;
        }
        else {
            return(1);
        }
    }

    // init receiver
    receiver_status.pkt_avaliable = 0;
    receiver_status.hop = -1;
    receiver_status.new_chm_flag = 0;
    receiver_status.interval = 0;
    receiver_status.access_addr = 0;
    receiver_status.crc_init = 0;
    receiver_status.chm[0] = 0;
    receiver_status.chm[1] = 0;
    receiver_status.chm[2] = 0;
    receiver_status.chm[3] = 0;
    receiver_status.chm[4] = 0;
    receiver_status.crc_ok = false;

    crc_init_internal = crc_init_reorder(crc_init);

    // scan
    do_exit = false;
    phase = 0;
    rx_buf_offset = 0;
    while(do_exit == false) { //hackrf_is_streaming(hackrf_dev) == HACKRF_TRUE?
        /*
           if ( (rx_buf_offset-rx_buf_offset_old) > 65536 || (rx_buf_offset-rx_buf_offset_old) < -65536 ) {
           printf("%d\n", rx_buf_offset);
           rx_buf_offset_old = rx_buf_offset;
           }
         * */
        // total buf len LEN_BUF = (8*4096)*2 =  (~ 8ms); tail length MAX_NUM_PHY_SAMPLE*2=LEN_BUF_MAX_NUM_PHY_SAMPLE

        rx_buf_offset_tmp = rx_buf_offset - LEN_BUF_MAX_NUM_PHY_SAMPLE;
        // cross point 0
        if (rx_buf_offset_tmp>=0 && rx_buf_offset_tmp<(LEN_BUF/2) && phase==1) {
            //printf("rx_buf_offset cross 0: %d %d %d\n", rx_buf_offset, (LEN_BUF/2), LEN_BUF_MAX_NUM_PHY_SAMPLE);
            phase = 0;

            memcpy((void *)(rx_buf+LEN_BUF), (void *)rx_buf, LEN_BUF_MAX_NUM_PHY_SAMPLE*sizeof(IQ_TYPE));
            rxp = (IQ_TYPE*)(rx_buf + (LEN_BUF/2));
            run_flag = true;
        }

        // cross point 1
        if (rx_buf_offset_tmp>=(LEN_BUF/2) && phase==0) {
            //printf("rx_buf_offset cross 1: %d %d %d\n", rx_buf_offset, (LEN_BUF/2), LEN_BUF_MAX_NUM_PHY_SAMPLE);
            phase = 1;

            rxp = (IQ_TYPE*)rx_buf;
            run_flag = true;
        }

        if (run_flag) {
#if 0
            // ------------------------for offline test -------------------------------------
            //save_phy_sample2(rx_buf+buf_sp, LEN_BUF/2, "/home/jxj/git/BTLE/matlab/sample_iq_4msps.txt");
            load_phy_sample2(tmp_buf, 2097152, "/home/jxj/git/BTLE/matlab/sample_iq_4msps.txt");
            receiver(tmp_buf, 2097152, 37, 0x8E89BED6, 0x555555, 1, 0);
            break;
            // ------------------------for offline test -------------------------------------
#endif

            // -----------------------------real online run--------------------------------
            //receiver(rxp, LEN_BUF_MAX_NUM_PHY_SAMPLE+(LEN_BUF/2), chan);
            receiver(rxp, (LEN_DEMOD_BUF_ACCESS-1)*2*SAMPLE_PER_SYMBOL+(LEN_BUF)/2, chan, access_addr, crc_init_internal, verbose_flag, raw_flag);
            fflush(stdout);
            // -----------------------------real online run--------------------------------

            if (hop_flag){
                if ( receiver_controller(rf_dev, verbose_flag, &chan, &access_addr, &crc_init_internal) != 0 )
                    goto program_quit;
            }

            run_flag = false;
        }
    }

program_quit:
    printf("Exit main loop ...\n");
    stop_close_board(rf_dev);

    if(fh_pcap_store) fclose(fh_pcap_store);
    return(0);
}

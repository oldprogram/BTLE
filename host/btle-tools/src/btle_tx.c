// Bluetooth Low Energy SDR transmitter tool by Xianjun Jiao (putaoshu@msn.com)

// app_parse_input.c -> app_calculate_pkt.h -> app_ble_tools.h -> common.h -> app_tools.h
//                              app_board.c ->
#include "./tx/app_ble_tools.h"
#include "./tx/app_board.h"
#include "./tx/app_parse_input.h"


#define MAX_NUM_PACKET (1024)
PKT_INFO packets[MAX_NUM_PACKET];

int main(int argc, char** argv) {
    int num_packet, i, j, num_items;
    int num_repeat = 0; // -1: inf; 0: 1; other: specific

    if (argc < 2) {
        usage();
        return(0);
    } else if ( (argc-1-1) > MAX_NUM_PACKET ){
        printf("Too many packets input! Maximum allowed is %d\n", MAX_NUM_PACKET);
    } else if (argc == 2 && ( strstr(argv[1], ".txt")!=NULL || strstr(argv[1], ".TXT")!=NULL) ) {  // from file
        char **items = malloc_2d(MAX_NUM_PACKET+2, MAX_NUM_CHAR_CMD);
        if (items == NULL) {
            printf("malloc failed!\n");
            return(-1);
        }

        if ( read_items_from_file(&num_items, items, MAX_NUM_PACKET+2, argv[1]) == -1 ) {
            release_2d(items, MAX_NUM_PACKET+2);
            return(-1);
        }
        num_packet = parse_input(num_items, items, &num_repeat);

        release_2d(items, MAX_NUM_PACKET+2);

        if ( num_repeat == -2 ){
            return(-1);
        }
    } else { // from command line
        num_packet = parse_input(argc, argv, &num_repeat);
        if ( num_repeat == -2 ){
            return(-1);
        }
    }
    printf("\n");

    if ( init_board() == -1 ){
        usage();
        return(-1);
    }

#if 0
    //-----------------------------------test tx buf---------------------------------
    set_freq_by_channel_number(37);

    // open the board-----------------------------------------
    if (open_board() == -1) {
        printf("main: open_board() failed\n");
        goto main_out;
    }

    do_exit = false;

    int tx_buffer_length_old, tx_valid_length_old, tx_count_old;
    int result = hackrf_start_tx(device, tx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("main: hackrf_start_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        goto main_out;
    }

    tx_buffer_length_old = tx_buffer_length;
    tx_valid_length_old = tx_valid_length;
    tx_count_old = tx_count;
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false) )
    {
        if ( (tx_count-tx_count_old)>32 || (tx_buffer_length-tx_buffer_length_old)>512 || (tx_buffer_length-tx_buffer_length_old)<0 || (tx_valid_length-tx_valid_length_old)>512 || (tx_valid_length-tx_valid_length_old)<0 ) {
            printf("%d %d %d(old %d %d %d)\n", tx_buffer_length, tx_valid_length, tx_count, tx_buffer_length_old, tx_valid_length_old, tx_count_old);
            tx_buffer_length_old = tx_buffer_length;
            tx_valid_length_old = tx_valid_length;
            tx_count_old = tx_count;
        }
    }

    if (do_exit)
    {
        printf("\nmain: Exiting...\n");
    }

    result = hackrf_stop_tx(device);
    if( result != HACKRF_SUCCESS ) {
        printf("main: hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
    }
    //----------------------------test tx buf-------------------------------
#endif

#if 1
#ifndef USE_BLADERF
    //flush hackrf onboard buf
#if 0
    for(i=0; i<(HACKRF_ONBOARD_BUF_SIZE/HACKRF_USB_BUF_SIZE)+16; i++) {
        if ( tx_one_buf(tx_zeros, HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA, packets[0].channel_number) == -1 ){
            close_board();
            goto main_out;
        }
    }
#endif

#if 0
    if ( tx_one_buf(tx_zeros, HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA, packets[0].channel_number) == -1 ){
        close_board();
        goto main_out;
    }
    if ( tx_one_buf(tx_zeros, HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA, packets[0].channel_number) == -1 ){
        close_board();
        goto main_out;
    }
#endif

#endif

    struct timeval time_tmp, time_current_pkt, time_pre_pkt;
    gettimeofday(&time_current_pkt, NULL);
    for (j=0; j<num_repeat; j++ ) {
        for (i=0; i<num_packet; i++) {
            time_pre_pkt = time_current_pkt;
            gettimeofday(&time_current_pkt, NULL);

            if ( tx_one_buf(packets[i].phy_sample, 2*packets[i].num_phy_sample, packets[i].channel_number) == -1 ){
                close_board();
                goto main_out;
            }

#if 0
            if ( tx_one_buf(tx_zeros, HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA, packets[0].channel_number) == -1 ){
                close_board();
                goto main_out;
            }
#endif

            printf("r%d p%d at %dus\n", j, i,  TimevalDiff(&time_current_pkt, &time_pre_pkt) );

            gettimeofday(&time_tmp, NULL);
            while(TimevalDiff(&time_tmp, &time_current_pkt)<( packets[i].space*1000 ) ) {
                gettimeofday(&time_tmp, NULL);
            }
        }
    }
    printf("\n");
#endif 

main_out:
    exit_board();
    printf("exit\n");

    return(0);
}

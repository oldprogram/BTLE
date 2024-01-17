/*************************************************************************
  > File Name: app_board.c
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 15:38:18 CST
 ************************************************************************/
#include "app_ble_tools.h"

#if SAMPLE_PER_SYMBOL==10
float gauss_coef[LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.551115e-17, 1.165734e-15, 2.231548e-14, 3.762546e-13, 5.522305e-12, 7.048379e-11, 7.826021e-10, 7.561773e-09, 6.360787e-08, 4.660229e-07, 2.975472e-06, 1.656713e-05, 8.050684e-05, 3.417749e-04, 1.269100e-03, 4.128134e-03, 1.178514e-02, 2.959908e-02, 6.560143e-02, 1.288102e-01, 2.252153e-01, 3.529425e-01, 4.999195e-01, 6.466991e-01, 7.735126e-01, 8.670612e-01, 9.226134e-01, 9.408018e-01, 9.226134e-01, 8.670612e-01, 7.735126e-01, 6.466991e-01, 4.999195e-01, 3.529425e-01, 2.252153e-01, 1.288102e-01, 6.560143e-02, 2.959908e-02, 1.178514e-02, 4.128134e-03, 1.269100e-03, 3.417749e-04, 8.050684e-05, 1.656713e-05, 2.975472e-06, 4.660229e-07, 6.360787e-08, 7.561773e-09, 7.826021e-10, 7.048379e-11, 5.522305e-12, 3.762546e-13, 2.231548e-14, 1.165734e-15, 5.551115e-17, 0, 0};
#endif
#if SAMPLE_PER_SYMBOL==8
float gauss_coef[LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.551115e-16, 2.231548e-14, 7.461809e-13, 2.007605e-11, 4.343541e-10, 7.561773e-09, 1.060108e-07, 1.197935e-06, 1.092397e-05, 8.050684e-05, 4.804052e-04, 2.326833e-03, 9.176993e-03, 2.959908e-02, 7.852842e-02, 1.727474e-01, 3.185668e-01, 4.999195e-01, 6.809419e-01, 8.249246e-01, 9.122945e-01, 9.408018e-01, 9.122945e-01, 8.249246e-01, 6.809419e-01, 4.999195e-01, 3.185668e-01, 1.727474e-01, 7.852842e-02, 2.959908e-02, 9.176993e-03, 2.326833e-03, 4.804052e-04, 8.050684e-05, 1.092397e-05, 1.197935e-06, 1.060108e-07, 7.561773e-09, 4.343541e-10, 2.007605e-11, 7.461809e-13, 2.231548e-14, 5.551115e-16, 0, 0};
#endif
#if SAMPLE_PER_SYMBOL==6
float gauss_coef[LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.665335e-16, 2.231548e-14, 2.290834e-12, 1.596947e-10, 7.561773e-09, 2.436464e-07, 5.354390e-06, 8.050684e-05, 8.317661e-04, 5.941078e-03, 2.959908e-02, 1.042296e-01, 2.646999e-01, 4.999195e-01, 7.344630e-01, 8.898291e-01, 9.408018e-01, 8.898291e-01, 7.344630e-01, 4.999195e-01, 2.646999e-01, 1.042296e-01, 2.959908e-02, 5.941078e-03, 8.317661e-04, 8.050684e-05, 5.354390e-06, 2.436464e-07, 7.561773e-09, 1.596947e-10, 2.290834e-12, 2.231548e-14, 1.665335e-16, 0};
#endif
#if SAMPLE_PER_SYMBOL==4
//float gauss_coef[LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2.231548e-14, 2.007605e-11, 7.561773e-09, 1.197935e-06, 8.050684e-05, 2.326833e-03, 2.959908e-02, 1.727474e-01, 4.999195e-01, 8.249246e-01, 9.408018e-01, 8.249246e-01, 4.999195e-01, 1.727474e-01, 2.959908e-02, 2.326833e-03, 8.050684e-05, 1.197935e-06, 7.561773e-09, 2.007605e-11, 2.231548e-14, 0};
float gauss_coef[LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL] = {7.561773e-09, 1.197935e-06, 8.050684e-05, 2.326833e-03, 2.959908e-02, 1.727474e-01, 4.999195e-01, 8.249246e-01, 9.408018e-01, 8.249246e-01, 4.999195e-01, 1.727474e-01, 2.959908e-02, 2.326833e-03, 8.050684e-05, 1.197935e-06};
#endif

uint64_t freq_hz;

volatile bool do_exit = false;

volatile int stop_tx = 1;
volatile int tx_len, tx_buffer_length, tx_valid_length;
volatile int tx_count = 0;

#define NUM_PRE_SEND_DATA (256)

#ifdef USE_BLADERF
#define NUM_BLADERF_BUF_SAMPLE 4096
volatile int16_t tx_buf[NUM_BLADERF_BUF_SAMPLE*2];
struct bladerf *dev = NULL;
#else
//volatile char tx_buf[MAX_NUM_PHY_SAMPLE*2];
#define HACKRF_ONBOARD_BUF_SIZE (32768) // in usb_bulk_buffer.h
#define HACKRF_USB_BUF_SIZE (4096) // in hackrf.c lib_device->buffer_size
char tx_zeros[HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA] = {0};
volatile char *tx_buf;
static hackrf_device* device = NULL;

int tx_callback(hackrf_transfer* transfer) {
#if 0
    tx_buffer_length = transfer->buffer_length; //always is 262144 in old driver, now it is 4096. (3008 for maximum BTLE packet)
    tx_valid_length = transfer->valid_length; //always is 262144 in old driver, now it is 4096. (3008 for maximum BTLE packet)
    tx_count++;
#endif

#if 0
    if (~stop_tx) {
        if ( (tx_len+NUM_PRE_SEND_DATA) <= transfer->valid_length ) {
            // don't feed data to the beginning of transfer->buffer, because tx needs warming up
            memset(transfer->buffer, 0, transfer->valid_length);
            memcpy(transfer->buffer+NUM_PRE_SEND_DATA, (char *)(tx_buf), tx_len);
            stop_tx = 1;
        } else {
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 2;
            return(-1);
        }
    } else {
        memset(transfer->buffer, 0, transfer->valid_length);
    }
#endif

#if 0 // ----------------- simple one   ----------------
    if (~stop_tx) {
        memset(transfer->buffer, 0, NUM_PRE_SEND_DATA);
        memcpy(transfer->buffer+NUM_PRE_SEND_DATA, (char *)(tx_buf), tx_len);
        stop_tx = 1;
    } else {
        memset(transfer->buffer, 0, transfer->valid_length);
    }
#endif

#if 1
    int size_left;
    if (stop_tx == 0) {
        memset(transfer->buffer, 0, NUM_PRE_SEND_DATA);
        memcpy(transfer->buffer+NUM_PRE_SEND_DATA, (char *)(tx_buf), tx_len);

        size_left = (transfer->valid_length - tx_len - NUM_PRE_SEND_DATA);
        memset(transfer->buffer+NUM_PRE_SEND_DATA+tx_len, 0, size_left);
    } else {
        memset(transfer->buffer, 0, transfer->valid_length);
    }
    stop_tx++;
#endif

    //lib_device->transfer_count = 4;
#if 0
    int size_left;
    switch(stop_tx) {

        case 0:
            memset(transfer->buffer, 0, NUM_PRE_SEND_DATA);
            memcpy(transfer->buffer+NUM_PRE_SEND_DATA, (char *)(tx_buf), tx_len);

            size_left = (transfer->valid_length - tx_len - NUM_PRE_SEND_DATA);
            memset(transfer->buffer+NUM_PRE_SEND_DATA+tx_len, 0, size_left);

            stop_tx = 1;
            break;

        case 1:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 2;
            break;

        case 2:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 3;
            break;

        case 3:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 4;
            break;

        case 4:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 5;
            break;

        case 5:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 6;
            break;

        case 6:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 7;
            break;

        case 7:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 8;
            break;

        case 8:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 9;
            break;

        default:
            memset(transfer->buffer, 0, transfer->valid_length);
            stop_tx = 9;
            break;

    }
#endif

    return(0);
}
#endif

#ifdef _MSC_VER
    BOOL WINAPI
sighandler(int signum)
{
    if (CTRL_C_EVENT == signum) {
        fprintf(stdout, "Caught signal %d\n", signum);
        do_exit = true;
        return TRUE;
    }
    return FALSE;
}
#else
void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}
#endif


inline void set_freq_by_channel_number(int channel_number) {

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
    }
}


#ifdef USE_BLADERF

int init_board() {
    int status;
    unsigned int actual;

#ifdef _MSC_VER
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) sighandler, TRUE );
#else
    if (signal(SIGINT, sigint_callback_handler) == SIG_ERR ||
            signal(SIGTERM, sigint_callback_handler) == SIG_ERR) {
        fprintf(stderr, "Failed to set up signal handler\n");
        return EXIT_FAILURE;
    }
#endif

    status = bladerf_open(&dev, NULL);
    if (status < 0) {
        fprintf(stderr, "Failed to open device: %s\n", bladerf_strerror(status));
        return EXIT_FAILURE;
    } else  {
        fprintf(stdout, "open device: %s\n", bladerf_strerror(status));
    }

    status = bladerf_is_fpga_configured(dev);
    if (status < 0) {
        fprintf(stderr, "Failed to determine FPGA state: %s\n",
                bladerf_strerror(status));
        return EXIT_FAILURE;
    } else if (status == 0) {
        fprintf(stderr, "Error: FPGA is not loaded.\n");
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else  {
        fprintf(stdout, "FPGA is loaded.\n");
    }

    status = bladerf_set_frequency(dev, BLADERF_MODULE_TX, 2402000000ull);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "set frequency: %lluHz %s\n", 2402000000ull,
                bladerf_strerror(status));
    }

    status = bladerf_set_sample_rate(dev, BLADERF_MODULE_TX, SAMPLE_PER_SYMBOL*1000000ul, &actual);
    if (status != 0) {
        fprintf(stderr, "Failed to set sample rate: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "set sample rate: %dHz %s\n", actual,
                bladerf_strerror(status));
    }

    status = bladerf_set_bandwidth(dev, BLADERF_MODULE_TX, SAMPLE_PER_SYMBOL*1000000ul/2, &actual);
    if (status != 0) {
        fprintf(stderr, "Failed to set bandwidth: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_set_bandwidth: %d %s\n", actual,
                bladerf_strerror(status));
    }

    status = bladerf_set_gain(dev, BLADERF_MODULE_TX, 57);
    if (status != 0) {
        fprintf(stderr, "Failed to set gain: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_set_gain: %d %s\n", 57,
                bladerf_strerror(status));
    }

#if 0 // old version do not have this API
    status = bladerf_get_gain(dev, BLADERF_MODULE_TX, &actual);
    if (status != 0) {
        fprintf(stderr, "Failed to get gain: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_get_gain: %d %s\n", actual,
                bladerf_strerror(status));
    }
#endif

    status = bladerf_sync_config(dev,
            BLADERF_MODULE_TX,
            BLADERF_FORMAT_SC16_Q11,
            32,
            NUM_BLADERF_BUF_SAMPLE,
            16,
            10);

    if (status != 0) {
        fprintf(stderr, "Failed to initialize TX sync handle: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_sync_config: %s\n",
                bladerf_strerror(status));
    }

    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    if (status < 0) {
        fprintf(stderr, "Failed to enable module: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "enable module true: %s\n",
                bladerf_strerror(status));
    }

    return(0);
}

int open_board(){return 0;}

int close_board(){
    int status;

    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, false);
    if (status < 0) {
        fprintf(stderr, "Failed to enable module: %s\n",
                bladerf_strerror(status));
    } else {
        fprintf(stdout, "enable module false: %s\n", bladerf_strerror(status));
    }

    bladerf_close(dev);

    printf("bladeRF closed.\n");
    return 0;
}
void exit_board() {
    return;
}
int tx_one_buf(char *buf, int length, int channel_number) {
    int status, i;

    set_freq_by_channel_number(channel_number);

    memset( (void *)tx_buf, 0, NUM_BLADERF_BUF_SAMPLE*2*sizeof(tx_buf[0]) );

    for (i=(NUM_BLADERF_BUF_SAMPLE*2-length); i<(NUM_BLADERF_BUF_SAMPLE*2); i++) {
        tx_buf[i] = ( (int)( buf[i-(NUM_BLADERF_BUF_SAMPLE*2-length)] ) )*16;
    }

    // Transmit samples
    status = bladerf_sync_tx(dev, (void *)tx_buf, NUM_BLADERF_BUF_SAMPLE, NULL, 10);
    if (status != 0) {
        printf("tx_one_buf: Failed to TX samples 1: %s\n",
                bladerf_strerror(status));
        return(-1);
    }

    if (do_exit)
    {
        printf("\ntx_one_buf: Exiting...\n");
        return(-1);
    }

    return(0);
}

#else
int init_board() {
    int result = hackrf_init();
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_init() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

#ifdef _MSC_VER
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) sighandler, TRUE );
#else
    signal(SIGINT, &sigint_callback_handler);
    signal(SIGILL, &sigint_callback_handler);
    signal(SIGFPE, &sigint_callback_handler);
    signal(SIGSEGV, &sigint_callback_handler);
    signal(SIGTERM, &sigint_callback_handler);
    signal(SIGABRT, &sigint_callback_handler);
#endif

    return(0);
}

int open_board() {
    int result;

    result = hackrf_open(&device);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_open() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    result = hackrf_set_freq(device, freq_hz);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_freq() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    result = hackrf_set_sample_rate(device, SAMPLE_PER_SYMBOL*1000000ul);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_sample_rate() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    /* range 0-47 step 1db */
    result = hackrf_set_txvga_gain(device, 47);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_txvga_gain() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

#if 0
    result = hackrf_set_antenna_enable(device, 1);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_antenna_enable() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }
#endif

    return(0);
}

void exit_board() {
    if(device != NULL)
    {
        hackrf_exit();
        printf("hackrf_exit() done\n");
    }
}

int close_board() {
    int result;

    if(device != NULL)
    {
        result = hackrf_stop_tx(device);
        if( result != HACKRF_SUCCESS ) {
            printf("close_board: hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
            return(-1);
        }

        result = hackrf_close(device);
        if( result != HACKRF_SUCCESS )
        {
            printf("close_board: hackrf_close() failed: %s (%d)\n", hackrf_error_name(result), result);
            return(-1);
        }

        return(0);
    } else {
        return(-1);
    }
}

int tx_one_buf(char *buf, int length, int channel_number) {
    int result;

    set_freq_by_channel_number(channel_number);

    //tx_buf = tx_zeros;
    //tx_len = HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA;
    tx_buf = buf;
    tx_len = length;

    // open the board-----------------------------------------
    if (open_board() == -1) {
        printf("tx_one_buf: open_board() failed\n");
        return(-1);
    }

    // first round TX---------------------------------
    stop_tx = 0;

    result = hackrf_start_tx(device, tx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_start_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false) )
    {
        if (stop_tx>=9) {
            break;
        }
    }

    if (do_exit)
    {
        printf("\ntx_one_buf: Exiting...\n");
        return(-1);
    }

    // result = hackrf_stop_tx(device);
    // if( result != HACKRF_SUCCESS ) {
    //   printf("tx_one_buf: hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
    //   return(-1);
    // }

#if 0
    do_exit = false;

    // second round TX-----------------------------------
    tx_buf = tx_zeros;
    tx_len = HACKRF_USB_BUF_SIZE-NUM_PRE_SEND_DATA;
    //tx_buf = buf;
    //tx_len = length;

    stop_tx = 0;

    result = hackrf_start_tx(device, tx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_start_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false) )
    {
        if (stop_tx==1) {
            break;
        }
    }

    if (do_exit)
    {
        printf("\ntx_one_buf: Exiting...\n");
        return(-1);
    }

    result = hackrf_stop_tx(device);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    do_exit = false;

    // another round to flush ------------------------------
    stop_tx = 0;

    result = hackrf_start_tx(device, tx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_start_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false) )
    {
        if (stop_tx==1) {
            break;
        }
    }

    if (do_exit)
    {
        printf("\ntx_one_buf: Exiting...\n");
        return(-1);
    }

    result = hackrf_stop_tx(device);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_stop_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    do_exit = false;

    // another round to flush ------------------------------
    stop_tx = 0;

    result = hackrf_start_tx(device, tx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("tx_one_buf: hackrf_start_tx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false) )
    {
        if (stop_tx==1) {
            break;
        }
    }

    if (do_exit)
    {
        printf("\ntx_one_buf: Exiting...\n");
        return(-1);
    }
#endif

    // close the board---------------------------------------
    if (close_board() == -1) {
        printf("tx_one_buf: close_board() failed\n");
        return(-1);
    }

    do_exit = false;

    return(0);
}
#endif // USE_BLADERF


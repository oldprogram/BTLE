/*************************************************************************
	> File Name: app_board_rx.c
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 13:53:51 CST
 ************************************************************************/
#include "app_ble_tools.h"


extern volatile IQ_TYPE rx_buf[];
extern volatile int rx_buf_offset;

volatile bool do_exit = false;
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

#ifdef USE_BLADERF //--------------------------------------BladeRF-----------------------
char *board_name = "BladeRF";
typedef struct bladerf_devinfo bladerf_devinfo;
typedef struct bladerf bladerf_device;
//typedef int16_t IQ_TYPE;
//volatile IQ_TYPE rx_buf[LEN_BUF+LEN_BUF_MAX_NUM_PHY_SAMPLE];

struct bladerf_async_task
{
    pthread_t rx_task;
    pthread_mutex_t stderr_lock;
};

struct bladerf_data
{
    void                **buffers;      /* Transmit buffers */
    size_t              num_buffers;    /* Number of buffers */
    size_t              samples_per_buffer; /* Number of samples per buffer */
    unsigned int        idx;            /* The next one that needs to go out */
    volatile IQ_TYPE    *rx_buf;        /* rx buffer */
    //    bladerf_module      module;         /* Direction */
    //    FILE                *fout;          /* Output file (RX only) */
    //    ssize_t             samples_left;   /* Number of samples left */
};

struct bladerf_stream *stream;
struct bladerf_async_task async_task;
struct bladerf_data rx_data;
//unsigned int count = 0;

void *stream_callback(struct bladerf *dev, struct bladerf_stream *stream,
        struct bladerf_metadata *metadata, void *samples,
        size_t num_samples, void *user_data)
{
    struct bladerf_data *my_data = (struct bladerf_data *)user_data;

    //count++ ;

    //if( (count&0xffff) == 0 ) {
    //    fprintf( stderr, "Called 0x%8.8x times\n", count ) ;
    //}

    /* Save off the samples to disk if we are in RX */
    //if( my_data->module == BLADERF_MODULE_RX ) {
    size_t i;
    int16_t *sample = (int16_t *)samples ;
    for(i = 0; i < num_samples ; i++ ) {
        //*(sample) &= 0xfff ;
        //if( (*sample)&0x800 ) *(sample) |= 0xf000 ;
        //*(sample+1) &= 0xfff ;
        //if( *(sample+1)&0x800 ) *(sample+1) |= 0xf000 ;
        //fprintf( my_data->fout, "%d, %d\n", *sample, *(sample+1) );

        rx_buf[rx_buf_offset] = (((*sample)>>4)&0xFF);
        rx_buf[rx_buf_offset+1] = (((*(sample+1))>>4)&0xFF);
        rx_buf_offset = (rx_buf_offset+2)&( LEN_BUF-1 ); //cyclic buffer

        sample += 2 ;
    }
    //my_data->samples_left -= num_samples ;
    //if( my_data->samples_left <= 0 ) {
    //    do_exit = true ;
    //}
    //}

    if (do_exit) {
        return NULL;
    } else {
        void *rv = my_data->buffers[my_data->idx];
        my_data->idx = (my_data->idx + 1) % my_data->num_buffers;
        return rv ;
    }
}

int board_set_freq(struct bladerf *dev, uint64_t freq_hz) {
    int status;
    status = bladerf_set_frequency(dev, BLADERF_MODULE_RX, freq_hz);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    }
    return(0);
}

/* Thread-safe wrapper around fprintf(stderr, ...) */
#define print_error(repeater_, ...) do { \
    pthread_mutex_lock(&repeater_->stderr_lock); \
    fprintf(stderr, __VA_ARGS__); \
    pthread_mutex_unlock(&repeater_->stderr_lock); \
} while (0)

void *rx_task_run(void *tmp)
{
    int status;
    struct bladerf_async_task *tmp_p = &async_task;

    /* Start stream and stay there until we kill the stream */
    status = bladerf_stream(stream, BLADERF_MODULE_RX);
    if (status < 0) {
        print_error(tmp_p, "RX stream failure: %s\r\n", bladerf_strerror(status));
    }
    return NULL;
}

inline int config_run_board(uint64_t freq_hz, int gain, int lnaGain, uint8_t amp, void **rf_dev) {
    int status;
    unsigned int actual;
    struct bladerf *dev;

    rx_data.idx = 0;
    rx_data.num_buffers = 2;
    rx_data.samples_per_buffer = (LEN_BUF/2);
    rx_data.rx_buf = rx_buf;

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

    status = bladerf_set_frequency(dev, BLADERF_MODULE_RX, freq_hz);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "set frequency: %luHz %s\n", freq_hz,
                bladerf_strerror(status));
    }

    status = bladerf_set_sample_rate(dev, BLADERF_MODULE_RX, SAMPLE_PER_SYMBOL*1000000ul, &actual);
    if (status != 0) {
        fprintf(stderr, "Failed to set sample rate: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "set sample rate: %dHz %s\n", actual,
                bladerf_strerror(status));
    }

    status = bladerf_set_bandwidth(dev, BLADERF_MODULE_RX, SAMPLE_PER_SYMBOL*1000000ul/2, &actual);
    if (status != 0) {
        fprintf(stderr, "Failed to set bandwidth: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_set_bandwidth: %d %s\n", actual,
                bladerf_strerror(status));
    }

    status = bladerf_set_gain(dev, BLADERF_MODULE_RX, gain);
    if (status != 0) {
        fprintf(stderr, "Failed to set gain: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "bladerf_set_gain: %d %s\n", gain,
                bladerf_strerror(status));
    }

#if 0 // old version do not have this API
    status = bladerf_get_gain(dev, BLADERF_MODULE_RX, &actual);
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

    /* Initialize the stream */
    status = bladerf_init_stream(
            &stream,
            dev,
            stream_callback,
            &rx_data.buffers,
            rx_data.num_buffers,
            BLADERF_FORMAT_SC16_Q11,
            rx_data.samples_per_buffer,
            rx_data.num_buffers,
            &rx_data
            );

    if (status != 0) {
        fprintf(stderr, "Failed to init stream: %s\n",
                bladerf_strerror(status));
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "init stream: %s\n",
                bladerf_strerror(status));
    }

    bladerf_set_stream_timeout(dev, BLADERF_MODULE_RX, 100);

    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    if (status < 0) {
        fprintf(stderr, "Failed to enable module: %s\n",
                bladerf_strerror(status));
        bladerf_deinit_stream(stream);
        bladerf_close(dev);
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "enable module true: %s\n",
                bladerf_strerror(status));
    }

    status = pthread_create(&(async_task.rx_task), NULL, rx_task_run, NULL);
    if (status < 0) {
        return EXIT_FAILURE;
    }

    (*rf_dev) = dev;
    return(0);
}

void stop_close_board(struct bladerf *dev){
    int status;

    bladerf_deinit_stream(stream);
    printf("bladerf_deinit_stream.\n");

    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, false);
    if (status < 0) {
        fprintf(stderr, "Failed to enable module: %s\n",
                bladerf_strerror(status));
    } else {
        fprintf(stdout, "enable module false: %s\n", bladerf_strerror(status));
    }

    bladerf_close(dev);
    printf("bladerf_close.\n");

    pthread_join(async_task.rx_task, NULL);
    //pthread_cancel(async_task.rx_task);
    printf("bladeRF rx thread quit.\n");
}

#else //-----------------------------the board is HACKRF-----------------------------
char *board_name = "HACKRF";

int rx_callback(hackrf_transfer* transfer) {
    int i;
    int8_t *p = (int8_t *)transfer->buffer;
    for( i=0; i<transfer->valid_length; i++) {
        rx_buf[rx_buf_offset] = p[i];
        rx_buf_offset = (rx_buf_offset+1)&( LEN_BUF-1 ); //cyclic buffer
    }
    //printf("%d\n", transfer->valid_length); // !!!!it is 262144 always!!!! Now it is 4096. Defined in hackrf.c lib_device->buffer_size
    return(0);
}

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

int board_set_freq(void *device, uint64_t freq_hz) {
    int result = hackrf_set_freq((hackrf_device*)device, freq_hz);
    if( result != HACKRF_SUCCESS ) {
        printf("board_set_freq: hackrf_set_freq() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }
    return(HACKRF_SUCCESS);
}

inline int open_board(uint64_t freq_hz, int gain, int lnaGain, uint8_t amp, hackrf_device** device) {
    int result;

    result = hackrf_open(device);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_open() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    result = hackrf_set_freq(*device, freq_hz);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_freq() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    result = hackrf_set_sample_rate(*device, SAMPLE_PER_SYMBOL*1000000ul);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_sample_rate() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    result = hackrf_set_baseband_filter_bandwidth(*device, SAMPLE_PER_SYMBOL*1000000ul/2);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_baseband_filter_bandwidth() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    printf("Setting VGA gain to %d\n", gain);
    result = hackrf_set_vga_gain(*device, gain);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_vga_gain() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    printf("Setting LNA gain to %d\n", lnaGain);  
    result = hackrf_set_lna_gain(*device, lnaGain);
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_lna_gain() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    printf(amp ? "Enabling amp\n" : "Disabling amp\n");
    result = hackrf_set_amp_enable(*device, amp);	
    if( result != HACKRF_SUCCESS ) {
        printf("open_board: hackrf_set_amp_enable() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    return(0);
}

void exit_board(hackrf_device *device) {
    if(device != NULL)
    {
        hackrf_exit();
        printf("hackrf_exit() done\n");
    }
}

inline int close_board(hackrf_device *device) {
    int result;

    if(device != NULL)
    {
        result = hackrf_stop_rx(device);
        if( result != HACKRF_SUCCESS ) {
            printf("close_board: hackrf_stop_rx() failed: %s (%d)\n", hackrf_error_name(result), result);
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

inline int run_board(hackrf_device* device) {
    int result;

    // result = hackrf_stop_rx(device); // this is not necessary for new hackrf driver
    // if( result != HACKRF_SUCCESS ) {
    // 	printf("run_board: hackrf_stop_rx() failed: %s (%d)\n", hackrf_error_name(result), result);
    // 	return(-1);
    // }

    result = hackrf_start_rx(device, rx_callback, NULL);
    if( result != HACKRF_SUCCESS ) {
        printf("run_board: hackrf_start_rx() failed: %s (%d)\n", hackrf_error_name(result), result);
        return(-1);
    }

    return(0);
}

inline int config_run_board(uint64_t freq_hz, int gain, int lnaGain, uint8_t amp, void **rf_dev) {
    hackrf_device *dev = NULL;

    (*rf_dev) = dev;

    if (init_board() != 0) {
        return(-1);
    }

    if ( open_board(freq_hz, gain, lnaGain, amp, &dev) != 0 ) {
        (*rf_dev) = dev;
        return(-1);
    }

    (*rf_dev) = dev;
    if ( run_board(dev) != 0 ) {
        return(-1);
    }

    return(0);
}

void stop_close_board(hackrf_device* device){
    if (close_board(device)!=0){
        return;
    }
    exit_board(device);
}

#endif  //#ifdef USE_BLADERF


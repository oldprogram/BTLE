/*************************************************************************
	> File Name: app_board_rx.h
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 13:53:56 CST
 ************************************************************************/

#ifndef _APP_BOARD_RX_H
#define _APP_BOARD_RX_H
#include "../common.h"

inline int config_run_board(uint64_t freq_hz, int gain, int lnaGain, uint8_t amp, void **rf_dev);

#ifdef USE_BLADERF
void stop_close_board(struct bladerf *dev);
int board_set_freq(struct bladerf *dev, uint64_t freq_hz);
#else
void stop_close_board(hackrf_device* device);
int board_set_freq(void *device, uint64_t freq_hz);
#endif

#endif

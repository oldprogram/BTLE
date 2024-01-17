/*************************************************************************
  > File Name: app_board.h
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 15:38:16 CST
 ************************************************************************/

#ifndef _APP_BOARD_H
#define _APP_BOARD_H

int init_board();
int open_board();
int close_board();
void exit_board();
int tx_one_buf(char *buf, int length, int channel_number);

#endif

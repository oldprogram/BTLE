/*************************************************************************
  > File Name: app_tools.h
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 17:42:35 CST
 ************************************************************************/

#ifndef _APP_TOOLS_H
#define _APP_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef int8_t IQ_TYPE;

// time
inline int TimevalDiff(const struct timeval *a, const struct timeval *b);

// 2d
char ** malloc_2d(int num_row, int num_col);
void release_2d(char **items, int num_row);

// print
void disp_bit_in_hex(char *bit, int num_bit);//+R
void disp_bit(char *bit, int num_bit);//+R
void disp_hex(uint8_t *hex, int num_hex);//+R
void disp_hex_in_bit(uint8_t *hex, int num_hex);//+R

// change value format
char* toupper_str(char *input_str, char *output_str);//+R
void octet_hex_to_bit(char *hex, char *bit);
int bit_to_int(char *bit);
void int_to_bit(int n, char *bit);
void int_to_bit(int n, char *bit);//@R
int convert_hex_to_bit(char *hex, char *bit);//+R
void uint32_to_bit_array(uint32_t uint32_in, uint8_t *bit);
void byte_array_to_bit_array(uint8_t *byte_in, int num_byte, uint8_t *bit);

// crc
uint_fast32_t crc_update(uint_fast32_t crc, const void *data, size_t data_len);//+R
uint_fast32_t crc24_byte(uint8_t *byte_in, int num_byte, int init_hex);//+R
void crc24(char *bit_in, int num_bit, char *init_hex, char *crc_result);
uint32_t crc_init_reorder(uint32_t crc_init);//@R
bool crc_check(uint8_t *tmp_byte, int body_len, uint32_t crc_init);//@R

//phy_sample load and save
void save_phy_sample(char *IQ_sample, int num_IQ_sample, char *filename);//T
void save_phy_sample_for_matlab(char *IQ_sample, int num_IQ_sample, char *filename);//T
void save_phy_sample2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename);//R
void load_phy_sample2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename);//R
void save_phy_sample_for_matlab2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename);//R

#endif

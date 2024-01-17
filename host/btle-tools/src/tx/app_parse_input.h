/*************************************************************************
  > File Name: app_parse_input.h
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 14:27:55 CST
 ************************************************************************/

#ifndef _APP_PARSE_INPUT_H
#define _APP_PARSE_INPUT_H

void usage();
int parse_input(int num_input, char** argv, int *num_repeat_return);
int read_items_from_file(int *num_items, char **items_buf, int num_row, char *filename);

#endif

/*************************************************************************
  > File Name: app_parse_input.c
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 14:27:52 CST
 ************************************************************************/
#include "app_calculate_pkt.h"

extern PKT_INFO packets[];

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 使用菜单 
//---------------------------------------------------------------------------------------
void usage() {
    printf("BLE packet generator. Xianjun Jiao. putaoshu@msn.com\n\n");
    printf("Usage:\n");
    printf("btle_tx packet1 packet2 ... packetX ...  rN\n");
    printf("or\n");
    printf("./btle_tx packets.txt\n");
    printf("(packets.txt contains parameters: packet1 ... packetX rN\n");
    printf("\nA packet sequence is composed by packet1 packet2 ... packetX\n");
    printf("rN means that the sequence will be repeated for N times\n");
    printf("packetX is packet descriptor string.\n");
    printf("For the format, see README for detailed information.\n");
}

//---------------------------------------------------------------------------------------
// 对内函数调用的函数 
//---------------------------------------------------------------------------------------
static int get_num_repeat(char *input_str, int *repeat_specific){
    int num_repeat;

    if (input_str[0] == 'r' || input_str[0] == 'R') {
        num_repeat = atol(input_str+1);
        (*repeat_specific) = 1;

        if (strlen(input_str)>1) {
            if (num_repeat < -1) {
                num_repeat = 1;
                printf("Detect num_repeat < -1! (-1 means inf). Set to %d\n", num_repeat);
            } else if (num_repeat == 0) {
                num_repeat = 1;
                if ( input_str[1] == '0') {
                    printf("Detect num_repeat = 0! (-1 means inf). Set to %d\n", num_repeat);
                } else {
                    printf("Detect invalid num_repeat! (-1 means inf). Set to %d\n", num_repeat);
                }
            }
        } else {
            num_repeat = 1;
            printf("num_repeat not specified! (-1 means inf). Set to %d\n", num_repeat);
        }
    } else if (isdigit(input_str[0])) {
        (*repeat_specific) = 0;
        num_repeat = 1;
        printf("num_repeat not specified! (-1 means inf). Set to %d\n", num_repeat);
    } else {
        num_repeat = -2;
        printf("Invalid last parameter! (It should be num_repeat. -1 means inf)\n");
    }

    return(num_repeat);
}


//---------------------------------------------------------------------------------------
// 对外 
//---------------------------------------------------------------------------------------
int parse_input(int num_input, char** argv, int *num_repeat_return){
    int repeat_specific = 0;

    int num_repeat = get_num_repeat(argv[num_input-1], &repeat_specific);
    if (num_repeat == -2) {
        return(-2);
    }

    int num_packet = 0;
    if (repeat_specific == 1){
        num_packet = num_input - 2;
    } else {
        num_packet = num_input - 1;
    }

    printf("num_repeat %d\n", num_repeat);
    printf("num_packet %d\n", num_packet);

    (*num_repeat_return) = num_repeat;

    int i;
    for (i=0; i<num_packet; i++) {

        if (strlen(argv[1+i]) > MAX_NUM_CHAR_CMD-1) {
            printf("Too long packet descriptor of packet %d! Maximum allowed are %d characters\n", i, MAX_NUM_CHAR_CMD-1);
            return(-2);
        }
        strcpy(packets[i].cmd_str, argv[1+i]);
        printf("\npacket %d\n", i);
        if (calculate_pkt_info( &(packets[i]) ) == -1){
            printf("failed!\n");
            return(-2);
        }
        printf("INFO bit:"); disp_bit_in_hex(packets[i].info_bit, packets[i].num_info_bit);
        printf(" PHY bit:"); disp_bit_in_hex(packets[i].phy_bit, packets[i].num_phy_bit);
        printf("PHY SMPL: PHY_bit_for_matlab.txt IQ_sample_for_matlab.txt IQ_sample.txt IQ_sample_byte.txt\n");
        save_phy_sample((char*)(packets[i].info_byte), packets[i].num_info_byte, "info_byte.txt");
        save_phy_sample((char*)(packets[i].phy_byte), packets[i].num_phy_byte, "phy_byte.txt");
        save_phy_sample(packets[i].phy_sample, 2*packets[i].num_phy_sample, "phy_sample.txt");
        save_phy_sample_for_matlab(packets[i].phy_sample, 2*packets[i].num_phy_sample, "IQ_sample_for_matlab.txt");
        save_phy_sample_for_matlab(packets[i].phy_bit, packets[i].num_phy_bit, "PHY_bit_for_matlab.txt");
    }

    return(num_packet);
}

int read_items_from_file(int *num_items, char **items_buf, int num_row, char *filename){

    FILE *fp = fopen(filename, "r");

    char file_line[MAX_NUM_CHAR_CMD*2];

    if (fp == NULL) {
        printf("fopen failed!\n");
        return(-1);
    }

    int num_lines = 0;
    char *p = (char *)12345;

    while( 1 ) {
        memset(file_line, 0, MAX_NUM_CHAR_CMD*2);
        p = fgets(file_line,  (MAX_NUM_CHAR_CMD*2), fp );

        if ( file_line[(MAX_NUM_CHAR_CMD*2)-1] != 0 ) {
            printf("A line is too long!\n");
            fclose(fp);
            return(-1);
        }

        if ( p==NULL ) {
            break;
        }

        if (file_line[0] != '#') {
            if ( (file_line[0] >= 48 && file_line[0] <= 57) || file_line[0] ==114 || file_line[0] == 82 ) { // valid line
                if (strlen(file_line) > (MAX_NUM_CHAR_CMD-1) ) {
                    printf("A line is too long!\n");
                    fclose(fp);
                    return(-1);
                } else {

                    if (num_lines == (num_row-1) ) {
                        printf("Too many lines!\n");
                        fclose(fp);
                        return(-1);
                    }

                    strcpy(items_buf[num_lines + 1], file_line);
                    num_lines++;
                }
            }
        }

        if (feof(fp)) {
            break;
        }
    }

    fclose(fp);

    (*num_items) = num_lines + 1;

    return(0);
}



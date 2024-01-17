/*************************************************************************
  > File Name: app_calculate_pkt.c
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 20:01:24 CST
 ************************************************************************/
#include "app_ble_tools.h"


#define DEFAULT_SPACE_MS (199)

static char *AD_TYPE_STR[] = {
    "FLAGS",
    "LOCAL_NAME08",
    "LOCAL_NAME09",
    "TXPOWER",
    "SERVICE02",
    "SERVICE03",
    "SERVICE04",
    "SERVICE05",
    "SERVICE06",
    "SERVICE07",
    "SERVICE_SOLI14",
    "SERVICE_SOLI15",
    "SERVICE_DATA",
    "MANUF_DATA",
    "CONN_INTERVAL",
    "SPACE"
};

static const int AD_TYPE_VAL[] = {
    0x01,  //"FLAGS",
    0x08,  //"LOCAL_NAME08",
    0x09,  //"LOCAL_NAME09",
    0x0A,  //"TXPOWER",
    0x02,  //"SERVICE02",
    0x03,  //"SERVICE03",
    0x04,  //"SERVICE04",
    0x05,  //"SERVICE05",
    0x06,  //"SERVICE06",
    0x07,  //"SERVICE07",
    0x14,  //"SERVICE_SOLI14",
    0x15,  //"SERVICE_SOLI15",
    0x16,  //"SERVICE_DATA",
    0xFF,  //"MANUF_DATA",
    0x12   //"CONN_INTERVAL",
};

static char tmp_str[MAX_NUM_CHAR_CMD];
static char tmp_str1[MAX_NUM_CHAR_CMD];
static float tmp_phy_bit_over_sampling[MAX_NUM_PHY_SAMPLE + 2*LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL];
//static float tmp_phy_bit_over_sampling1[MAX_NUM_PHY_SAMPLE];


//---------------------------------------------------------------------------------------
// 对外函数调用的函数--tools 
//---------------------------------------------------------------------------------------
#if 1 // fixed point version
#include "gauss_cos_sin_table.h"

int gen_sample_from_phy_byte(uint8_t *byte,  int8_t *sample, int num_byte) {
    int num_bit = num_byte*8;
    int num_sample = (num_bit*SAMPLE_PER_SYMBOL)+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL);

    int8_t *tmp_phy_bit_over_sampling_int8 = (int8_t *)tmp_phy_bit_over_sampling;

    int i, j, overall_bit_idx, sub_bit_idx;

    for (i=0; i<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1); i++) {
        tmp_phy_bit_over_sampling_int8[i] = 0;
    }
    for (i=(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1+num_bit*SAMPLE_PER_SYMBOL); i<(2*LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-2+num_bit*SAMPLE_PER_SYMBOL); i++) {
        tmp_phy_bit_over_sampling_int8[i] = 0;
    }
    for(j=0; j<num_byte; j++) {
        sub_bit_idx = 0;
        for (i=0; i<(8*SAMPLE_PER_SYMBOL); i = i + SAMPLE_PER_SYMBOL) {
            overall_bit_idx = j*8*SAMPLE_PER_SYMBOL + i;
            (*(int*)(&(tmp_phy_bit_over_sampling_int8[overall_bit_idx+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1)]))) = 0xff & (( (byte[j]>>sub_bit_idx & 0x01) ) * 2 - 1);
            sub_bit_idx++;
        }
    }

    int16_t tmp = 0;
    sample[0] = cos_table_int8[tmp];
    sample[1] = sin_table_int8[tmp];

    int len_conv_result = num_sample - 1;
    for (i=0; i<len_conv_result; i++) {
        int16_t acc = 0;
        for (j=3; j<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-4); j++) {
            acc = acc + gauss_coef_int8[(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL)-j-1]*tmp_phy_bit_over_sampling_int8[i+j];
        }

        tmp = (tmp + acc)&1023;
        sample[(i+1)*2 + 0] = cos_table_int8[tmp];
        sample[(i+1)*2 + 1] = sin_table_int8[tmp];
    }

    return(num_sample);
}

int gen_sample_from_phy_bit(char *bit, char *sample, int num_bit) {
    int num_sample = (num_bit*SAMPLE_PER_SYMBOL)+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL);

    int8_t *tmp_phy_bit_over_sampling_int8 = (int8_t *)tmp_phy_bit_over_sampling;
    //int16_t *tmp_phy_bit_over_sampling1_int16 = (int16_t *)tmp_phy_bit_over_sampling1;

    int i, j;

    for (i=0; i<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1); i++) {
        tmp_phy_bit_over_sampling_int8[i] = 0;
    }
    for (i=(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1+num_bit*SAMPLE_PER_SYMBOL); i<(2*LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-2+num_bit*SAMPLE_PER_SYMBOL); i++) {
        tmp_phy_bit_over_sampling_int8[i] = 0;
    }
    for (i=0; i<(num_bit*SAMPLE_PER_SYMBOL); i++) {
        if (i%SAMPLE_PER_SYMBOL == 0) {
            tmp_phy_bit_over_sampling_int8[i+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1)] = ( bit[i/SAMPLE_PER_SYMBOL] ) * 2 - 1;
        } else {
            tmp_phy_bit_over_sampling_int8[i+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1)] = 0;
        }
    }

#if 1 // new method

    int16_t tmp = 0;
    sample[0] = cos_table_int8[tmp];
    sample[1] = sin_table_int8[tmp];

    int len_conv_result = num_sample - 1;
    for (i=0; i<len_conv_result; i++) {
        int16_t acc = 0;
        for (j=3; j<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-4); j++) {
            acc = acc + gauss_coef_int8[(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL)-j-1]*tmp_phy_bit_over_sampling_int8[i+j];
        }

        tmp = (tmp + acc)&1023;
        sample[(i+1)*2 + 0] = cos_table_int8[tmp];
        sample[(i+1)*2 + 1] = sin_table_int8[tmp];
    }

#else // old method

    int len_conv_result = num_sample - 1;
    for (i=0; i<len_conv_result; i++) {
        int16_t acc = 0;
        for (j=0; j<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL); j++) {
            acc = acc + gauss_coef_int16[(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL)-j-1]*tmp_phy_bit_over_sampling_int16[i+j];
        }
        tmp_phy_bit_over_sampling1_int16[i] = acc;
    }

    int16_t tmp = 0;
    sample[0] = cos_table_int8[tmp];
    sample[1] = sin_table_int8[tmp];
    for (i=1; i<num_sample; i++) {
        tmp = (tmp + tmp_phy_bit_over_sampling1_int16[i-1])&1023;
        sample[i*2 + 0] = cos_table_int8[tmp];
        sample[i*2 + 1] = sin_table_int8[tmp];
    }

#endif

    return(num_sample);
}

#else // float point version

int gen_sample_from_phy_bit(char *bit, char *sample, int num_bit) {
    int num_sample = (num_bit*SAMPLE_PER_SYMBOL)+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL);

    int i, j;

    for (i=0; i<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1); i++) {
        tmp_phy_bit_over_sampling[i] = 0.0;
    }
    for (i=(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1+num_bit*SAMPLE_PER_SYMBOL); i<(2*LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-2+num_bit*SAMPLE_PER_SYMBOL); i++) {
        tmp_phy_bit_over_sampling[i] = 0.0;
    }
    for (i=0; i<(num_bit*SAMPLE_PER_SYMBOL); i++) {
        if (i%SAMPLE_PER_SYMBOL == 0) {
            tmp_phy_bit_over_sampling[i+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1)] = (float)( bit[i/SAMPLE_PER_SYMBOL] ) * 2.0 - 1.0;
        } else {
            tmp_phy_bit_over_sampling[i+(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL-1)] = 0.0;
        }
    }

    int len_conv_result = num_sample - 1;
    for (i=0; i<len_conv_result; i++) {
        float acc = 0;
        for (j=0; j<(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL); j++) {
            acc = acc + gauss_coef[(LEN_GAUSS_FILTER*SAMPLE_PER_SYMBOL)-j-1]*tmp_phy_bit_over_sampling[i+j];
        }
        tmp_phy_bit_over_sampling1[i] = acc;
    }

    float tmp = 0;
    sample[0] = (char)round( cos(tmp)*(float)AMPLITUDE );
    sample[1] = (char)round( sin(tmp)*(float)AMPLITUDE );
    for (i=1; i<num_sample; i++) {
        tmp = tmp + (M_PI*MOD_IDX)*tmp_phy_bit_over_sampling1[i-1]/((float)SAMPLE_PER_SYMBOL);
        sample[i*2 + 0] = (char)round( cos(tmp)*(float)AMPLITUDE );
        sample[i*2 + 1] = (char)round( sin(tmp)*(float)AMPLITUDE );
    }

    return(num_sample);
}

#endif

//---------------------------------------------------------------------------------------
// 对外函数调用的函数--get field 
//---------------------------------------------------------------------------------------
char* get_next_field(char *str_input, char *p_out, char *seperator, int size_of_p_out) {
    char *tmp_p = strstr(str_input, seperator);

    if (tmp_p == str_input){
        printf("Duplicated seperator %s!\n", seperator);
        return(NULL);
    } else if (tmp_p == NULL) {
        if (strlen(str_input) > (size_of_p_out-1) ) {
            printf("Number of input exceed output buffer!\n");
            return(NULL);
        } else {
            strcpy(p_out, str_input);
            return(str_input);
        }
    }

    if ( (tmp_p-str_input)>(size_of_p_out-1) ) {
        printf("Number of input exceed output buffer!\n");
        return(NULL);
    }

    char *p;
    for (p=str_input; p<tmp_p; p++) {
        p_out[p-str_input] = (*p);
    }
    p_out[p-str_input] = 0;

    return(tmp_p+1);
}

char* get_next_field_value(char *current_p, int *value_return, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }

    (*value_return) = atol(tmp_str);

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}

char* get_next_field_bit(char *current_p, char *bit_return, int *num_bit_return, int stream_flip, int octet_limit, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    // stream_flip: 0: normal order; 1: flip octets order in sequence
    int i;
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }
    int num_hex = strlen(tmp_str);
    while(tmp_str[num_hex-1]<=32 || tmp_str[num_hex-1]>=127) {
        num_hex--;
    }

    if (num_hex%2 != 0) {
        printf("get_next_field_bit: Half octet is encountered! num_hex %d\n", num_hex);
        printf("%s\n", tmp_str);
        (*return_flag) = -1;
        return(next_p);
    }

    if ( num_hex>(octet_limit*2) ) {
        printf("Too many octets! Maximum allowed is %d\n", octet_limit);
        (*return_flag) = -1;
        return(next_p);
    }
    if (num_hex <= 1) { // NULL data
        (*return_flag) = 0;
        (*num_bit_return) = 0;
        return(next_p);
    }

    int num_bit_tmp;
    if (stream_flip == 1) {
        strcpy(tmp_str1, tmp_str);
        for (i=0; i<num_hex; i=i+2) {
            tmp_str[num_hex-i-2] = tmp_str1[i];
            tmp_str[num_hex-i-1] = tmp_str1[i+1];
        }
    }
    num_bit_tmp = convert_hex_to_bit(tmp_str, bit_return);
    if ( num_bit_tmp == -1 ) {
        (*return_flag) = -1;
        return(next_p);
    }
    (*num_bit_return) = num_bit_tmp;

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}

char* get_next_field_name(char *current_p, char *name, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }
    if (strcmp(toupper_str(tmp_str, tmp_str), name) != 0) {
        //    printf("%s field is expected!\n", name);
        (*return_flag) = -1;
        return(next_p);
    }

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}

char* get_next_field_char(char *current_p, char *bit_return, int *num_bit_return, int stream_flip, int octet_limit, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    // stream_flip: 0: normal order; 1: flip octets order in sequence
    int i;
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }
    int num_hex = strlen(tmp_str);
    while(tmp_str[num_hex-1]<=32 || tmp_str[num_hex-1]>=127) {
        num_hex--;
    }

    if ( num_hex>octet_limit ) {
        printf("Too many octets(char)! Maximum allowed is %d\n", octet_limit);
        (*return_flag) = -1;
        return(next_p);
    }
    if (num_hex <= 1) { // NULL data
        (*return_flag) = 0;
        (*num_bit_return) = 0;
        return(next_p);
    }

    if (stream_flip == 1) {
        for (i=0; i<num_hex; i++) {
            int_to_bit(tmp_str[num_hex-i-1], bit_return + 8*i);
        }
    } else {
        for (i=0; i<num_hex; i++) {
            int_to_bit(tmp_str[i], bit_return + 8*i);
        }
    }

    (*num_bit_return) = 8*num_hex;

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}

char* get_next_field_hex(char *current_p, char *hex_return, int stream_flip, int octet_limit, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    // stream_flip: 0: normal order; 1: flip octets order in sequence
    int i;
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }
    int num_hex = strlen(tmp_str);
    while(tmp_str[num_hex-1]<=32 || tmp_str[num_hex-1]>=127) {
        num_hex--;
    }

    if (num_hex%2 != 0) {
        printf("get_next_field_hex: Half octet is encountered! num_hex %d\n", num_hex);
        printf("%s\n", tmp_str);
        (*return_flag) = -1;
        return(next_p);
    }

    if ( num_hex>(octet_limit*2) ) {
        printf("Too many octets! Maximum allowed is %d\n", octet_limit);
        (*return_flag) = -1;
        return(next_p);
    }

    if (stream_flip == 1) {
        strcpy(tmp_str1, tmp_str);
        for (i=0; i<num_hex; i=i+2) {
            tmp_str[num_hex-i-2] = tmp_str1[i];
            tmp_str[num_hex-i-1] = tmp_str1[i+1];
        }
    }

    strcpy(hex_return, tmp_str);
    hex_return[num_hex] = 0;

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}

char* get_next_field_bit_part_flip(char *current_p, char *bit_return, int *num_bit_return, int stream_flip, int octet_limit, int *return_flag) {
    // return_flag: -1 failed; 0 success; 1 success and this is the last field
    // stream_flip: 0: normal order; 1: flip octets order in sequence
    int i;
    char *next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if (next_p == NULL) {
        (*return_flag) = -1;
        return(next_p);
    }
    int num_hex = strlen(tmp_str);
    while(tmp_str[num_hex-1]<=32 || tmp_str[num_hex-1]>=127) {
        num_hex--;
    }

    if (num_hex%2 != 0) {
        printf("get_next_field_bit: Half octet is encountered! num_hex %d\n", num_hex);
        printf("%s\n", tmp_str);
        (*return_flag) = -1;
        return(next_p);
    }

    if ( num_hex>(octet_limit*2) ) {
        printf("Too many octets! Maximum allowed is %d\n", octet_limit);
        (*return_flag) = -1;
        return(next_p);
    }
    if (num_hex <= 1) { // NULL data
        (*return_flag) = 0;
        (*num_bit_return) = 0;
        return(next_p);
    }

    int num_bit_tmp;

    num_hex = 2*stream_flip;
    strcpy(tmp_str1, tmp_str);
    for (i=0; i<num_hex; i=i+2) {
        tmp_str[num_hex-i-2] = tmp_str1[i];
        tmp_str[num_hex-i-1] = tmp_str1[i+1];
    }

    num_bit_tmp = convert_hex_to_bit(tmp_str, bit_return);
    if ( num_bit_tmp == -1 ) {
        (*return_flag) = -1;
        return(next_p);
    }
    (*num_bit_return) = num_bit_tmp;

    if (next_p == current_p) {
        (*return_flag) = 1;
        return(next_p);
    }

    (*return_flag) = 0;
    return(next_p);
}


char *get_next_field_name_char(char *input_p, char *name, char *out_bit, int *num_bit, int flip_flag, int octet_limit, int *ret_last){
    // ret_last: -1 failed; 0 success; 1 success and this is the last field
    int ret;
    char *current_p = input_p;

    char *next_p = get_next_field_name(current_p, name, &ret);
    if (ret != 0) { // failed or the last
        (*ret_last) = -1;
        return(NULL);
    }

    current_p = next_p;
    next_p = get_next_field_char(current_p, out_bit, num_bit, flip_flag, octet_limit, &ret);
    (*ret_last) = ret;
    if (ret == -1) { // failed
        return(NULL);
    }

    return(next_p);
}

char *get_next_field_name_hex(char *input_p, char *name, char *out_hex, int flip_flag, int octet_limit, int *ret_last){
    // ret_last: -1 failed; 0 success; 1 success and this is the last field
    int ret;
    char *current_p = input_p;

    char *next_p = get_next_field_name(current_p, name, &ret);
    if (ret != 0) { // failed or the last
        (*ret_last) = -1;
        return(NULL);
    }

    current_p = next_p;
    next_p = get_next_field_hex(current_p, out_hex, flip_flag, octet_limit, &ret);
    (*ret_last) = ret;
    if (ret == -1) { // failed
        return(NULL);
    }

    return(next_p);
}

char *get_next_field_name_bit_part_flip(char *input_p, char *name, char *out_bit, int *num_bit, int flip_flag, int octet_limit, int *ret_last){
    // ret_last: -1 failed; 0 success; 1 success and this is the last field
    int ret;
    char *current_p = input_p;

    char *next_p = get_next_field_name(current_p, name, &ret);
    if (ret != 0) { // failed or the last
        (*ret_last) = -1;
        return(NULL);
    }

    current_p = next_p;
    next_p = get_next_field_bit_part_flip(current_p, out_bit, num_bit, flip_flag, octet_limit, &ret);
    (*ret_last) = ret;
    if (ret == -1) { // failed
        return(NULL);
    }

    return(next_p);
}

char *get_next_field_name_bit(char *input_p, char *name, char *out_bit, int *num_bit, int flip_flag, int octet_limit, int *ret_last){
    // ret_last: -1 failed; 0 success; 1 success and this is the last field
    int ret;
    char *current_p = input_p;

    char *next_p = get_next_field_name(current_p, name, &ret);
    if (ret != 0) { // failed or the last
        (*ret_last) = -1;
        return(NULL);
    }

    current_p = next_p;
    next_p = get_next_field_bit(current_p, out_bit, num_bit, flip_flag, octet_limit, &ret);
    (*ret_last) = ret;
    if (ret == -1) { // failed
        return(NULL);
    }

    return(next_p);
}


char *get_next_field_name_value(char *input_p, char *name, int *val, int *ret_last){
    // ret_last: -1 failed; 0 success; 1 success and this is the last field
    int ret;
    char *current_p = input_p;

    char *next_p = get_next_field_name(current_p, name, &ret);
    if (ret != 0) { // failed or the last
        (*ret_last) = -1;
        return(NULL);
    }

    current_p = next_p;
    next_p = get_next_field_value(current_p, val, &ret);
    (*ret_last) = ret;
    if (ret == -1) { // failed
        return(NULL);
    }

    return(next_p);
}


//---------------------------------------------------------------------------------------
// 对外函数调用的函数--calculate_sample 
//---------------------------------------------------------------------------------------
int calculate_sample_for_RAW(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 39-RAW-AAD6BE898E5F134B5D86F2999CC3D7DF5EDF15DEE39AA2E5D0728EB68B0E449B07C547B80EAA8DD257A0E5EACB0B-SPACE-1000
    char *current_p;
    int ret;

    pkt->num_info_bit = 0;
    printf("num_info_bit %d\n", pkt->num_info_bit);

    current_p = pkt_str;
    current_p = get_next_field_bit(current_p, pkt->phy_bit, &(pkt->num_phy_bit), 0, MAX_NUM_PHY_BYTE, &ret);
    if (ret == -1) {
        return(-1);
    }
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    if (ret==1) {
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) {
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_DISCOVERY(char *pkt_str, PKT_INFO*pkt) {
    // example
    // ./btle_tx 37-DISCOVERY-TxAdd-1-RxAdd-0-AdvA-010203040506-FLAGS-02-LOCALNAME09-CA-TXPOWER-03-SERVICE03-180D1810-SERVICEDATA-180D40-MANUFDATA-0001FF-CONN_INTERVAL-0006 (-SERVICESOLI-0123)
    // FLAGS: 0x01 LE Limited Discoverable Mode; 0x02 LE General Discoverable Mode
    // SERVICE:
    // 0x02 16-bit Service UUIDs More 16-bit UUIDs available
    // 0x03 16-bit Service UUIDs Complete list of 16-bit UUIDs available
    // 0x04 32-bit Service UUIDs More 32-bit UUIDs available
    // 0x05 32-bit Service UUIDs Complete list of 32-bit UUIDs available
    // 0x06 128-bit Service UUIDs More 128-bit UUIDs available
    // 0x07 128-bit Service UUIDs Complete list of 128-bit UUIDs available
    char *current_p;
    int ret, num_bit_tmp, num_octet_tmp, i, j;

    pkt->num_info_bit = 0;
    pkt->num_info_byte = 0;

    // gen preamble and access address
    num_bit_tmp = convert_hex_to_bit("AA", pkt->info_bit);
    num_octet_tmp = 1;
    pkt->info_byte[0] = 0xAA;

    printf("AA %d\n", num_bit_tmp);
    disp_bit(pkt->info_bit, num_bit_tmp);
    disp_hex_in_bit(pkt->info_byte, num_octet_tmp);

    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;
    pkt->num_info_byte = pkt->num_info_byte + num_octet_tmp;

    num_bit_tmp = convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);
    num_octet_tmp = 4;
    (pkt->info_byte + pkt->num_info_byte)[0] = 0xD6;
    (pkt->info_byte + pkt->num_info_byte)[1] = 0xBE;
    (pkt->info_byte + pkt->num_info_byte)[2] = 0x89;
    (pkt->info_byte + pkt->num_info_byte)[3] = 0x8E;

    printf("D6BE898E %d\n", num_bit_tmp);
    disp_bit(pkt->info_bit + pkt->num_info_bit, num_bit_tmp);
    disp_hex_in_bit(pkt->info_byte + pkt->num_info_byte, num_octet_tmp);

    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;
    pkt->num_info_byte = pkt->num_info_byte + num_octet_tmp;

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length
    pkt->num_info_byte = pkt->num_info_byte + 2;

    // get AdvA
    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    num_octet_tmp = 6;
    (pkt->info_byte + pkt->num_info_byte)[0] = 0x06;
    (pkt->info_byte + pkt->num_info_byte)[1] = 0x05;
    (pkt->info_byte + pkt->num_info_byte)[2] = 0x04;
    (pkt->info_byte + pkt->num_info_byte)[3] = 0x03;
    (pkt->info_byte + pkt->num_info_byte)[4] = 0x02;
    (pkt->info_byte + pkt->num_info_byte)[5] = 0x01;
    printf("ADVA buffer begin from %d\n",  pkt->num_info_byte);

    printf("010203040506 %d\n", num_bit_tmp);
    disp_bit(pkt->info_bit + pkt->num_info_bit, num_bit_tmp);
    disp_hex_in_bit(pkt->info_byte + pkt->num_info_byte, num_octet_tmp);

    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;
    pkt->num_info_byte = pkt->num_info_byte + num_octet_tmp;

    // then get AdvData. maximum 31 octets
    int octets_left_room = 31;
    while(ret == 0) {
        // get name of next field
        for(i=0; i<NUM_AD_TYPE; i++) {
            get_next_field_name(current_p, AD_TYPE_STR[i], &ret);
            if (ret == 0) {
                break;
            }
        }

        //printf("i %d %s\n", i, AD_TYPE_STR[i]);

        if (ret != 0) {
            printf("Get name of AD TYPE failed. i %d ret %d NUM_AD_TYPE %d\n", i, ret, NUM_AD_TYPE);
            return(-1);
        }

        if (i==(NUM_AD_TYPE-1)) { // it is SPACE field, should be processed later on.
            break;
        }

        // get data followed according to AD TYPE
        // except LOCAL_NAME, all others are values.
        octets_left_room = octets_left_room  - 2; // 2 -- length and AD_TYPE
        if (i == LOCAL_NAME08 || i == LOCAL_NAME09) {
            current_p = get_next_field_name_char(current_p, AD_TYPE_STR[i], pkt->info_bit+ 2*8 + pkt->num_info_bit, &num_bit_tmp, 0, octets_left_room, &ret);

            num_octet_tmp = num_bit_tmp/8;

            //sprintf((char*)(pkt->info_byte + 2 + pkt->num_info_byte), "CA1308 11950 22.626 113.823 8"); // this is fake. let us use real.
            for(j=0; j<num_octet_tmp; j++) {
                pkt->info_byte[2+pkt->num_info_byte+j] = bit_to_int(pkt->info_bit+ 2*8 + pkt->num_info_bit + j*8);
            }
            pkt->info_byte[2+pkt->num_info_byte+j] = 0;

            printf("display buffer begin from %d\n",  2 + pkt->num_info_byte);

            //printf("CA1308 11950 22.626 113.823 8 %d\n", num_bit_tmp); // this is fake. let us use real.
            printf("%s %d\n", (char *)(pkt->info_byte + 2+ pkt->num_info_byte), num_bit_tmp);

            disp_bit(pkt->info_bit + 2*8 + pkt->num_info_bit, num_bit_tmp);
            disp_hex_in_bit(pkt->info_byte + 2+ pkt->num_info_byte, num_octet_tmp);

        } else if (i == SERVICE02 || i == SERVICE03 || i == SERVICE04 || i == SERVICE05 || i == SERVICE06 || i == SERVICE07) {
            current_p = get_next_field_name_bit(current_p, AD_TYPE_STR[i], pkt->info_bit+ 2*8 + pkt->num_info_bit, &num_bit_tmp, 1, octets_left_room, &ret);
        } else if (i == SERVICE_DATA) {
            current_p = get_next_field_name_bit_part_flip(current_p, AD_TYPE_STR[i], pkt->info_bit+ 2*8 + pkt->num_info_bit, &num_bit_tmp, 2, octets_left_room, &ret);
        } else {
            current_p = get_next_field_name_bit(current_p, AD_TYPE_STR[i], pkt->info_bit+ 2*8 + pkt->num_info_bit, &num_bit_tmp, 0, octets_left_room, &ret);
        }
        if (ret == -1) { // failed
            return(-1);
        }

        // fill length and AD_TYPE octets
        num_octet_tmp = num_bit_tmp/8;
        int_to_bit(num_octet_tmp+1, pkt->info_bit + pkt->num_info_bit);
        (pkt->info_byte + pkt->num_info_byte)[0] = num_octet_tmp+1;
        int_to_bit(AD_TYPE_VAL[i], pkt->info_bit + pkt->num_info_bit + 8 );
        (pkt->info_byte + 1 + pkt->num_info_byte)[0] = AD_TYPE_VAL[i];

        printf("length and AD_TYPE %d\n", 16);
        disp_bit(pkt->info_bit + pkt->num_info_bit,  2*8);
        disp_hex_in_bit(pkt->info_byte + pkt->num_info_byte,  2);

        pkt->num_info_bit = pkt->num_info_bit + 2*8 + num_bit_tmp;
        pkt->num_info_byte = pkt->num_info_byte + 2 + num_octet_tmp;

        octets_left_room = octets_left_room  - num_octet_tmp;
    }

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);
    printf("num_info_byte %d\n", pkt->num_info_byte);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);
    fill_adv_pdu_header_byte(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_byte+5);

    printf("before crc24\n");
    disp_bit_in_hex(pkt->info_bit, pkt->num_info_bit);
    disp_hex(pkt->info_byte, pkt->num_info_byte);

    crc24_and_scramble_byte_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    gen_sample_from_phy_byte(pkt->phy_byte, pkt->phy_sample1, pkt->num_phy_byte);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_IBEACON(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-IBEACON-AdvA-010203040506-UUID-B9407F30F5F8466EAFF925556B57FE6D-Major-0008-Minor-0009-TxPower-C5-Space-100 r10
    // UUID indicates Estimote
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    int txadd = 1;
    int rxadd = 0;
    int payload_len = 36;
    fill_adv_pdu_header(ADV_IND, txadd, rxadd, payload_len, pkt->info_bit+5*8);
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length
    printf("payload_len %d\n", payload_len);

    // get AdvA
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // set fixed ibeacon prefix
    num_bit_tmp = convert_hex_to_bit("02011A1AFF4C000215", pkt->info_bit+pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get UUID
    current_p = get_next_field_name_bit(current_p, "UUID", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 16, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get major
    current_p = get_next_field_name_bit(current_p, "MAJOR", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get minor
    current_p = get_next_field_name_bit(current_p, "MINOR", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get tx power
    current_p = get_next_field_name_bit(current_p, "TXPOWER", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 1, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    printf("num_info_bit %d\n", pkt->num_info_bit);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_ADV_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-ADV_IND-TxAdd-1-RxAdd-0-AdvA-010203040506-AdvData-00112233445566778899AABBCCDDEEFF
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get AdvA and AdvData
    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "ADVDATA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 31, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_ADV_DIRECT_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-ADV_DIRECT_IND-TxAdd-1-RxAdd-0-AdvA-010203040506-InitA-0708090A0B0C
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get AdvA and InitA
    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "INITA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_ADV_NONCONN_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-ADV_NONCONN_IND-TxAdd-1-RxAdd-0-AdvA-010203040506-AdvData-00112233445566778899AABBCCDDEEFF
    return( calculate_sample_for_ADV_IND(pkt_str, pkt) );
}

int calculate_sample_for_ADV_SCAN_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-ADV_SCAN_IND-TxAdd-1-RxAdd-0-AdvA-010203040506-AdvData-00112233445566778899AABBCCDDEEFF
    return( calculate_sample_for_ADV_IND(pkt_str, pkt) );
}
int calculate_sample_for_SCAN_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-SCAN_REQ-TxAdd-1-RxAdd-0-ScanA-010203040506-AdvA-0708090A0B0C
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get ScanA and AdvA
    current_p = get_next_field_name_bit(current_p, "SCANA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_SCAN_RSP(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-SCAN_RSP-TxAdd-1-RxAdd-0-AdvA-010203040506-ScanRspData-00112233445566778899AABBCCDDEEFF
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get AdvA and ScanRspData
    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "SCANRSPDATA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 31, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_CONNECT_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // ./btle_tx 37-CONNECT_REQ-TxAdd-1-RxAdd-0-InitA-010203040506-AdvA-0708090A0B0C-AA-01020304-CRCInit-050607-WinSize-08-WinOffset-090A-Interval-0B0C-Latency-0D0E-Timeout-0F00-ChM-0102030405-Hop-3-SCA-4
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble and access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("D6BE898E", pkt->info_bit + pkt->num_info_bit);

    // get txadd and rxadd
    current_p = pkt_str;
    int txadd, rxadd;
    current_p = get_next_field_name_value(current_p, "TXADD", &txadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "RXADD", &rxadd, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get InitA and AdvA
    current_p = get_next_field_name_bit(current_p, "INITA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "ADVA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 6, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get AA CRCInit WinSize WinOffset Interval Latency Timeout ChM Hop SCA
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "CRCINIT", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 3, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "WINSIZE", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 1, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "WINOFFSET", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "INTERVAL", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "LATENCY", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "TIMEOUT", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "CHM", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 5, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int hop;
    current_p = get_next_field_name_value(current_p, "HOP", &hop, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 5;

    int sca;
    current_p = get_next_field_name_value(current_p, "SCA", &sca, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 3;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_adv_pdu_header(pkt->pkt_type, txadd, rxadd, payload_len, pkt->info_bit+5*8);
    fill_hop_sca(hop, sca, pkt->info_bit+pkt->num_info_bit-8);

    crc24_and_scramble_to_gen_phy_bit("555555", pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_DATA(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_DATA-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-DATA-X-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // get DATA
    current_p = get_next_field_name_bit(current_p, "DATA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 31, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_CONNECTION_UPDATE_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_CONNECTION_UPDATE_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-Instant-0000-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get WinSize WinOffset Interval Latency Timeout Instant
    current_p = get_next_field_name_bit(current_p, "WINSIZE", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 1, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;
    current_p = get_next_field_name_bit(current_p, "WINOFFSET", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "INTERVAL", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "LATENCY", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "TIMEOUT", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "INSTANT", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_CHANNEL_MAP_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_CHANNEL_MAP_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-ChM-1FFFFFFFFF-Instant-0001-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get ChM Instant
    current_p = get_next_field_name_bit(current_p, "CHM", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 5, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;
    current_p = get_next_field_name_bit(current_p, "INSTANT", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_TERMINATE_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_TERMINATE_IND-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-ErrorCode-00-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get ErrorCode
    current_p = get_next_field_name_bit(current_p, "ERRORCODE", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 1, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_ENC_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_ENC_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-Rand-0102030405060708-EDIV-090A-SKDm-0102030405060708-IVm-090A0B0C-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get Rand EDIV SKDm IVm
    current_p = get_next_field_name_bit(current_p, "RAND", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 8, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "EDIV", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "SKDM", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 8, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "IVM", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_ENC_RSP(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_ENC_RSP-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-SKDs-0102030405060708-IVs-01020304-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get SKDs IVs
    current_p = get_next_field_name_bit(current_p, "SKDS", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 8, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "IVS", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_START_ENC_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_START_ENC_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get NO PAYLOAD
    // ....

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_START_ENC_RSP(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_START_ENC_RSP-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-CRCInit-A77B22-Space-100

    return( calculate_sample_for_LL_START_ENC_REQ(pkt_str, pkt) );
}

int calculate_sample_for_LL_UNKNOWN_RSP(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_UNKNOWN_RSP-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-UnknownType-01-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get UnknownType
    current_p = get_next_field_name_bit(current_p, "UNKNOWNTYPE", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 1, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_FEATURE_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_FEATURE_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-FeatureSet-0102030405060708-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get FeatureSet
    current_p = get_next_field_name_bit(current_p, "FEATURESET", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 8, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_FEATURE_RSP(char *pkt_str, PKT_INFO *pkt) {

    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_FEATURE_RSP-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-FeatureSet-0102030405060708-CRCInit-A77B22-Space-100

    return(calculate_sample_for_LL_FEATURE_REQ(pkt_str, pkt));
}

int calculate_sample_for_LL_PAUSE_ENC_REQ(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_PAUSE_ENC_REQ-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-CRCInit-A77B22-Space-100
    return( calculate_sample_for_LL_START_ENC_REQ(pkt_str, pkt) );
}

int calculate_sample_for_LL_PAUSE_ENC_RSP(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_PAUSE_ENC_RSP-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-CRCInit-A77B22-Space-100
    return( calculate_sample_for_LL_START_ENC_REQ(pkt_str, pkt) );
}

int calculate_sample_for_LL_VERSION_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_VERSION_IND-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-VersNr-01-CompId-0203-SubVersNr-0405-CRCInit-A77B22-Space-100
    char *current_p;
    int ret, num_bit_tmp;

    pkt->num_info_bit = 0;

    // gen preamble (may be changed later according to access address
    pkt->num_info_bit = pkt->num_info_bit + convert_hex_to_bit("AA", pkt->info_bit);

    // get access address
    current_p = pkt_str;
    current_p = get_next_field_name_bit(current_p, "AA", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 4, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    if ( (*(pkt->info_bit+pkt->num_info_bit) ) == 1 ) {
        convert_hex_to_bit("55", pkt->info_bit);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    // get LLID NESN SN MD
    int llid, nesn, sn, md;
    current_p = get_next_field_name_value(current_p, "LLID", &llid, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "NESN", &nesn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "SN", &sn, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }

    current_p = get_next_field_name_value(current_p, "MD", &md, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + 16; // 16 is header length

    // fill opcode
    get_opcode(pkt->pkt_type, pkt->info_bit + pkt->num_info_bit);
    pkt->num_info_bit = pkt->num_info_bit + 8; // 8 is opcode

    // get VersNr CompId SubVersNr
    current_p = get_next_field_name_bit(current_p, "VERSNR", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 0, 1, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "COMPID", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    current_p = get_next_field_name_bit(current_p, "SUBVERSNR", pkt->info_bit+pkt->num_info_bit, &num_bit_tmp, 1, 2, &ret);
    if (ret != 0) { // failed or the last
        return(-1);
    }
    pkt->num_info_bit = pkt->num_info_bit + num_bit_tmp;

    int payload_len = (pkt->num_info_bit/8) - 7;
    printf("payload_len %d\n", payload_len);
    printf("num_info_bit %d\n", pkt->num_info_bit);

    fill_data_pdu_header(llid, nesn, sn, md, payload_len, pkt->info_bit+5*8);

    // get CRC init
    char crc_init[7];
    current_p = get_next_field_name_hex(current_p, "CRCINIT", crc_init, 0, 3, &ret);
    if (ret == -1) { // failed
        return(-1);
    }
    crc24_and_scramble_to_gen_phy_bit(crc_init, pkt);
    printf("num_phy_bit %d\n", pkt->num_phy_bit);

    pkt->num_phy_sample = gen_sample_from_phy_bit(pkt->phy_bit, pkt->phy_sample, pkt->num_phy_bit);
    printf("num_phy_sample %d\n", pkt->num_phy_sample);

    // get space value
    if (ret==1) { // if space value not present
        pkt->space = DEFAULT_SPACE_MS;
        printf("space %d\n", pkt->space);
        return(0);
    }

    int space;
    current_p = get_next_field_name_value(current_p, "SPACE", &space, &ret);
    if (ret == -1) { // failed
        return(-1);
    }

    if (space <= 0) {
        printf("Invalid space!\n");
        return(-1);
    }

    pkt->space = space;
    printf("space %d\n", pkt->space);

    return(0);
}

int calculate_sample_for_LL_REJECT_IND(char *pkt_str, PKT_INFO *pkt) {
    // example
    // Connection establishment (http://processors.wiki.ti.com/index.php/BLE_sniffer_guide)
    // ./btle_tx 37-ADV_IND-TxAdd-0-RxAdd-0-AdvA-90D7EBB19299-AdvData-0201050702031802180418-Space-100  37-CONNECT_REQ-TxAdd-0-RxAdd-0-InitA-001830EA965F-AdvA-90D7EBB19299-AA-60850A1B-CRCInit-A77B22-WinSize-02-WinOffset-000F-Interval-0050-Latency-0000-Timeout-07D0-ChM-1FFFFFFFFF-Hop-9-SCA-5-Space-100 9-LL_REJECT_IND-AA-60850A1B-LLID-1-NESN-0-SN-0-MD-0-ErrorCode-00-CRCInit-A77B22-Space-100

    return( calculate_sample_for_LL_TERMINATE_IND(pkt_str, pkt) );
}


//---------------------------------------------------------------------------------------
// 该函数调用上面的分支 calculate_xxxxxxxx 函数 
//---------------------------------------------------------------------------------------
int calculate_sample_from_pkt_type(char *type_str, char *pkt_str, PKT_INFO *pkt) {
    if ( strcmp( toupper_str(type_str, tmp_str), "RAW" ) == 0 ) {
        pkt->pkt_type = RAW;
        printf("pkt_type RAW\n");
        if ( calculate_sample_for_RAW(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "DISCOVERY" ) == 0 ) {
        pkt->pkt_type = DISCOVERY;
        printf("pkt_type DISCOVERY\n");
        if ( calculate_sample_for_DISCOVERY(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    }
    else if ( strcmp( toupper_str(type_str, tmp_str), "IBEACON" ) == 0 ) {
        pkt->pkt_type = IBEACON;
        printf("pkt_type IBEACON\n");
        if ( calculate_sample_for_IBEACON(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "ADV_IND" ) == 0 ) {
        pkt->pkt_type = ADV_IND;
        printf("pkt_type ADV_IND\n");
        if ( calculate_sample_for_ADV_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "ADV_DIRECT_IND" ) == 0 ) {
        pkt->pkt_type = ADV_DIRECT_IND;
        printf("pkt_type ADV_DIRECT_IND\n");
        if ( calculate_sample_for_ADV_DIRECT_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "ADV_NONCONN_IND" ) == 0 ) {
        pkt->pkt_type = ADV_NONCONN_IND;
        printf("pkt_type ADV_NONCONN_IND\n");
        if ( calculate_sample_for_ADV_NONCONN_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "ADV_SCAN_IND" ) == 0 ) {
        pkt->pkt_type = ADV_SCAN_IND;
        printf("pkt_type ADV_SCAN_IND\n");
        if ( calculate_sample_for_ADV_SCAN_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "SCAN_REQ" ) == 0 ) {
        pkt->pkt_type = SCAN_REQ;
        printf("pkt_type SCAN_REQ\n");
        if ( calculate_sample_for_SCAN_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "SCAN_RSP" ) == 0 ) {
        pkt->pkt_type = SCAN_RSP;
        printf("pkt_type SCAN_RSP\n");
        if ( calculate_sample_for_SCAN_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "CONNECT_REQ" ) == 0 ) {
        pkt->pkt_type = CONNECT_REQ;
        printf("pkt_type CONNECT_REQ\n");
        if ( calculate_sample_for_CONNECT_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_DATA" ) == 0 ) {
        pkt->pkt_type = LL_DATA;
        printf("pkt_type LL_DATA\n");
        if ( calculate_sample_for_LL_DATA(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_CONNECTION_UPDATE_REQ" ) == 0 ) {
        pkt->pkt_type = LL_CONNECTION_UPDATE_REQ;
        printf("pkt_type LL_CONNECTION_UPDATE_REQ\n");
        if ( calculate_sample_for_LL_CONNECTION_UPDATE_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_CHANNEL_MAP_REQ" ) == 0 ) {
        pkt->pkt_type = LL_CHANNEL_MAP_REQ;
        printf("pkt_type LL_CHANNEL_MAP_REQ\n");
        if ( calculate_sample_for_LL_CHANNEL_MAP_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_TERMINATE_IND" ) == 0 ) {
        pkt->pkt_type = LL_TERMINATE_IND;
        printf("pkt_type LL_TERMINATE_IND\n");
        if ( calculate_sample_for_LL_TERMINATE_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_ENC_REQ" ) == 0 ) {
        pkt->pkt_type = LL_ENC_REQ;
        printf("pkt_type LL_ENC_REQ\n");
        if ( calculate_sample_for_LL_ENC_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_ENC_RSP" ) == 0 ) {
        pkt->pkt_type = LL_ENC_RSP;
        printf("pkt_type LL_ENC_RSP\n");
        if ( calculate_sample_for_LL_ENC_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_START_ENC_REQ" ) == 0 ) {
        pkt->pkt_type = LL_START_ENC_REQ;
        printf("pkt_type LL_START_ENC_REQ\n");
        if ( calculate_sample_for_LL_START_ENC_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_START_ENC_RSP" ) == 0 ) {
        pkt->pkt_type = LL_START_ENC_RSP;
        printf("pkt_type LL_START_ENC_RSP\n");
        if ( calculate_sample_for_LL_START_ENC_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_UNKNOWN_RSP" ) == 0 ) {
        pkt->pkt_type = LL_UNKNOWN_RSP;
        printf("pkt_type LL_UNKNOWN_RSP\n");
        if ( calculate_sample_for_LL_UNKNOWN_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_FEATURE_REQ" ) == 0 ) {
        pkt->pkt_type = LL_FEATURE_REQ;
        printf("pkt_type LL_FEATURE_REQ\n");
        if ( calculate_sample_for_LL_FEATURE_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_FEATURE_RSP" ) == 0 ) {
        pkt->pkt_type = LL_FEATURE_RSP;
        printf("pkt_type LL_FEATURE_RSP\n");
        if ( calculate_sample_for_LL_FEATURE_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_PAUSE_ENC_REQ" ) == 0 ) {
        pkt->pkt_type = LL_PAUSE_ENC_REQ;
        printf("pkt_type LL_PAUSE_ENC_REQ\n");
        if ( calculate_sample_for_LL_PAUSE_ENC_REQ(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_PAUSE_ENC_RSP" ) == 0 ) {
        pkt->pkt_type = LL_PAUSE_ENC_RSP;
        printf("pkt_type LL_PAUSE_ENC_RSP\n");
        if ( calculate_sample_for_LL_PAUSE_ENC_RSP(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_VERSION_IND" ) == 0 ) {
        pkt->pkt_type = LL_VERSION_IND;
        printf("pkt_type LL_VERSION_IND\n");
        if ( calculate_sample_for_LL_VERSION_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else if ( strcmp( toupper_str(type_str, tmp_str), "LL_REJECT_IND" ) == 0 ) {
        pkt->pkt_type = LL_REJECT_IND;
        printf("pkt_type LL_REJECT_IND\n");
        if ( calculate_sample_for_LL_REJECT_IND(pkt_str, pkt) == -1 ) {
            return(-1);
        }
    } else {
        pkt->pkt_type = INVALID_TYPE;
        printf("pkt_type INVALID_TYPE\n");
        return(-1);
    }

    return(0);
}

//---------------------------------------------------------------------------------------
// 对外 
//---------------------------------------------------------------------------------------
int calculate_pkt_info( PKT_INFO *pkt ){
    char *cmd_str = pkt->cmd_str;
    char *next_p;
    int ret;

    // get channel number
    int channel_number;
    next_p = get_next_field_value(cmd_str, &channel_number, &ret);
    if (ret != 0) {
        printf("Getting channel number failed! It should be 0~39.\n");
        return(-1);
    }

    if (channel_number < 0 || channel_number > 39){
        printf("Invalid channel number is found. It should be 0~39.\n");
        return(-1);
    }

    if (channel_number == 0) {
        if (tmp_str[0] != '0' ||  tmp_str[1] != 0  ) {
            printf("Invalid channel number is found. It should be 0~39.\n");
            return(-1);
        }
    }

    pkt->channel_number = channel_number;
    printf("channel_number %d\n", channel_number);

    // get pkt_type
    char *current_p = next_p;
    next_p = get_next_field(current_p, tmp_str, "-", MAX_NUM_CHAR_CMD);
    if ( next_p == NULL  || next_p==current_p ) {
        printf("Getting packet type failed!\n");
        return(-1);
    }

    if ( calculate_sample_from_pkt_type(tmp_str, next_p, pkt) == -1 ){
        if ( pkt->pkt_type == INVALID_TYPE ) {
            printf("Invalid packet type!\n");
        } else {
            printf("Invalid packet content for specific packet type!\n");
        }
        return(-1);
    }

    return(0);
}



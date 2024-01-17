/*************************************************************************
  > File Name: app_tools.c
  > Author: 
  > Mail: 
  > Created Time: Wed 10 Jan 2024 17:42:32 CST
 ************************************************************************/
#include "app_tools.h"

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 时间相关 
//---------------------------------------------------------------------------------------
inline int TimevalDiff(const struct timeval *a, const struct timeval *b)
{
    return( (a->tv_sec - b->tv_sec)*1000000 + (a->tv_usec - b->tv_usec) );
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 2d 相关 
//---------------------------------------------------------------------------------------
char ** malloc_2d(int num_row, int num_col) {
    int i, j;

    char **items = (char **)malloc(num_row * sizeof(char *));

    if (items == NULL) {
        return(NULL);
    }

    for (i=0; i<num_row; i++) {
        items[i] = (char *)malloc( num_col * sizeof(char));

        if (items[i] == NULL) {
            for (j=i-1; j>=0; j--) {
                free(items[i]);
            }
            return(NULL);
        }
    }

    return(items);
}

void release_2d(char **items, int num_row) {
    int i;
    for (i=0; i<num_row; i++){
        free((char *) items[i]);
    }
    free ((char *) items);
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 打印相关 
//---------------------------------------------------------------------------------------
void disp_bit_in_hex(char *bit, int num_bit)
{
    int i, a;
    for(i=0; i<num_bit; i=i+8) {
        a = bit[i] + bit[i+1]*2 + bit[i+2]*4 + bit[i+3]*8 + bit[i+4]*16 + bit[i+5]*32 + bit[i+6]*64 + bit[i+7]*128;
        //a = bit[i+7] + bit[i+6]*2 + bit[i+5]*4 + bit[i+4]*8 + bit[i+3]*16 + bit[i+2]*32 + bit[i+1]*64 + bit[i]*128;
        printf("%02x", a);
    }
    printf("\n");
}

void disp_bit(char *bit, int num_bit)
{
    int i, bit_val;
    for(i=0; i<num_bit; i++) {
        bit_val = bit[i];
        if (i%8 == 0 && i != 0) {
            printf(" ");
        } else if (i%4 == 0 && i != 0) {
            printf("-");
        }
        printf("%d", bit_val);
    }
    printf("\n");
}


void disp_hex(uint8_t *hex, int num_hex)
{
    int i;
    for(i=0; i<num_hex; i++)
    {
        printf("%02x", hex[i]);
    }
    printf("\n");
}

void disp_hex_in_bit(uint8_t *hex, int num_hex)
{
    int i, j, bit_val;

    for(j=0; j<num_hex; j++) {

        for(i=0; i<8; i++) {
            bit_val = (hex[j]>>i)&0x01;
            if (i==4) {
                printf("-");
            }
            printf("%d", bit_val);
        }

        printf(" ");

    }

    printf("\n");
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- 转换相关 
//---------------------------------------------------------------------------------------
char* toupper_str(char *input_str, char *output_str) {
    int len_str = strlen(input_str);
    int i;

    for (i=0; i<=len_str; i++) {
        output_str[i] = toupper( input_str[i] );
    }

    return(output_str);
}

void octet_hex_to_bit(char *hex, char *bit) {
    char tmp_hex[3];

    tmp_hex[0] = hex[0];
    tmp_hex[1] = hex[1];
    tmp_hex[2] = 0;

    int n = strtol(tmp_hex, NULL, 16);

    bit[0] = 0x01&(n>>0);
    bit[1] = 0x01&(n>>1);
    bit[2] = 0x01&(n>>2);
    bit[3] = 0x01&(n>>3);
    bit[4] = 0x01&(n>>4);
    bit[5] = 0x01&(n>>5);
    bit[6] = 0x01&(n>>6);
    bit[7] = 0x01&(n>>7);
}

int bit_to_int(char *bit) {
    int n = 0;
    int i;
    for(i=0; i<8; i++) {
        n = ( (n<<1) | bit[7-i] );
    }
    return(n);
}

void int_to_bit(int n, char *bit) {
    bit[0] = 0x01&(n>>0);
    bit[1] = 0x01&(n>>1);
    bit[2] = 0x01&(n>>2);
    bit[3] = 0x01&(n>>3);
    bit[4] = 0x01&(n>>4);
    bit[5] = 0x01&(n>>5);
    bit[6] = 0x01&(n>>6);
    bit[7] = 0x01&(n>>7);
}

void int_to_bit2(int n, uint8_t *bit) {
    bit[0] = 0x01&(n>>0);
    bit[1] = 0x01&(n>>1);
    bit[2] = 0x01&(n>>2);
    bit[3] = 0x01&(n>>3);
    bit[4] = 0x01&(n>>4);
    bit[5] = 0x01&(n>>5);
    bit[6] = 0x01&(n>>6);
    bit[7] = 0x01&(n>>7);
}

int convert_hex_to_bit(char *hex, char *bit){
    int num_hex_orig = strlen(hex);
    //while(hex[num_hex-1]<=32 || hex[num_hex-1]>=127) {
    //  num_hex--;
    //}
    int i, num_hex;
    num_hex = num_hex_orig;
    for(i=0; i<num_hex_orig; i++) {
        if ( !( (hex[i]>=48 && hex[i]<=57) || (hex[i]>=65 && hex[i]<=70) || (hex[i]>=97 && hex[i]<=102) ) ) //not a hex
            num_hex--;
    }

    if (num_hex%2 != 0) {
        printf("convert_hex_to_bit: Half octet is encountered! num_hex %d\n", num_hex);
        printf("%s\n", hex);
        return(-1);
    }

    int num_bit = num_hex*4;

    int j;
    for (i=0; i<num_hex; i=i+2) {
        j = i*4;
        octet_hex_to_bit(hex+i, bit+j);
    }

    return(num_bit);
}

void uint32_to_bit_array(uint32_t uint32_in, uint8_t *bit) {
    int i;
    uint32_t uint32_tmp = uint32_in;
    for(i=0; i<32; i++) {
        bit[i] = 0x01&uint32_tmp;
        uint32_tmp = (uint32_tmp>>1);
    }
}

void byte_array_to_bit_array(uint8_t *byte_in, int num_byte, uint8_t *bit) {
    int i, j;
    j=0;
    for(i=0; i<num_byte*8; i=i+8) {
        int_to_bit2(byte_in[j], bit+i);
        j++;
    }
}


//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- CRC24 相关 
//---------------------------------------------------------------------------------------
/**
 * Static table used for the table_driven implementation.
 *****************************************************************************/
static const uint_fast32_t crc_table[256] = {
    0x000000, 0x01b4c0, 0x036980, 0x02dd40, 0x06d300, 0x0767c0, 0x05ba80, 0x040e40,
    0x0da600, 0x0c12c0, 0x0ecf80, 0x0f7b40, 0x0b7500, 0x0ac1c0, 0x081c80, 0x09a840,
    0x1b4c00, 0x1af8c0, 0x182580, 0x199140, 0x1d9f00, 0x1c2bc0, 0x1ef680, 0x1f4240,
    0x16ea00, 0x175ec0, 0x158380, 0x143740, 0x103900, 0x118dc0, 0x135080, 0x12e440,
    0x369800, 0x372cc0, 0x35f180, 0x344540, 0x304b00, 0x31ffc0, 0x332280, 0x329640,
    0x3b3e00, 0x3a8ac0, 0x385780, 0x39e340, 0x3ded00, 0x3c59c0, 0x3e8480, 0x3f3040,
    0x2dd400, 0x2c60c0, 0x2ebd80, 0x2f0940, 0x2b0700, 0x2ab3c0, 0x286e80, 0x29da40,
    0x207200, 0x21c6c0, 0x231b80, 0x22af40, 0x26a100, 0x2715c0, 0x25c880, 0x247c40,
    0x6d3000, 0x6c84c0, 0x6e5980, 0x6fed40, 0x6be300, 0x6a57c0, 0x688a80, 0x693e40,
    0x609600, 0x6122c0, 0x63ff80, 0x624b40, 0x664500, 0x67f1c0, 0x652c80, 0x649840,
    0x767c00, 0x77c8c0, 0x751580, 0x74a140, 0x70af00, 0x711bc0, 0x73c680, 0x727240,
    0x7bda00, 0x7a6ec0, 0x78b380, 0x790740, 0x7d0900, 0x7cbdc0, 0x7e6080, 0x7fd440,
    0x5ba800, 0x5a1cc0, 0x58c180, 0x597540, 0x5d7b00, 0x5ccfc0, 0x5e1280, 0x5fa640,
    0x560e00, 0x57bac0, 0x556780, 0x54d340, 0x50dd00, 0x5169c0, 0x53b480, 0x520040,
    0x40e400, 0x4150c0, 0x438d80, 0x423940, 0x463700, 0x4783c0, 0x455e80, 0x44ea40,
    0x4d4200, 0x4cf6c0, 0x4e2b80, 0x4f9f40, 0x4b9100, 0x4a25c0, 0x48f880, 0x494c40,
    0xda6000, 0xdbd4c0, 0xd90980, 0xd8bd40, 0xdcb300, 0xdd07c0, 0xdfda80, 0xde6e40,
    0xd7c600, 0xd672c0, 0xd4af80, 0xd51b40, 0xd11500, 0xd0a1c0, 0xd27c80, 0xd3c840,
    0xc12c00, 0xc098c0, 0xc24580, 0xc3f140, 0xc7ff00, 0xc64bc0, 0xc49680, 0xc52240,
    0xcc8a00, 0xcd3ec0, 0xcfe380, 0xce5740, 0xca5900, 0xcbedc0, 0xc93080, 0xc88440,
    0xecf800, 0xed4cc0, 0xef9180, 0xee2540, 0xea2b00, 0xeb9fc0, 0xe94280, 0xe8f640,
    0xe15e00, 0xe0eac0, 0xe23780, 0xe38340, 0xe78d00, 0xe639c0, 0xe4e480, 0xe55040,
    0xf7b400, 0xf600c0, 0xf4dd80, 0xf56940, 0xf16700, 0xf0d3c0, 0xf20e80, 0xf3ba40,
    0xfa1200, 0xfba6c0, 0xf97b80, 0xf8cf40, 0xfcc100, 0xfd75c0, 0xffa880, 0xfe1c40,
    0xb75000, 0xb6e4c0, 0xb43980, 0xb58d40, 0xb18300, 0xb037c0, 0xb2ea80, 0xb35e40,
    0xbaf600, 0xbb42c0, 0xb99f80, 0xb82b40, 0xbc2500, 0xbd91c0, 0xbf4c80, 0xbef840,
    0xac1c00, 0xada8c0, 0xaf7580, 0xaec140, 0xaacf00, 0xab7bc0, 0xa9a680, 0xa81240,
    0xa1ba00, 0xa00ec0, 0xa2d380, 0xa36740, 0xa76900, 0xa6ddc0, 0xa40080, 0xa5b440,
    0x81c800, 0x807cc0, 0x82a180, 0x831540, 0x871b00, 0x86afc0, 0x847280, 0x85c640,
    0x8c6e00, 0x8ddac0, 0x8f0780, 0x8eb340, 0x8abd00, 0x8b09c0, 0x89d480, 0x886040,
    0x9a8400, 0x9b30c0, 0x99ed80, 0x985940, 0x9c5700, 0x9de3c0, 0x9f3e80, 0x9e8a40,
    0x972200, 0x9696c0, 0x944b80, 0x95ff40, 0x91f100, 0x9045c0, 0x929880, 0x932c40
};


/**
 * Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
uint_fast32_t crc_update(uint_fast32_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int tbl_idx;

    while (data_len--) {
        tbl_idx = (crc ^ *d) & 0xff;
        crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffff;

        d++;
    }
    return crc & 0xffffff;
}

uint_fast32_t crc24_byte(uint8_t *byte_in, int num_byte, int init_hex) {
    uint_fast32_t crc = init_hex;

    crc = crc_update(crc, byte_in, num_byte);

    return(crc);
}

void crc24(char *bit_in, int num_bit, char *init_hex, char *crc_result) {
    char bit_store[24], bit_store_update[24];
    int i;
    convert_hex_to_bit(init_hex, bit_store);

    for (i=0; i<num_bit; i++) {
        char new_bit = (bit_store[23]+bit_in[i])%2;
        bit_store_update[0] = new_bit;
        bit_store_update[1] = (bit_store[0]+new_bit)%2;
        bit_store_update[2] = bit_store[1];
        bit_store_update[3] = (bit_store[2]+new_bit)%2;
        bit_store_update[4] = (bit_store[3]+new_bit)%2;
        bit_store_update[5] = bit_store[4];
        bit_store_update[6] = (bit_store[5]+new_bit)%2;

        bit_store_update[7] = bit_store[6];
        bit_store_update[8] = bit_store[7];

        bit_store_update[9] = (bit_store[8]+new_bit)%2;
        bit_store_update[10] = (bit_store[9]+new_bit)%2;

        memcpy(bit_store_update+11, bit_store+10, 13);

        memcpy(bit_store, bit_store_update, 24);
    }

    for (i=0; i<24; i++) {
        crc_result[i] = bit_store[23-i];
    }
}

uint32_t crc_init_reorder(uint32_t crc_init) {
    int i;
    uint32_t crc_init_tmp, crc_init_input, crc_init_input_tmp;

    crc_init_input_tmp = crc_init;
    crc_init_input = 0;

    crc_init_input = ( (crc_init_input)|(crc_init_input_tmp&0xFF) );

    crc_init_input_tmp = (crc_init_input_tmp>>8);
    crc_init_input = ( (crc_init_input<<8)|(crc_init_input_tmp&0xFF) );

    crc_init_input_tmp = (crc_init_input_tmp>>8);
    crc_init_input = ( (crc_init_input<<8)|(crc_init_input_tmp&0xFF) );

    //printf("%06x\n", crc_init_input);

    crc_init_input = (crc_init_input<<1);
    crc_init_tmp = 0;
    for(i=0; i<24; i++) {
        crc_init_input = (crc_init_input>>1);
        crc_init_tmp = ( (crc_init_tmp<<1)|( crc_init_input&0x01 ) );
    }
    return(crc_init_tmp);
}

bool crc_check(uint8_t *tmp_byte, int body_len, uint32_t crc_init) {
    int crc24_checksum, crc24_received;//, i;
                                       //uint32_t crc_init_tmp, crc_init_input;
                                       // method 1
                                       //crc_init_tmp = ( (~crc_init)&0xFFFFFF );

                                       // method 2
#if 0
    crc_init_input = (crc_init<<1);
    crc_init_tmp = 0;
    for(i=0; i<24; i++) {
        crc_init_input = (crc_init_input>>1);
        crc_init_tmp = ( (crc_init_tmp<<1)|( crc_init_input&0x01 ) );
    }
#endif

    crc24_checksum = crc24_byte(tmp_byte, body_len, crc_init); // 0x555555 --> 0xaaaaaa. maybe because byte order
    crc24_received = 0;
    crc24_received = ( (crc24_received << 8) | tmp_byte[body_len+2] );
    crc24_received = ( (crc24_received << 8) | tmp_byte[body_len+1] );
    crc24_received = ( (crc24_received << 8) | tmp_byte[body_len+0] );
    return(crc24_checksum!=crc24_received);
}

//---------------------------------------------------------------------------------------
// 对外函数调用的函数-- phy_sample load and save 
//---------------------------------------------------------------------------------------
void save_phy_sample(char *IQ_sample, int num_IQ_sample, char *filename)
{
    int i;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("save_phy_sample: fopen failed!\n");
        return;
    }

    for(i=0; i<num_IQ_sample; i++) {
        if (i%24 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%d, ", IQ_sample[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);
}

void save_phy_sample_for_matlab(char *IQ_sample, int num_IQ_sample, char *filename)
{
    int i;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("save_phy_sample_for_matlab: fopen failed!\n");
        return;
    }

    for(i=0; i<num_IQ_sample; i++) {
        if (i%24 == 0) {
            fprintf(fp, "...\n");
        }
        fprintf(fp, "%d ", IQ_sample[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);
}

void save_phy_sample2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename)
{
    int i;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("save_phy_sample: fopen failed!\n");
        return;
    }

    for(i=0; i<num_IQ_sample; i++) {
        if (i%64 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%d, ", IQ_sample[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);
}

void load_phy_sample2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename)
{
    int i, tmp_val;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("load_phy_sample: fopen failed!\n");
        return;
    }

    i = 0;
    while( ~feof(fp) ) {
        if ( fscanf(fp, "%d,", &tmp_val) ) {
            IQ_sample[i] = tmp_val;
            i++;
        }
        if (num_IQ_sample != -1) {
            if (i==num_IQ_sample) {
                break;
            }
        }
        //printf("%d\n", i);
    }
    printf("%d I/Q are read.\n", i);

    fclose(fp);
}

void save_phy_sample_for_matlab2(IQ_TYPE *IQ_sample, int num_IQ_sample, char *filename)
{
    int i;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("save_phy_sample_for_matlab: fopen failed!\n");
        return;
    }

    for(i=0; i<num_IQ_sample; i++) {
        if (i%64 == 0) {
            fprintf(fp, "...\n");
        }
        fprintf(fp, "%d ", IQ_sample[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);
}

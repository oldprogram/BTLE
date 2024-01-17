/*************************************************************************
	> File Name: app_calculate_pkt.c
	> Author: 
	> Mail: 
	> Created Time: Fri 12 Jan 2024 16:17:58 CST
 ************************************************************************/
#include "app_ble_tools.h"
#include "app_calculate_pkt.h"
#include "scramble_table.h"

char *LL_PDU_TYPE_STR[] = {
    "LL_RESERVED",
    "LL_DATA1",
    "LL_DATA2",
    "LL_CTRL"
};

char *LL_CTRL_PDU_PAYLOAD_TYPE_STR[] = {
    "LL_CONNECTION_UPDATE_REQ",
    "LL_CHANNEL_MAP_REQ",
    "LL_TERMINATE_IND",
    "LL_ENC_REQ",
    "LL_ENC_RSP",
    "LL_START_ENC_REQ",
    "LL_START_ENC_RSP",
    "LL_UNKNOWN_RSP",
    "LL_FEATURE_REQ",
    "LL_FEATURE_RSP",
    "LL_PAUSE_ENC_REQ",
    "LL_PAUSE_ENC_RSP",
    "LL_VERSION_IND",
    "LL_REJECT_IND",
    "LL_RESERVED"
};

char *ADV_PDU_TYPE_STR[] = {
    "ADV_IND",
    "ADV_DIRECT_IND",
    "ADV_NONCONN_IND",
    "SCAN_REQ",
    "SCAN_RSP",
    "CONNECT_REQ",
    "ADV_SCAN_IND",
    "RESERVED0",
    "RESERVED1",
    "RESERVED2",
    "RESERVED3",
    "RESERVED4",
    "RESERVED5",
    "RESERVED6",
    "RESERVED7",
    "RESERVED8"
};


//static uint8_t demod_buf_preamble_access[SAMPLE_PER_SYMBOL][LEN_DEMOD_BUF_PREAMBLE_ACCESS];
static uint8_t demod_buf_access[SAMPLE_PER_SYMBOL][LEN_DEMOD_BUF_ACCESS];
//uint8_t preamble_access_byte[NUM_PREAMBLE_ACCESS_BYTE] = {0xAA, 0xD6, 0xBE, 0x89, 0x8E};
uint8_t access_byte[NUM_ACCESS_ADDR_BYTE] = {0xD6, 0xBE, 0x89, 0x8E};
//uint8_t preamble_access_bit[NUM_PREAMBLE_ACCESS_BYTE*8];
uint8_t access_bit[NUM_ACCESS_ADDR_BYTE*8];
uint8_t access_bit_mask[NUM_ACCESS_ADDR_BYTE*8];
uint8_t tmp_byte[2+37+3]; // header length + maximum payload length 37 + 3 octets CRC

RECV_STATUS receiver_status;

extern char* filename_pcap;
extern FILE *fh_pcap_store;

void demod_byte(IQ_TYPE* rxp, int num_byte, uint8_t *out_byte) {
    int i, j;
    int I0, Q0, I1, Q1;
    uint8_t bit_decision;
    int sample_idx = 0;

    for (i=0; i<num_byte; i++) {
        out_byte[i] = 0;
        for (j=0; j<8; j++) {
            I0 = rxp[sample_idx];
            Q0 = rxp[sample_idx+1];
            I1 = rxp[sample_idx+2];
            Q1 = rxp[sample_idx+3];
            bit_decision = (I0*Q1 - I1*Q0)>0? 1 : 0;
            out_byte[i] = out_byte[i] | (bit_decision<<j);

            sample_idx = sample_idx + SAMPLE_PER_SYMBOL*2;
        }
    }
}

inline int search_unique_bits(IQ_TYPE* rxp, int search_len, uint8_t *unique_bits, uint8_t *unique_bits_mask, const int num_bits) {
    int i, sp, j, i0, q0, i1, q1, k, p, phase_idx;
    bool unequal_flag;
    const int demod_buf_len = num_bits;
    int demod_buf_offset = 0;

    //demod_buf_preamble_access[SAMPLE_PER_SYMBOL][LEN_DEMOD_BUF_PREAMBLE_ACCESS]
    //memset(demod_buf_preamble_access, 0, SAMPLE_PER_SYMBOL*LEN_DEMOD_BUF_PREAMBLE_ACCESS);
    memset(demod_buf_access, 0, SAMPLE_PER_SYMBOL*LEN_DEMOD_BUF_ACCESS);
    for(i=0; i<search_len*SAMPLE_PER_SYMBOL*2; i=i+(SAMPLE_PER_SYMBOL*2)) {
        sp = ( (demod_buf_offset-demod_buf_len+1)&(demod_buf_len-1) );
        //sp = (demod_buf_offset-demod_buf_len+1);
        //if (sp>=demod_buf_len)
        //  sp = sp - demod_buf_len;

        for(j=0; j<(SAMPLE_PER_SYMBOL*2); j=j+2) {
            i0 = rxp[i+j];
            q0 = rxp[i+j+1];
            i1 = rxp[i+j+2];
            q1 = rxp[i+j+3];

            phase_idx = j/2;
            //demod_buf_preamble_access[phase_idx][demod_buf_offset] = (i0*q1 - i1*q0) > 0? 1: 0;
            demod_buf_access[phase_idx][demod_buf_offset] = (i0*q1 - i1*q0) > 0? 1: 0;

            k = sp;
            unequal_flag = false;
            for (p=0; p<demod_buf_len; p++) {
                //if (demod_buf_preamble_access[phase_idx][k] != unique_bits[p]) {
                if (demod_buf_access[phase_idx][k] != unique_bits[p] && unique_bits_mask[p]) {
                    unequal_flag = true;
                    break;
                }
                k = ( (k + 1)&(demod_buf_len-1) );
                //k = (k + 1);
                //if (k>=demod_buf_len)
                //  k = k - demod_buf_len;
            }

            if(unequal_flag==false) {
                return( i + j - (demod_buf_len-1)*SAMPLE_PER_SYMBOL*2 );
            }

        }

        demod_buf_offset  = ( (demod_buf_offset+1)&(demod_buf_len-1) );
        //demod_buf_offset  = (demod_buf_offset+1);
        //if (demod_buf_offset>=demod_buf_len)
        //  demod_buf_offset = demod_buf_offset - demod_buf_len;
    }

    return(-1);
}

int parse_adv_pdu_payload_byte(uint8_t *payload_byte, int num_payload_byte, ADV_PDU_TYPE pdu_type, void *adv_pdu_payload) {
    ADV_PDU_PAYLOAD_TYPE_0_2_4_6 *payload_type_0_2_4_6 = NULL;
    ADV_PDU_PAYLOAD_TYPE_1_3 *payload_type_1_3 = NULL;
    ADV_PDU_PAYLOAD_TYPE_5 *payload_type_5 = NULL;
    ADV_PDU_PAYLOAD_TYPE_R *payload_type_R = NULL;
    if (num_payload_byte<6) {
        //payload_parse_result_str = ['Payload Too Short (only ' num2str(length(payload_bits)) ' bits)'];
        printf("Error: Payload Too Short (only %d bytes)!\n", num_payload_byte);
        return(-1);
    }

    if (pdu_type == ADV_IND || pdu_type == ADV_NONCONN_IND || pdu_type == SCAN_RSP || pdu_type == ADV_SCAN_IND) {
        payload_type_0_2_4_6 = (ADV_PDU_PAYLOAD_TYPE_0_2_4_6 *)adv_pdu_payload;

        //AdvA = reorder_bytes_str( payload_bytes(1 : (2*6)) );
        payload_type_0_2_4_6->AdvA[0] = payload_byte[5];
        payload_type_0_2_4_6->AdvA[1] = payload_byte[4];
        payload_type_0_2_4_6->AdvA[2] = payload_byte[3];
        payload_type_0_2_4_6->AdvA[3] = payload_byte[2];
        payload_type_0_2_4_6->AdvA[4] = payload_byte[1];
        payload_type_0_2_4_6->AdvA[5] = payload_byte[0];

        //AdvData = payload_bytes((2*6+1):end);
        //for(i=0; i<(num_payload_byte-6); i++) {
        //  payload_type_0_2_4_6->Data[i] = payload_byte[6+i];
        //}
        memcpy(payload_type_0_2_4_6->Data, payload_byte+6, num_payload_byte-6);

        //payload_parse_result_str = ['AdvA:' AdvA ' AdvData:' AdvData];
    } else if (pdu_type == ADV_DIRECT_IND || pdu_type == SCAN_REQ) {
        if (num_payload_byte!=12) {
            printf("Error: Payload length %d bytes. Need to be 12 for PDU Type %s!\n", num_payload_byte, ADV_PDU_TYPE_STR[pdu_type]);
            return(-1);
        }
        payload_type_1_3 = (ADV_PDU_PAYLOAD_TYPE_1_3 *)adv_pdu_payload;

        //AdvA = reorder_bytes_str( payload_bytes(1 : (2*6)) );
        payload_type_1_3->A0[0] = payload_byte[5];
        payload_type_1_3->A0[1] = payload_byte[4];
        payload_type_1_3->A0[2] = payload_byte[3];
        payload_type_1_3->A0[3] = payload_byte[2];
        payload_type_1_3->A0[4] = payload_byte[1];
        payload_type_1_3->A0[5] = payload_byte[0];

        //InitA = reorder_bytes_str( payload_bytes((2*6+1):end) );
        payload_type_1_3->A1[0] = payload_byte[11];
        payload_type_1_3->A1[1] = payload_byte[10];
        payload_type_1_3->A1[2] = payload_byte[9];
        payload_type_1_3->A1[3] = payload_byte[8];
        payload_type_1_3->A1[4] = payload_byte[7];
        payload_type_1_3->A1[5] = payload_byte[6];

        //payload_parse_result_str = ['AdvA:' AdvA ' InitA:' InitA];
    } else if (pdu_type == CONNECT_REQ) {
        if (num_payload_byte!=34) {
            printf("Error: Payload length %d bytes. Need to be 34 for PDU Type %s!\n", num_payload_byte, ADV_PDU_TYPE_STR[pdu_type]);
            return(-1);
        }
        payload_type_5 = (ADV_PDU_PAYLOAD_TYPE_5 *)adv_pdu_payload;

        //InitA = reorder_bytes_str( payload_bytes(1 : (2*6)) );
        payload_type_5->InitA[0] = payload_byte[5];
        payload_type_5->InitA[1] = payload_byte[4];
        payload_type_5->InitA[2] = payload_byte[3];
        payload_type_5->InitA[3] = payload_byte[2];
        payload_type_5->InitA[4] = payload_byte[1];
        payload_type_5->InitA[5] = payload_byte[0];

        //AdvA = reorder_bytes_str( payload_bytes((2*6+1):(2*6+2*6)) );
        payload_type_5->AdvA[0] = payload_byte[11];
        payload_type_5->AdvA[1] = payload_byte[10];
        payload_type_5->AdvA[2] = payload_byte[9];
        payload_type_5->AdvA[3] = payload_byte[8];
        payload_type_5->AdvA[4] = payload_byte[7];
        payload_type_5->AdvA[5] = payload_byte[6];

        //AA = reorder_bytes_str( payload_bytes((2*6+2*6+1):(2*6+2*6+2*4)) );
        payload_type_5->AA[0] = payload_byte[15];
        payload_type_5->AA[1] = payload_byte[14];
        payload_type_5->AA[2] = payload_byte[13];
        payload_type_5->AA[3] = payload_byte[12];

        //CRCInit = payload_bytes((2*6+2*6+2*4+1):(2*6+2*6+2*4+2*3));
        payload_type_5->CRCInit = ( payload_byte[16] );
        payload_type_5->CRCInit = ( (payload_type_5->CRCInit << 8) | payload_byte[17] );
        payload_type_5->CRCInit = ( (payload_type_5->CRCInit << 8) | payload_byte[18] );

        //WinSize = payload_bytes((2*6+2*6+2*4+2*3+1):(2*6+2*6+2*4+2*3+2*1));
        payload_type_5->WinSize = payload_byte[19];

        //WinOffset = reorder_bytes_str( payload_bytes((2*6+2*6+2*4+2*3+2*1+1):(2*6+2*6+2*4+2*3+2*1+2*2)) );
        payload_type_5->WinOffset = ( payload_byte[21] );
        payload_type_5->WinOffset = ( (payload_type_5->WinOffset << 8) | payload_byte[20] );

        //Interval = reorder_bytes_str( payload_bytes((2*6+2*6+2*4+2*3+2*1+2*2+1):(2*6+2*6+2*4+2*3+2*1+2*2+2*2)) );
        payload_type_5->Interval = ( payload_byte[23] );
        payload_type_5->Interval = ( (payload_type_5->Interval << 8) | payload_byte[22] );

        //Latency = reorder_bytes_str( payload_bytes((2*6+2*6+2*4+2*3+2*1+2*2+2*2+1):(2*6+2*6+2*4+2*3+2*1+2*2+2*2+2*2)) );
        payload_type_5->Latency = ( payload_byte[25] );
        payload_type_5->Latency = ( (payload_type_5->Latency << 8) | payload_byte[24] );

        //Timeout = reorder_bytes_str( payload_bytes((2*6+2*6+2*4+2*3+2*1+2*2+2*2+2*2+1):(2*6+2*6+2*4+2*3+2*1+2*2+2*2+2*2+2*2)) );
        payload_type_5->Timeout = ( payload_byte[27] );
        payload_type_5->Timeout = ( (payload_type_5->Timeout << 8) | payload_byte[26] );

        //ChM = reorder_bytes_str( payload_bytes((2*6+2*6+2*4+2*3+2*1+2*2+2*2+2*2+2*2+1):(2*6+2*6+2*4+2*3+2*1+2*2+2*2+2*2+2*2+2*5)) );
        payload_type_5->ChM[0] = payload_byte[32];
        payload_type_5->ChM[1] = payload_byte[31];
        payload_type_5->ChM[2] = payload_byte[30];
        payload_type_5->ChM[3] = payload_byte[29];
        payload_type_5->ChM[4] = payload_byte[28];

        //tmp_bits = payload_bits((end-7) : end);
        //Hop = num2str( bi2de(tmp_bits(1:5), 'right-msb') );
        //SCA = num2str( bi2de(tmp_bits(6:end), 'right-msb') );
        payload_type_5->Hop = (payload_byte[33]&0x1F);
        payload_type_5->SCA = ((payload_byte[33]>>5)&0x07);

        receiver_status.hop = payload_type_5->Hop;
        receiver_status.new_chm_flag = 1;
        receiver_status.interval = payload_type_5->Interval;

        receiver_status.access_addr  = ( payload_byte[15]);
        receiver_status.access_addr  = ( (receiver_status.access_addr  << 8) | payload_byte[14] );
        receiver_status.access_addr  = ( (receiver_status.access_addr  << 8) | payload_byte[13] );
        receiver_status.access_addr  = ( (receiver_status.access_addr  << 8) | payload_byte[12] );

        receiver_status.crc_init = payload_type_5->CRCInit;

        receiver_status.chm[0] = payload_type_5->ChM[0];
        receiver_status.chm[1] = payload_type_5->ChM[1];
        receiver_status.chm[2] = payload_type_5->ChM[2];
        receiver_status.chm[3] = payload_type_5->ChM[3];
        receiver_status.chm[4] = payload_type_5->ChM[4];
    } else {
        payload_type_R = (ADV_PDU_PAYLOAD_TYPE_R *)adv_pdu_payload;

        //for(i=0; i<(num_payload_byte); i++) {
        //  payload_type_R->payload_byte[i] = payload_byte[i];
        //}
        memcpy(payload_type_R->payload_byte, payload_byte, num_payload_byte);

        //printf("Warning: Reserved PDU type %d\n", pdu_type);
        //return(-1);
    }

    return(0);
}

int parse_ll_pdu_payload_byte(uint8_t *payload_byte, int num_payload_byte, LL_PDU_TYPE pdu_type, void *ll_pdu_payload) {
    int ctrl_pdu_type;
    LL_DATA_PDU_PAYLOAD_TYPE *data_payload = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_0 *ctrl_payload_type_0 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_1 *ctrl_payload_type_1 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_2_7_13 *ctrl_payload_type_2_7_13 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_3 *ctrl_payload_type_3 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_4 *ctrl_payload_type_4 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_5_6_10_11 *ctrl_payload_type_5_6_10_11 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_8_9 *ctrl_payload_type_8_9 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_12 *ctrl_payload_type_12 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_R *ctrl_payload_type_R = NULL;

    if (num_payload_byte==0) {
        if (pdu_type == LL_RESERVED || pdu_type==LL_DATA1) {
            return(0);
        }
        else if (pdu_type == LL_DATA2 || pdu_type == LL_CTRL) {
            printf("Error: LL PDU TYPE%d(%s) should not have payload length 0!\n", pdu_type, LL_PDU_TYPE_STR[pdu_type]);
            return(-1);
        }
    }

    if (pdu_type == LL_RESERVED || pdu_type == LL_DATA1 || pdu_type == LL_DATA2) {
        data_payload = (LL_DATA_PDU_PAYLOAD_TYPE *)ll_pdu_payload;
        memcpy(data_payload->Data, payload_byte, num_payload_byte);
    } else if (pdu_type == LL_CTRL) {
        ctrl_pdu_type = payload_byte[0];
        if (ctrl_pdu_type == LL_CONNECTION_UPDATE_REQ) {
            if (num_payload_byte!=12) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 12!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }

            ctrl_payload_type_0 = (LL_CTRL_PDU_PAYLOAD_TYPE_0 *)ll_pdu_payload;
            ctrl_payload_type_0->Opcode = ctrl_pdu_type;

            ctrl_payload_type_0->WinSize = payload_byte[1];

            ctrl_payload_type_0->WinOffset = ( payload_byte[3] );
            ctrl_payload_type_0->WinOffset = ( (ctrl_payload_type_0->WinOffset << 8) | payload_byte[2] );

            ctrl_payload_type_0->Interval = ( payload_byte[5] );
            ctrl_payload_type_0->Interval = ( (ctrl_payload_type_0->Interval << 8) | payload_byte[4] );

            ctrl_payload_type_0->Latency = ( payload_byte[7] );
            ctrl_payload_type_0->Latency = ( (ctrl_payload_type_0->Latency << 8) | payload_byte[6] );

            ctrl_payload_type_0->Timeout = ( payload_byte[9] );
            ctrl_payload_type_0->Timeout = ( (ctrl_payload_type_0->Timeout << 8) | payload_byte[8] );

            ctrl_payload_type_0->Instant = ( payload_byte[11] );
            ctrl_payload_type_0->Instant = ( (ctrl_payload_type_0->Instant << 8) | payload_byte[10] );

            receiver_status.interval = ctrl_payload_type_0->Interval;

        } else if (ctrl_pdu_type == LL_CHANNEL_MAP_REQ) {
            if (num_payload_byte!=8) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 8!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_1 = (LL_CTRL_PDU_PAYLOAD_TYPE_1 *)ll_pdu_payload;
            ctrl_payload_type_1->Opcode = ctrl_pdu_type;

            ctrl_payload_type_1->ChM[0] = payload_byte[5];
            ctrl_payload_type_1->ChM[1] = payload_byte[4];
            ctrl_payload_type_1->ChM[2] = payload_byte[3];
            ctrl_payload_type_1->ChM[3] = payload_byte[2];
            ctrl_payload_type_1->ChM[4] = payload_byte[1];

            ctrl_payload_type_1->Instant = ( payload_byte[7] );
            ctrl_payload_type_1->Instant = ( (ctrl_payload_type_1->Instant << 8) | payload_byte[6] );

            receiver_status.new_chm_flag = 1;

            receiver_status.chm[0] = ctrl_payload_type_1->ChM[0];
            receiver_status.chm[1] = ctrl_payload_type_1->ChM[1];
            receiver_status.chm[2] = ctrl_payload_type_1->ChM[2];
            receiver_status.chm[3] = ctrl_payload_type_1->ChM[3];
            receiver_status.chm[4] = ctrl_payload_type_1->ChM[4];

        } else if (ctrl_pdu_type == LL_TERMINATE_IND || ctrl_pdu_type == LL_UNKNOWN_RSP || ctrl_pdu_type == LL_REJECT_IND) {
            if (num_payload_byte!=2) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 2!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_2_7_13 = (LL_CTRL_PDU_PAYLOAD_TYPE_2_7_13 *)ll_pdu_payload;
            ctrl_payload_type_2_7_13->Opcode = ctrl_pdu_type;

            ctrl_payload_type_2_7_13->ErrorCode = payload_byte[1];

        } else if (ctrl_pdu_type == LL_ENC_REQ) {
            if (num_payload_byte!=23) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 23!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_3 = (LL_CTRL_PDU_PAYLOAD_TYPE_3 *)ll_pdu_payload;
            ctrl_payload_type_3->Opcode = ctrl_pdu_type;

            ctrl_payload_type_3->Rand[0] = payload_byte[8];
            ctrl_payload_type_3->Rand[1] = payload_byte[7];
            ctrl_payload_type_3->Rand[2] = payload_byte[6];
            ctrl_payload_type_3->Rand[3] = payload_byte[5];
            ctrl_payload_type_3->Rand[4] = payload_byte[4];
            ctrl_payload_type_3->Rand[5] = payload_byte[3];
            ctrl_payload_type_3->Rand[6] = payload_byte[2];
            ctrl_payload_type_3->Rand[7] = payload_byte[1];

            ctrl_payload_type_3->EDIV[0] = payload_byte[10];
            ctrl_payload_type_3->EDIV[1] = payload_byte[9];

            ctrl_payload_type_3->SKDm[0] = payload_byte[18];
            ctrl_payload_type_3->SKDm[1] = payload_byte[17];
            ctrl_payload_type_3->SKDm[2] = payload_byte[16];
            ctrl_payload_type_3->SKDm[3] = payload_byte[15];
            ctrl_payload_type_3->SKDm[4] = payload_byte[14];
            ctrl_payload_type_3->SKDm[5] = payload_byte[13];
            ctrl_payload_type_3->SKDm[6] = payload_byte[12];
            ctrl_payload_type_3->SKDm[7] = payload_byte[11];

            ctrl_payload_type_3->IVm[0] = payload_byte[22];
            ctrl_payload_type_3->IVm[1] = payload_byte[21];
            ctrl_payload_type_3->IVm[2] = payload_byte[20];
            ctrl_payload_type_3->IVm[3] = payload_byte[19];

        } else if (ctrl_pdu_type == LL_ENC_RSP) {
            if (num_payload_byte!=13) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 13!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_4 = (LL_CTRL_PDU_PAYLOAD_TYPE_4 *)ll_pdu_payload;
            ctrl_payload_type_4->Opcode = ctrl_pdu_type;

            ctrl_payload_type_4->SKDs[0] = payload_byte[8];
            ctrl_payload_type_4->SKDs[1] = payload_byte[7];
            ctrl_payload_type_4->SKDs[2] = payload_byte[6];
            ctrl_payload_type_4->SKDs[3] = payload_byte[5];
            ctrl_payload_type_4->SKDs[4] = payload_byte[4];
            ctrl_payload_type_4->SKDs[5] = payload_byte[3];
            ctrl_payload_type_4->SKDs[6] = payload_byte[2];
            ctrl_payload_type_4->SKDs[7] = payload_byte[1];

            ctrl_payload_type_4->IVs[0] = payload_byte[12];
            ctrl_payload_type_4->IVs[1] = payload_byte[11];
            ctrl_payload_type_4->IVs[2] = payload_byte[10];
            ctrl_payload_type_4->IVs[3] = payload_byte[9];

        } else if (ctrl_pdu_type == LL_START_ENC_REQ || ctrl_pdu_type == LL_START_ENC_RSP || ctrl_pdu_type == LL_PAUSE_ENC_REQ || ctrl_pdu_type == LL_PAUSE_ENC_RSP) {
            if (num_payload_byte!=1) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 1!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_5_6_10_11 = (LL_CTRL_PDU_PAYLOAD_TYPE_5_6_10_11 *)ll_pdu_payload;
            ctrl_payload_type_5_6_10_11->Opcode = ctrl_pdu_type;

        } else if (ctrl_pdu_type == LL_FEATURE_REQ || ctrl_pdu_type == LL_FEATURE_RSP) {
            if (num_payload_byte!=9) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 9!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_8_9 = (LL_CTRL_PDU_PAYLOAD_TYPE_8_9 *)ll_pdu_payload;
            ctrl_payload_type_8_9->Opcode = ctrl_pdu_type;

            ctrl_payload_type_8_9->FeatureSet[0] = payload_byte[8];
            ctrl_payload_type_8_9->FeatureSet[1] = payload_byte[7];
            ctrl_payload_type_8_9->FeatureSet[2] = payload_byte[6];
            ctrl_payload_type_8_9->FeatureSet[3] = payload_byte[5];
            ctrl_payload_type_8_9->FeatureSet[4] = payload_byte[4];
            ctrl_payload_type_8_9->FeatureSet[5] = payload_byte[3];
            ctrl_payload_type_8_9->FeatureSet[6] = payload_byte[2];
            ctrl_payload_type_8_9->FeatureSet[7] = payload_byte[1];

        } else if (ctrl_pdu_type == LL_VERSION_IND) {
            if (num_payload_byte!=6) {
                printf("Error: LL CTRL PDU TYPE%d(%s) should have payload length 6!\n", ctrl_pdu_type, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type]);
                return(-1);
            }
            ctrl_payload_type_12 = (LL_CTRL_PDU_PAYLOAD_TYPE_12 *)ll_pdu_payload;
            ctrl_payload_type_12->Opcode = ctrl_pdu_type;

            ctrl_payload_type_12->VersNr = payload_byte[1];

            ctrl_payload_type_12->CompId = (  payload_byte[3] );
            ctrl_payload_type_12->CompId = ( (ctrl_payload_type_12->CompId << 8) | payload_byte[2] );

            ctrl_payload_type_12->SubVersNr = (  payload_byte[5] );
            ctrl_payload_type_12->SubVersNr = ( (ctrl_payload_type_12->SubVersNr << 8) | payload_byte[4] );

        } else {
            ctrl_payload_type_R = (LL_CTRL_PDU_PAYLOAD_TYPE_R *)ll_pdu_payload;
            ctrl_payload_type_R->Opcode = ctrl_pdu_type;
            memcpy(ctrl_payload_type_R->payload_byte, payload_byte+1, num_payload_byte-1);
        }
    }

    return(ctrl_pdu_type);
}

void parse_ll_pdu_header_byte(uint8_t *byte_in, LL_PDU_TYPE *llid, int *nesn, int *sn, int *md, int *payload_len) {
    (*llid) = (LL_PDU_TYPE)(byte_in[0]&0x03);
    (*nesn) = ( (byte_in[0]&0x04) != 0 );
    (*sn) = ( (byte_in[0]&0x08) != 0 );
    (*md) = ( (byte_in[0]&0x10) != 0 );
    (*payload_len) = (byte_in[1]&0x1F);
}

void parse_adv_pdu_header_byte(uint8_t *byte_in, ADV_PDU_TYPE *pdu_type, int *tx_add, int *rx_add, int *payload_len) {
    //% pdy_type_str = {'ADV_IND', 'ADV_DIRECT_IND', 'ADV_NONCONN_IND', 'SCAN_REQ', 'SCAN_RSP', 'CONNECT_REQ', 'ADV_SCAN_IND', 'Reserved', 'Reserved', 'Reserved', 'Reserved', 'Reserved', 'Reserved', 'Reserved', 'Reserved'};
    //pdu_type = bi2de(bits(1:4), 'right-msb');
    (*pdu_type) = (ADV_PDU_TYPE)(byte_in[0]&0x0F);
    //% disp(['   PDU Type: ' pdy_type_str{pdu_type+1}]);

    //tx_add = bits(7);
    //% disp(['     Tx Add: ' num2str(tx_add)]);
    (*tx_add) = ( (byte_in[0]&0x40) != 0 );

    //rx_add = bits(8);
    //% disp(['     Rx Add: ' num2str(rx_add)]);
    (*rx_add) = ( (byte_in[0]&0x80) != 0 );

    //payload_len = bi2de(bits(9:14), 'right-msb');
    (*payload_len) = (byte_in[1]&0x3F);
}

inline void receiver_init(void) {
    byte_array_to_bit_array(access_byte, 4, access_bit);
}


void print_ll_pdu_payload(void *ll_pdu_payload, LL_PDU_TYPE pdu_type, int ctrl_pdu_type, int num_payload_byte, bool crc_flag) {
    int i;
    LL_DATA_PDU_PAYLOAD_TYPE *data_payload = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_0 *ctrl_payload_type_0 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_1 *ctrl_payload_type_1 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_2_7_13 *ctrl_payload_type_2_7_13 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_3 *ctrl_payload_type_3 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_4 *ctrl_payload_type_4 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_5_6_10_11 *ctrl_payload_type_5_6_10_11 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_8_9 *ctrl_payload_type_8_9 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_12 *ctrl_payload_type_12 = NULL;
    LL_CTRL_PDU_PAYLOAD_TYPE_R *ctrl_payload_type_R = NULL;

    if (num_payload_byte==0) {
        printf("CRC%d\n", crc_flag);
        return;
    }

    if (pdu_type == LL_RESERVED || pdu_type == LL_DATA1 || pdu_type == LL_DATA2) {
        data_payload = (LL_DATA_PDU_PAYLOAD_TYPE *)ll_pdu_payload;
        //memcpy(data_payload->Data, payload_byte, num_payload_byte);
        printf("LL_Data:");
        for(i=0; i<(num_payload_byte); i++) {
            printf("%02x", data_payload->Data[i]);
        }
    } else if (pdu_type == LL_CTRL) {
        if (ctrl_pdu_type == LL_CONNECTION_UPDATE_REQ) {
            ctrl_payload_type_0 = (LL_CTRL_PDU_PAYLOAD_TYPE_0 *)ll_pdu_payload;
            printf("Op%02x(%s) WSize:%02x WOffset:%04x Itrvl:%04x Ltncy:%04x Timot:%04x Inst:%04x", 
                    ctrl_payload_type_0->Opcode,
                    LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type],
                    ctrl_payload_type_0->WinSize, ctrl_payload_type_0->WinOffset, ctrl_payload_type_0->Interval, ctrl_payload_type_0->Latency, ctrl_payload_type_0->Timeout, ctrl_payload_type_0->Instant);

        } else if (ctrl_pdu_type == LL_CHANNEL_MAP_REQ) {
            ctrl_payload_type_1 = (LL_CTRL_PDU_PAYLOAD_TYPE_1 *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_1->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );
            printf(" ChM:");
            for(i=0; i<5; i++) {
                printf("%02x", ctrl_payload_type_1->ChM[i]);
            }
            printf(" Inst:%04x", ctrl_payload_type_1->Instant );

        } else if (ctrl_pdu_type == LL_TERMINATE_IND || ctrl_pdu_type == LL_UNKNOWN_RSP || ctrl_pdu_type == LL_REJECT_IND) {
            ctrl_payload_type_2_7_13 = (LL_CTRL_PDU_PAYLOAD_TYPE_2_7_13 *)ll_pdu_payload;
            printf("Op%02x(%s) Err:%02x", ctrl_payload_type_2_7_13->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type], ctrl_payload_type_2_7_13->ErrorCode );

        } else if (ctrl_pdu_type == LL_ENC_REQ) {
            ctrl_payload_type_3 = (LL_CTRL_PDU_PAYLOAD_TYPE_3 *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_3->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );
            printf(" Rand:");
            for(i=0; i<8; i++) {
                printf("%02x", ctrl_payload_type_3->Rand[i]);
            }
            printf(" EDIV:");
            for(i=0; i<2; i++) {
                printf("%02x", ctrl_payload_type_3->EDIV[i]);
            }
            printf(" SKDm:");
            for(i=0; i<8; i++) {
                printf("%02x", ctrl_payload_type_3->SKDm[i]);
            }
            printf(" IVm:");
            for(i=0; i<4; i++) {
                printf("%02x", ctrl_payload_type_3->IVm[i]);
            }

        } else if (ctrl_pdu_type == LL_ENC_RSP) {
            ctrl_payload_type_4 = (LL_CTRL_PDU_PAYLOAD_TYPE_4 *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_4->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );
            printf(" SKDs:");
            for(i=0; i<8; i++) {
                printf("%02x", ctrl_payload_type_4->SKDs[i]);
            }
            printf(" IVs:");
            for(i=0; i<4; i++) {
                printf("%02x", ctrl_payload_type_4->IVs[i]);
            }

        } else if (ctrl_pdu_type == LL_START_ENC_REQ || ctrl_pdu_type == LL_START_ENC_RSP || ctrl_pdu_type == LL_PAUSE_ENC_REQ || ctrl_pdu_type == LL_PAUSE_ENC_RSP) {
            ctrl_payload_type_5_6_10_11 = (LL_CTRL_PDU_PAYLOAD_TYPE_5_6_10_11 *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_5_6_10_11->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );

        } else if (ctrl_pdu_type == LL_FEATURE_REQ || ctrl_pdu_type == LL_FEATURE_RSP) {
            ctrl_payload_type_8_9 = (LL_CTRL_PDU_PAYLOAD_TYPE_8_9 *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_8_9->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );
            printf(" FteurSet:");
            for(i=0; i<8; i++) {
                printf("%02x", ctrl_payload_type_8_9->FeatureSet[i]);
            }

        } else if (ctrl_pdu_type == LL_VERSION_IND) {
            ctrl_payload_type_12 = (LL_CTRL_PDU_PAYLOAD_TYPE_12 *)ll_pdu_payload;
            printf("Op%02x(%s) Ver:%02x CompId:%04x SubVer:%04x", 
                    ctrl_payload_type_12->Opcode,
                    LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type],
                    ctrl_payload_type_12->VersNr, ctrl_payload_type_12->CompId, ctrl_payload_type_12->SubVersNr);

        } else {
            if (ctrl_pdu_type>LL_REJECT_IND)
                ctrl_pdu_type = LL_REJECT_IND+1;
            ctrl_payload_type_R = (LL_CTRL_PDU_PAYLOAD_TYPE_R *)ll_pdu_payload;
            printf("Op%02x(%s)", ctrl_payload_type_R->Opcode, LL_CTRL_PDU_PAYLOAD_TYPE_STR[ctrl_pdu_type] );
            printf(" Byte:");
            for(i=0; i<(num_payload_byte-1); i++) {
                printf("%02x", ctrl_payload_type_R->payload_byte[i]);
            }
        }
    }

    printf(" CRC%d\n", crc_flag);
}

void print_adv_pdu_payload(void *adv_pdu_payload, ADV_PDU_TYPE pdu_type, int payload_len, bool crc_flag) {
    int i;
    ADV_PDU_PAYLOAD_TYPE_5 *adv_pdu_payload_5;
    ADV_PDU_PAYLOAD_TYPE_1_3 *adv_pdu_payload_1_3;
    ADV_PDU_PAYLOAD_TYPE_0_2_4_6 *adv_pdu_payload_0_2_4_6;
    ADV_PDU_PAYLOAD_TYPE_R *adv_pdu_payload_R;
    // print payload out
    if (pdu_type==ADV_IND || pdu_type==ADV_NONCONN_IND || pdu_type==SCAN_RSP || pdu_type==ADV_SCAN_IND) {
        adv_pdu_payload_0_2_4_6 = (ADV_PDU_PAYLOAD_TYPE_0_2_4_6 *)(adv_pdu_payload);
        printf("AdvA:");
        for(i=0; i<6; i++) {
            printf("%02x", adv_pdu_payload_0_2_4_6->AdvA[i]);
        }
        printf(" Data:");
        for(i=0; i<(payload_len-6); i++) {
            printf("%02x", adv_pdu_payload_0_2_4_6->Data[i]);
        }
    } else if (pdu_type==ADV_DIRECT_IND || pdu_type==SCAN_REQ) {
        adv_pdu_payload_1_3 = (ADV_PDU_PAYLOAD_TYPE_1_3 *)(adv_pdu_payload);
        printf("A0:");
        for(i=0; i<6; i++) {
            printf("%02x", adv_pdu_payload_1_3->A0[i]);
        }
        printf(" A1:");
        for(i=0; i<6; i++) {
            printf("%02x", adv_pdu_payload_1_3->A1[i]);
        }
    } else if (pdu_type==CONNECT_REQ) {
        adv_pdu_payload_5 = (ADV_PDU_PAYLOAD_TYPE_5 *)(adv_pdu_payload);
        printf("InitA:");
        for(i=0; i<6; i++) {
            printf("%02x", adv_pdu_payload_5->InitA[i]);
        }
        printf(" AdvA:");
        for(i=0; i<6; i++) {
            printf("%02x", adv_pdu_payload_5->AdvA[i]);
        }
        printf(" AA:");
        for(i=0; i<4; i++) {
            printf("%02x", adv_pdu_payload_5->AA[i]);
        }
        printf(" CRCInit:%06x WSize:%02x WOffset:%04x Itrvl:%04x Ltncy:%04x Timot:%04x", adv_pdu_payload_5->CRCInit, adv_pdu_payload_5->WinSize, adv_pdu_payload_5->WinOffset, adv_pdu_payload_5->Interval, adv_pdu_payload_5->Latency, adv_pdu_payload_5->Timeout);
        printf(" ChM:");
        for(i=0; i<5; i++) {
            printf("%02x", adv_pdu_payload_5->ChM[i]);
        }
        printf(" Hop:%d SCA:%d", adv_pdu_payload_5->Hop, adv_pdu_payload_5->SCA);
    } else {
        adv_pdu_payload_R = (ADV_PDU_PAYLOAD_TYPE_R *)(adv_pdu_payload);
        printf("Byte:");
        for(i=0; i<(payload_len); i++) {
            printf("%02x", adv_pdu_payload_R->payload_byte[i]);
        }
    }
    printf(" CRC%d\n", crc_flag);
}

// demodulates and parses a packet
void receiver(IQ_TYPE *rxp_in, int buf_len, int channel_number, uint32_t access_addr, uint32_t crc_init, int verbose_flag, int raw_flag) {
    static int pkt_count = 0;
    static ADV_PDU_PAYLOAD_TYPE_R adv_pdu_payload;
    static LL_DATA_PDU_PAYLOAD_TYPE ll_data_pdu_payload;
    static struct timeval time_current_pkt, time_pre_pkt;
    const int demod_buf_len = LEN_BUF_MAX_NUM_PHY_SAMPLE+(LEN_BUF/2);

    ADV_PDU_TYPE adv_pdu_type;
    LL_PDU_TYPE ll_pdu_type;

    IQ_TYPE *rxp = rxp_in;
    int num_demod_byte, hit_idx, buf_len_eaten, adv_tx_add, adv_rx_add, ll_nesn, ll_sn, ll_md, payload_len, time_diff, ll_ctrl_pdu_type, i;
    int num_symbol_left = buf_len/(SAMPLE_PER_SYMBOL*2); //2 for IQ
    bool crc_flag;
    bool adv_flag = (channel_number==37 || channel_number==38 || channel_number==39);

    if (pkt_count == 0) { // the 1st time run
        gettimeofday(&time_current_pkt, NULL);
        time_pre_pkt = time_current_pkt;
    }

    uint32_to_bit_array(access_addr, access_bit);
    buf_len_eaten = 0;
    while( 1 ) 
    {
        hit_idx = search_unique_bits(rxp, num_symbol_left, access_bit, access_bit_mask, LEN_DEMOD_BUF_ACCESS);
        if ( hit_idx == -1 ) {
            break;
        }
        //pkt_count++;
        //printf("hit %d\n", hit_idx);

        //printf("%d %d %d %d %d %d %d %d\n", rxp[hit_idx+0], rxp[hit_idx+1], rxp[hit_idx+2], rxp[hit_idx+3], rxp[hit_idx+4], rxp[hit_idx+5], rxp[hit_idx+6], rxp[hit_idx+7]);

        buf_len_eaten = buf_len_eaten + hit_idx;
        //printf("%d\n", buf_len_eaten);

        buf_len_eaten = buf_len_eaten + 8*NUM_ACCESS_ADDR_BYTE*2*SAMPLE_PER_SYMBOL;// move to beginning of PDU header
        rxp = rxp_in + buf_len_eaten;

        if (raw_flag)
            num_demod_byte = 42;
        else
            num_demod_byte = 2; // PDU header has 2 octets

        buf_len_eaten = buf_len_eaten + 8*num_demod_byte*2*SAMPLE_PER_SYMBOL;
        //if ( buf_len_eaten > buf_len ) {
        if ( buf_len_eaten > demod_buf_len ) {
            break;
        }

        demod_byte(rxp, num_demod_byte, tmp_byte);

        if(!raw_flag) scramble_byte(tmp_byte, num_demod_byte, scramble_table[channel_number], tmp_byte);
        rxp = rxp_in + buf_len_eaten;
        num_symbol_left = (buf_len-buf_len_eaten)/(SAMPLE_PER_SYMBOL*2);

        if (raw_flag) { //raw recv stop here
            pkt_count++;

            gettimeofday(&time_current_pkt, NULL);
            time_diff = TimevalDiff(&time_current_pkt, &time_pre_pkt);
            time_pre_pkt = time_current_pkt;

            printf("%ld.%06ld Pkt%d Ch%d AA:%08x ", time_current_pkt.tv_sec, time_current_pkt.tv_usec, pkt_count, channel_number, access_addr);
            printf("Raw:");
            for(i=0; i<42; i++) {
                printf("%02x", tmp_byte[i]);
            }
            printf("\n");

            continue;
        }

        if (adv_flag)
        {
            parse_adv_pdu_header_byte(tmp_byte, &adv_pdu_type, &adv_tx_add, &adv_rx_add, &payload_len);
            if( payload_len<6 || payload_len>37 ) {
                if (verbose_flag) {
                    printf("XXXus PktBAD Ch%d AA:%08x ", channel_number, access_addr);
                    printf("ADV_PDU_t%d:%s T%d R%d PloadL%d ", adv_pdu_type, ADV_PDU_TYPE_STR[adv_pdu_type], adv_tx_add, adv_rx_add, payload_len);
                    printf("Error: ADV payload length should be 6~37!\n");
                }
                continue;
            }
        } else {
            parse_ll_pdu_header_byte(tmp_byte, &ll_pdu_type, &ll_nesn, &ll_sn, &ll_md, &payload_len);
        }


        //num_pdu_payload_crc_bits = (payload_len+3)*8;
        num_demod_byte = (payload_len+3);
        buf_len_eaten = buf_len_eaten + 8*num_demod_byte*2*SAMPLE_PER_SYMBOL;
        //if ( buf_len_eaten > buf_len ) {
        if ( buf_len_eaten > demod_buf_len ) {
            //printf("\n");
            break;
        }

        demod_byte(rxp, num_demod_byte, tmp_byte+2);
        scramble_byte(tmp_byte+2, num_demod_byte, scramble_table[channel_number]+2, tmp_byte+2);
        rxp = rxp_in + buf_len_eaten;
        num_symbol_left = (buf_len-buf_len_eaten)/(SAMPLE_PER_SYMBOL*2);

        crc_flag = crc_check(tmp_byte, payload_len+2, crc_init);
        pkt_count++;
        receiver_status.pkt_avaliable = 1;
        receiver_status.crc_ok = (crc_flag==0);

        gettimeofday(&time_current_pkt, NULL);
        time_diff = TimevalDiff(&time_current_pkt, &time_pre_pkt);
        time_pre_pkt = time_current_pkt;

        printf("%07dus Pkt%03d Ch%d AA:%08x ", time_diff, pkt_count, channel_number, access_addr);
        if(filename_pcap != NULL)
            write_packet_to_file(fh_pcap_store, payload_len+2, tmp_byte, channel_number, access_addr);

        if (adv_flag) {
            printf("ADV_PDU_t%d:%s T%d R%d PloadL%d ", adv_pdu_type, ADV_PDU_TYPE_STR[adv_pdu_type], adv_tx_add, adv_rx_add, payload_len);

            if (parse_adv_pdu_payload_byte(tmp_byte+2, payload_len, adv_pdu_type, (void *)(&adv_pdu_payload) ) != 0 ) {
                continue;
            }
            print_adv_pdu_payload((void *)(&adv_pdu_payload), adv_pdu_type, payload_len, crc_flag);
        } else {
            printf("LL_PDU_t%d:%s NESN%d SN%d MD%d PloadL%d ", ll_pdu_type, LL_PDU_TYPE_STR[ll_pdu_type], ll_nesn, ll_sn, ll_md, payload_len);

            if ( ( ll_ctrl_pdu_type=parse_ll_pdu_payload_byte(tmp_byte+2, payload_len, ll_pdu_type, (void *)(&ll_data_pdu_payload) )  ) < 0 ) {
                continue;
            }
            print_ll_pdu_payload((void *)(&ll_data_pdu_payload), ll_pdu_type, ll_ctrl_pdu_type, payload_len, crc_flag);
        }
    }
}


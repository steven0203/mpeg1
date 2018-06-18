#ifndef CODE_H
#define CODE_H



static const unsigned int sequence_header_code=0x000001B3;
static const unsigned int sequence_end_code=0x000001B7;
static const unsigned int extension_start_code=0x000001B5;
static const unsigned int user_data_start_code=0x000001B2;
static const unsigned int group_start_code=0x000001B8;
static const unsigned int picture_start_code=0x00000100;
static const unsigned int slice_start_code_low=0x00000101;
static const unsigned int slice_start_code_up=0x000001AF;
static const unsigned char end_of_block_run=100;
static const unsigned char escape_run=101;

#define macroblock_stuffing 34
#define macroblock_escape 35
#define I_picture_type 1
#define P_picture_type 2
#define B_picture_type 3
#define D_picture_type 4
#define dct_coeff_first 1
#define dct_coeff_next 0b11
#define end_of_block 0b10

typedef struct{
    unsigned char run;
    int level;
}dct_coeff;

#endif
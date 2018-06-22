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


typedef struct
{
    unsigned int horizotal_size;
    unsigned int vertical_size;
    unsigned char pel_aspect_ratio;
    unsigned char picture_rate;
    unsigned int bit_rate ;
    bool marker_bit;
    unsigned int vbv_buffer_size;
    bool constrained_parameters_flag;
    bool load_intra_quantizer_matrix;
    bool load_non_intra_quantizer_matrix;

}sequenceHeader;


typedef struct
{
    unsigned int time_code;
    bool closed_gop;
    bool broken_link;
}gopHeader;

typedef struct
{
    unsigned short temporal_reference;
    unsigned char picture_coding_type;
    unsigned short vbv_delay;
    bool full_pel_forward_vector;
    unsigned char forward_f;
    unsigned char forward_r_size;
    bool full_pel_backward_vector;
    unsigned char backward_f;
    unsigned char backward_r_size;

}pictureHeader;

typedef struct
{
    unsigned char slice_vertical_position ;
    unsigned char quantizer_scale;
}sliceHeader;

typedef struct
{
    unsigned int address_increment;
    bool quant;
    bool motion_forward;
    bool motion_backward;
    bool pattern;
    bool intra;
}macroblockHeader;



typedef struct
{
    bool quant;
    bool motion_forward;
    bool motion_backward;
    bool pattern;
    bool intra;
}macroblock_type;

typedef struct
{
    int y;
    int cr;
    int cb;
}yCbCr;



typedef struct
{
    int height;
    int width;
    int mb_width;
    int mb_height;
    yCbCr *data;
}picture;



#endif
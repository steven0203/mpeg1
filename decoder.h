#ifndef DEOCDER_H
#define DECODER_H
#include"dataStream.h"
#include"code.h"
#include"vlc.h"

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

void getExtensionData(dataStream &);
void getUserData(dataStream &);


/*Get sequence header and quantization matrix. */
int get_sequence_header(dataStream &,sequenceHeader *,unsigned char [64],unsigned char  [64]);
int group_of_pictures(dataStream &,gopHeader *,int &,int);
int get_picture(dataStream &,pictureHeader *);
int get_slice(dataStream &,sliceHeader *,pictureHeader *);
int get_macroblock(dataStream &,pictureHeader*,sliceHeader*,int &);
int get_block(dataStream &,bool [6],int ,macroblock_type,pictureHeader *);
int video_sequence(dataStream &,int);
#endif
#include"decoder.h"
#include"code.h"
#include"vlc.h"

static vlc vlcDecoder;
static const unsigned char default_intra_quantization_matrix[64]={
    8,16,19,22,26,27,29,34,
    16,16,22,24,27,29,34,37,
    19,22,26,27,29,34,34,38,
    22,22,26,27,29,34,37,40,
    22,26,27,29,32,35,40,48,
    26,27,29,32,35,40,48,58,
    26,27,29,34,38,46,56,69,
    27,29,35,38,46,56,69,83
};

static const unsigned char default_non_intra_quantization_matrix[64]={
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16
};

int get_sequence_header(dataStream &stream,sequenceHeader *header,unsigned char intraQuantizer[64],unsigned char nonIntraQuantizer[64])
{
    header->horizotal_size=stream.getBits(12)>>(32-12);
    header->vertical_size=stream.getBits(12)>>(32-12);
    header->pel_aspect_ratio=stream.getBits(4)>>(32-4);
    header->picture_rate=stream.getBits(4)>>(32-4);
    header->bit_rate=stream.getBits(18)>>(32-18);
    header->marker_bit=stream.getFirstBit();
    header->vbv_buffer_size=stream.getBits(10)>>(32-10);
    header->constrained_parameters_flag=stream.getFirstBit();
    header->load_intra_quantizer_matrix=stream.getFirstBit();
    header->load_non_intra_quantizer_matrix=stream.getFirstBit();
    if(header->load_intra_quantizer_matrix)
        for(int i=0;i<64;++i)
            intraQuantizer[i]=stream.getBits(8)>>(32-8);
    else 
        for(int i=0;i<64;++i)
            intraQuantizer[i]=default_intra_quantization_matrix[i];
    if(header->load_non_intra_quantizer_matrix)
        for(int i=0;i<64;++i)
            nonIntraQuantizer[i]=stream.getBits(8)>>(32-8);
    else 
        for(int i=0;i<64;++i)
            nonIntraQuantizer[i]=default_non_intra_quantization_matrix[i];
    stream.getNextStartCode();
    if(stream.next_start_code==extension_start_code)
        getExtensionData(stream);
    if(stream.next_start_code==user_data_start_code)
        getUserData(stream);
    return 0;
}



void getExtensionData(dataStream &stream)
{
    unsigned int bits=stream.getBits(24);
    while(bits!=256)
    {
        bits=(bits<<8)^(stream.getBits(8)>>16);
    }
    stream.getNextStartCode();
}

void getUserData(dataStream &stream)
{
    unsigned int bits=stream.getBits(24);
    while(bits!=256)
    {
        bits=(bits<<8)^(stream.getBits(8)>>16);
    }
    stream.getNextStartCode();
}

int group_of_pictures(dataStream &stream,gopHeader *gop,int &pictureNumber,int limit)
{
    gop->time_code=stream.getBits(25)>>(32-25);
    gop->closed_gop=stream.getFirstBit();
    gop->broken_link=stream.getFirstBit();
    stream.getNextStartCode();
    if(stream.next_start_code==extension_start_code)
        getExtensionData(stream);
    if(stream.next_start_code==user_data_start_code)
        getUserData(stream);
    pictureHeader pHeader;
    while(stream.next_start_code==picture_start_code)
    {
        printf("picture %d\n",++pictureNumber);
        get_picture(stream,&pHeader);
        if(pictureNumber==limit)
            break;
    }
    return 0;
}

int get_picture(dataStream &stream,pictureHeader *header)
{
    unsigned char tmp;
    header->temporal_reference=stream.getBits(10)>>(32-10);
    header->picture_coding_type=stream.getBits(3)>>(32-3);
    header->vbv_delay=stream.getBits(16)>>(16);
    if(header->picture_coding_type==2||header->picture_coding_type==3)
    {
        header->full_pel_forward_vector=stream.getFirstBit();
        tmp=stream.getBits(3)>>(32-3);
        header->forward_r_size=tmp-1;
        header->forward_f=1<<(tmp-1);
    }
    if(header->picture_coding_type==3)
    {
        header->full_pel_backward_vector=stream.getFirstBit();
        tmp=stream.getBits(3)>>(32-3);
        header->backward_r_size=tmp-1;
        header->backward_f=1<<(tmp-1);
    }
    while(stream.getFirstBit())
    {
        stream.getBits(8);
    }
    stream.getNextStartCode();
    if(stream.next_start_code==extension_start_code)
        getExtensionData(stream);
    if(stream.next_start_code==user_data_start_code)
        getUserData(stream);
    printf("temporal_reference %d\n",header->temporal_reference);
    printf("picture_coding_type %d\n",header->picture_coding_type);
    printf("vbv_delay %d\n",header->vbv_delay);
    sliceHeader sHeader;
    int i=0;
    while(stream.next_start_code>=slice_start_code_low&&stream.next_start_code<=slice_start_code_up)
    {
        get_slice(stream,&sHeader,header);
    }
    return 0;
}
int get_slice(dataStream & stream,sliceHeader *header,pictureHeader *pHeader)
{
    printf("slice code  %x \n",stream.next_start_code);
    header->slice_vertical_position=(unsigned char)(stream.next_start_code&0x000000FF);
    header->quantizer_scale=stream.getBits(5)>>(32-5);

    while(stream.getFirstBit())
    {
        stream.getBits(8);
    }
    printf("quantizer_scale %d\n",header->quantizer_scale);
    int index=0;
    while(1)
    {
        get_macroblock(stream,pHeader,header,index);
        if(stream.getBits(23)==0)
        {
            stream.seekBack(23);
            break;
        }
        stream.seekBack(23);

    }
    stream.getNextStartCode();
    return 0;
}


int get_macroblock(dataStream &stream,pictureHeader *pHeader,sliceHeader *sHeader,int &index)
{   

    unsigned short address=vlcDecoder.getMacroAddress(stream);
    unsigned short escape=0;
    while(address==macroblock_stuffing)
        address=vlcDecoder.getMacroAddress(stream);
    while(address==macroblock_escape)
    {   
        escape+=1;
        address=vlcDecoder.getMacroAddress(stream);
    }
    address=escape*33+address;
    index+=address;
    printf("macroblock  %d \n",index);
    printf("address increment  %d\n",address);
    macroblock_type type;
    type=vlcDecoder.getMacroType(stream,pHeader->picture_coding_type);
    printf("type.quant %d\n",type.quant);
    printf("type.motion_forward %d\n",type.motion_forward);
    printf("type.motion_backward %d\n",type.motion_backward);
    printf("type.pattern %d\n",type.pattern);
    printf("type.intra %d\n",type.intra);
    if(type.quant)
    {
        sHeader->quantizer_scale=stream.getBits(5)>>(32-5);
        printf("quantizer_scale %d\n");
    }

    int motion_horizontal_forward_code=0,motion_vertical_forward_code=0;
    unsigned char motion_horizontal_forward_r=0,motion_vertical_forward_r=0;
    if(type.motion_forward)
    {
        motion_horizontal_forward_code=vlcDecoder.getMotionCode(stream);
        if(pHeader->forward_f!=1&&motion_horizontal_forward_code!=0)
            motion_horizontal_forward_r=stream.getBits(pHeader->forward_r_size)>>(32-pHeader->forward_r_size);
        motion_vertical_forward_code=vlcDecoder.getMotionCode(stream);
        if(pHeader->forward_f!=1&&motion_vertical_forward_code!=0)
            motion_vertical_forward_r=stream.getBits(pHeader->forward_r_size)>>(32-pHeader->forward_r_size);
        printf("motion_horizontal_forward_code %d\n",motion_horizontal_forward_code);
        printf("motion_vertical_forward_code %d\n",motion_vertical_forward_code);

    }
    int motion_horizontal_backward_code=0,motion_vertical_backward_code=0;
    unsigned char motion_horizontal_backward_r=0,motion_vertical_backward_r=0;
    if(type.motion_backward)
    {
        motion_horizontal_backward_code=vlcDecoder.getMotionCode(stream);
        if ((pHeader->backward_f != 1)&&(motion_horizontal_backward_code != 0))
            motion_horizontal_backward_r=stream.getBits(pHeader->backward_r_size)>>(32-pHeader->backward_r_size);
        motion_vertical_backward_code=vlcDecoder.getMotionCode(stream);
        if ((pHeader->backward_f != 1)&&(motion_vertical_backward_code != 0))
            motion_vertical_backward_r=stream.getBits(pHeader->backward_r_size)>>(32-pHeader->backward_r_size);
    }
    unsigned char cbp=0;
    if(type.pattern)
        cbp=vlcDecoder.getCBP(stream);
    bool pattern_code[6];
    printf("pattern code \n");
    for(int i=0;i<6;++i)
    {
        pattern_code[i] = 0;
        if ( cbp & (1<<(5-i)) ) 
            pattern_code[i] = 1;
        if (type.intra)pattern_code[i] = 1 ;
        printf("%d ", pattern_code[i]);
    }
    printf("\n");
    for(int i=0;i<6;++i)
        get_block(stream,pattern_code,i,type,pHeader);
    if(pHeader->picture_coding_type==D_picture_type)
        stream.getFirstBit();
}


int get_block(dataStream & stream,bool pattern_code[6],int i,macroblock_type type,pictureHeader *pHeader)
{
    printf("block %d \n",i);
    unsigned char dct_dc_size;
    unsigned char dct_dc_differential;
    dct_coeff dctCoeff;
    if(pattern_code[i])
    {
        if(type.intra)
        {
            if (i<4)
            {
                dct_dc_size=vlcDecoder.getDctDcSizeLum(stream);
                printf("dct_dc_size lum  %d\n",dct_dc_size);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                    printf("dct_dc diff %d\n",dct_dc_differential);
                }
            }
            else
            {
                dct_dc_size=vlcDecoder.getDctDcSizeChr(stream);
                 printf("dct_dc_size chr  %d\n",dct_dc_size);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                    printf("dct_dc diff %d\n",dct_dc_differential);
                }
            }
        }
        else
        {
            dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_first);
            printf("dct_coeff_first run : %d level : %d \n ",dctCoeff.run,dctCoeff.level);
        }
        if(pHeader->picture_coding_type!=D_picture_type)
        {
            dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_next);
            while(dctCoeff.run!=end_of_block_run)
            {
                printf("dct_coeff_next : %d level : %d \n",dctCoeff.run,dctCoeff.level);
                dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_next);
            }
            printf("EOB\n");
        }
    }
}


int video_sequence(dataStream & stream,int pictureLimit)
{
    stream.getNextStartCode();
    sequenceHeader seqHeader;
    unsigned char intraQuantizationMatrix[64];
    unsigned char nonIntraQuantizationMatrix[64];
    gopHeader gHeader;
    int pictureNum=0;
    while(stream.next_start_code==sequence_header_code)
    {
        get_sequence_header(stream,&seqHeader,intraQuantizationMatrix,nonIntraQuantizationMatrix);
        while(stream.next_start_code==group_start_code)
        {
            group_of_pictures(stream,&gHeader,pictureNum,pictureLimit);
        }
    }
}

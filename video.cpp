#include"video.h"
#include"code.h"
#include"vlc.h"
#include<cstdlib>
#include"decoder.h"
#include<cmath>
#include<queue>
#include <opencv2/opencv.hpp>

#define lum_motion_vector 1
#define chr_motion_vector 2

using namespace std;
using namespace cv;

static vlc vlcDecoder;
static const int default_intra_quantization_matrix[64]={
    8,16,19,22,26,27,29,34,
    16,16,22,24,27,29,34,37,
    19,22,26,27,29,34,34,38,
    22,22,26,27,29,34,37,40,
    22,26,27,29,32,35,40,48,
    26,27,29,32,35,40,48,58,
    26,27,29,34,38,46,56,69,
    27,29,35,38,46,56,69,83
};

static const int default_non_intra_quantization_matrix[64]={
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16
};

static int intraQuantizer[64];
static int nonIntraQuantizer[64];

bool isEnd;
queue<Mat *> *displayBuffer;
sequenceHeader seqHeader;


int get_sequence_header(dataStream &stream,sequenceHeader *header)
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

    //If video define its own quantization matrices,we need to load quantization matrices,
    //else we use default quantization matrices.
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

    //If Sequence Header has extension and user data,deocoder need to get it.
    if(stream.next_start_code==extension_start_code)
        getExtensionData(stream);
    if(stream.next_start_code==user_data_start_code)
        getUserData(stream);


    return 0;
}



void getExtensionData(dataStream &stream)
{
    unsigned int bits=stream.getBits(24);
   
    while(bits!=256) // nextbits () != '0000 0000 0000 0000 0000 0001'
    {
        bits=(bits<<8)^(stream.getBits(8)>>16);
    }
    stream.getNextStartCode();
}

void getUserData(dataStream &stream)
{
    unsigned int bits=stream.getBits(24);
    
    while(bits!=256)// nextbits () != '0000 0000 0000 0000 0000 0001'
    {
        bits=(bits<<8)^(stream.getBits(8)>>16);
    }
    stream.getNextStartCode();
}

picture *forPicture,*backPicture,*curPicture;

int group_of_pictures(dataStream &stream,gopHeader *gop)
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
        get_picture(stream,&pHeader);
    return 0;
}
int get_picture(dataStream &stream,pictureHeader *header)
{

    unsigned char tmp;
    header->temporal_reference=stream.getBits(10)>>(32-10);
    header->picture_coding_type=stream.getBits(3)>>(32-3);
    header->vbv_delay=stream.getBits(16)>>(16);
    
    //When the picture is I picture or B picture,need to get forward data
    if(header->picture_coding_type==P_picture_type||header->picture_coding_type==B_picture_type)
    {
        header->full_pel_forward_vector=stream.getFirstBit();
        tmp=stream.getBits(3)>>(32-3);
        header->forward_r_size=tmp-1;
        header->forward_f=1<<(tmp-1);
    }
    //When the picture is  B picture,need to get backward data
    if(header->picture_coding_type==B_picture_type)
    {
        header->full_pel_backward_vector=stream.getFirstBit();
        tmp=stream.getBits(3)>>(32-3);
        header->backward_r_size=tmp-1;
        header->backward_f=1<<(tmp-1);
    }


    //extra_bit_picture 1 
    while(stream.getFirstBit())
    {
        stream.getBits(8);//extra_information_picture 8
    }
    stream.getNextStartCode();

    //extension and user data
    if(stream.next_start_code==extension_start_code)
        getExtensionData(stream);
    if(stream.next_start_code==user_data_start_code)
        getUserData(stream);

    initialCur(seqHeader.vertical_size,seqHeader.horizotal_size);
    
    //When this picture is I picuture or P picture,move the last forward picture to display buffer,
    // and make the last backward picture to be the forward picture
    if(header->picture_coding_type==I_picture_type||header->picture_coding_type==P_picture_type)
    {
        if(forPicture)
        {        
            free(forPicture->data);
            free(forPicture);
        }
        forPicture=backPicture;    

        if(forPicture)
            pictureToBuffer(forPicture);
    }

    sliceHeader sHeader;
    while(stream.next_start_code>=slice_start_code_low&&stream.next_start_code<=slice_start_code_up)
    {
        get_slice(stream,&sHeader,header);
    }

    //When this picture is I picuture or P picture,current picture is backward piture.
    if(header->picture_coding_type==I_picture_type||header->picture_coding_type==P_picture_type)
        backPicture=curPicture;
    else if(header->picture_coding_type==B_picture_type)
    {
        pictureToBuffer(curPicture);
        free(curPicture->data);
        free(curPicture);
        curPicture=NULL;
    }

    return 0;
}

static int recon_right_for_prev,recon_down_for_prev;
static int recon_right_back_prev,recon_down_back_prev;
int get_slice(dataStream & stream,sliceHeader *header,pictureHeader *pHeader)
{
    header->slice_vertical_position=(unsigned char)(stream.next_start_code&0x000000FF);
    header->quantizer_scale=stream.getBits(5)>>(32-5);

    while(stream.getFirstBit())//extra_bit_slice 1
    {
        stream.getBits(8);  //extra_information_slice 8

    }

    //Initialize the data used for macroblock 
    int past_intra_address=-2;
    int dc_past[3]={1024,1024,1024};
    int previous_macroblock_address=(header->slice_vertical_position-1)*curPicture->mb_width-1;
    recon_right_for_prev=0,recon_down_for_prev=0;
    recon_right_back_prev=0,recon_down_back_prev=0;

    while(1)
    {
        get_macroblock(stream,pHeader,header,past_intra_address,previous_macroblock_address,dc_past);
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


int get_macroblock(dataStream &stream,pictureHeader *pHeader,sliceHeader *sHeader,int &past_intra_address,int &previous_macroblock_address,int dc_past[3])
{   
    unsigned short addressIncre=vlcDecoder.getMacroAddress(stream);
    unsigned short escape=0;
    yCbCr pels[256];
    yCbCr pels_back[256];
    int macroblock_address;
    while(addressIncre==macroblock_stuffing)
        addressIncre=vlcDecoder.getMacroAddress(stream);
    while(addressIncre==macroblock_escape)
    {   
        escape+=1;
        addressIncre=vlcDecoder.getMacroAddress(stream);
    }
    addressIncre=escape*33+addressIncre;
    macroblock_address=previous_macroblock_address+addressIncre;

    //Deocde skipped MacroBlock
    for(int i=1;macroblock_address-previous_macroblock_address-i>0;++i)
    {
        skipMacroBlock( *pHeader, previous_macroblock_address+i);
        //In P piture's skipped macroblck,recon_right_for_prev recon_down_for_prev set to 0
        if(pHeader->picture_coding_type==P_picture_type)
        {
            recon_right_for_prev=0;
            recon_down_for_prev=0;
        }
        for(int j=0;j<3;++j)
            dc_past[j]=128*8;
    }


    previous_macroblock_address=macroblock_address;

    macroblock_type type;
    type=vlcDecoder.getMacroType(stream,pHeader->picture_coding_type);

    if(type.quant)
    {
        sHeader->quantizer_scale=stream.getBits(5)>>(32-5);
    }


    int motion_horizontal_forward_code=0,motion_vertical_forward_code=0;
    unsigned char motion_horizontal_forward_r=0,motion_vertical_forward_r=0;
    int recon_right_for,recon_down_for;
    if(type.motion_forward)
    {
        motion_horizontal_forward_code=vlcDecoder.getMotionCode(stream);
        if(pHeader->forward_f!=1&&motion_horizontal_forward_code!=0)
            motion_horizontal_forward_r=stream.getBits(pHeader->forward_r_size)>>(32-pHeader->forward_r_size);
        motion_vertical_forward_code=vlcDecoder.getMotionCode(stream);
        if(pHeader->forward_f!=1&&motion_vertical_forward_code!=0)
            motion_vertical_forward_r=stream.getBits(pHeader->forward_r_size)>>(32-pHeader->forward_r_size);
        motionVectorFor(*pHeader, motion_horizontal_forward_code,motion_horizontal_forward_r,
            motion_vertical_forward_code, motion_vertical_forward_r,recon_right_for,recon_down_for);
    }
    else 
    {
        switch(pHeader->picture_coding_type)
        {
            case P_picture_type:
                recon_right_for_prev=0;
                recon_down_for_prev=0;
                recon_right_for=0;
                recon_down_for=0;
                break;
            case B_picture_type:
                recon_right_for=recon_right_for_prev;
                recon_down_for=recon_right_for_prev;

                break;
            default:
                break;
        };
    }


    int motion_horizontal_backward_code=0,motion_vertical_backward_code=0;
    unsigned char motion_horizontal_backward_r=0,motion_vertical_backward_r=0;
    int recon_right_back,recon_down_back;
    if(type.motion_backward)
    {
        motion_horizontal_backward_code=vlcDecoder.getMotionCode(stream);
        if ((pHeader->backward_f != 1)&&(motion_horizontal_backward_code != 0))
            motion_horizontal_backward_r=stream.getBits(pHeader->backward_r_size)>>(32-pHeader->backward_r_size);
        motion_vertical_backward_code=vlcDecoder.getMotionCode(stream);
        if ((pHeader->backward_f != 1)&&(motion_vertical_backward_code != 0))
            motion_vertical_backward_r=stream.getBits(pHeader->backward_r_size)>>(32-pHeader->backward_r_size);
        if(pHeader->picture_coding_type==B_picture_type)
        motionVectorBack(*pHeader, motion_horizontal_backward_code,motion_horizontal_backward_r,
            motion_vertical_backward_code, motion_vertical_backward_r,recon_right_back,recon_down_back);
    }
    else if(pHeader->picture_coding_type==B_picture_type)
    {
        recon_right_back=recon_right_back_prev;
        recon_down_back=recon_right_back_prev;
    }
    
    unsigned char cbp=0;
    if(type.pattern)
        cbp=vlcDecoder.getCBP(stream);
    bool pattern_code[6];
    for(int i=0;i<6;++i)
    {
        pattern_code[i] = 0;
        if ( cbp & (1<<(5-i)) ) 
            pattern_code[i] = 1;
        if (type.intra)pattern_code[i] = 1 ;
    }


    int blocks[6][64]={0};
    for(int i=0;i<6;++i)
    {
        get_block(stream,pattern_code,i,type,pHeader,blocks);
    }


    if(type.intra)
    {
        intraMacroBlockDecode(blocks,sHeader->quantizer_scale,macroblock_address,past_intra_address ,dc_past);
        zigZag(blocks);
        IDCT(blocks);
        levelShift(blocks);
        macroblockToPicture(blocks,macroblock_address);
        if(pHeader->picture_coding_type==B_picture_type)
        {
            recon_right_back_prev=0;
            recon_down_back_prev=0;
            recon_right_for_prev=0;
            recon_down_for_prev=0;
        }
    }
    else 
    {
        switch (pHeader->picture_coding_type)
        {
            case(P_picture_type):
                getPels(recon_right_for,recon_down_for,pels,macroblock_address,forPicture);
                break;
            case(B_picture_type):
                if(type.motion_forward &&type.motion_backward)
                {
                    getPels(recon_right_for,recon_down_for,pels,macroblock_address,forPicture);
                    getPels(recon_right_back,recon_down_back,pels_back,macroblock_address,backPicture);
                    for(int i=0;i<256;++i)
                    {    
                        pels[i].y= (pels[i].y+pels_back[i].y)/2;
                        pels[i].cb=(pels[i].cb+pels_back[i].cb)/2;
                        pels[i].cr= (pels[i].cr+pels_back[i].cr)/2;
                    }    
                }
                else if(type.motion_forward)
                    getPels(recon_right_for,recon_down_for,pels,macroblock_address,forPicture);
                else if(type.motion_backward) 
                    getPels(recon_right_back,recon_down_back,pels,macroblock_address,backPicture);
                break;
        }
        deQuantize(blocks,nonIntraQuantizer,sHeader->quantizer_scale,false);
        zigZag(blocks);
        IDCT(blocks);
        macroblockToPicture(blocks,macroblock_address,pels);
        for(int j=0;j<3;++j)
            dc_past[j]=1024;
    }

    if(pHeader->picture_coding_type==D_picture_type)
        stream.getFirstBit();

}


int get_block(dataStream & stream,bool pattern_code[6],int i,macroblock_type type,pictureHeader *pHeader,int blocks[6][64])
{
    unsigned char dct_dc_size;
    unsigned char dct_dc_differential;
    dct_coeff dctCoeff;
    int index=0;
    //Initialize block
    for(int k=0;k<64;++k)
    {
        blocks[i][k]=0;
    }
    if(pattern_code[i])
    {
        if(type.intra)
        {
            if (i<4)//block's lum
            {
                dct_dc_size=vlcDecoder.getDctDcSizeLum(stream);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                    if ( dct_dc_differential & ( 1 << (dct_dc_size-1)) ) blocks[i][0] =dct_dc_differential ;
                        else blocks[i][0] = ( -1 << (dct_dc_size) ) | (dct_dc_differential+1) ;

                }

            }
            else
            {
                dct_dc_size=vlcDecoder.getDctDcSizeChr(stream);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                    if ( dct_dc_differential & ( 1 << (dct_dc_size-1)) ) blocks[i][0] =dct_dc_differential ;
                        else blocks[i][0] = ( -1 << (dct_dc_size) ) | (dct_dc_differential+1) ;
                }
            }
        }
        else
        {
            dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_first);
            index=dctCoeff.run;
            blocks[i][index]=dctCoeff.level;
        }
        if(pHeader->picture_coding_type!=D_picture_type)
        {

            dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_next);
            while(dctCoeff.run!=end_of_block_run)
            {
                index+=(dctCoeff.run+1);
                blocks[i][index]=dctCoeff.level;
                dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_next);
            }
        }
    }
}


int video_sequence(dataStream & stream,queue<Mat *> *buffer)
{
    stream.getNextStartCode();
    unsigned char intraQuantizationMatrix[64];
    unsigned char nonIntraQuantizationMatrix[64];
    gopHeader gHeader;
    forPicture =NULL,backPicture=NULL,curPicture=NULL;
    isEnd=false;
    displayBuffer=buffer;
    while(stream.next_start_code==sequence_header_code)
    {
        get_sequence_header(stream,&seqHeader);
        while(stream.next_start_code==group_start_code)
        {
            group_of_pictures(stream,&gHeader);
        }
    }
    isEnd=true;
}

void macroblockToPicture(int macroblock[6][64],int macroblock_address)
{
    int mb_row = macroblock_address /curPicture->mb_width;
    int mb_column = macroblock_address % curPicture->mb_width;
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i;
            int col=mb_column*16+j;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[0][i*8+j];
        }

    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i;
            int col=mb_column*16+j+8;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[1][i*8+j];
        }

    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i+8;
            int col=mb_column*16+j;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[2][i*8+j];
        }

    
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i+8;
            int col=mb_column*16+j+8;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[3][i*8+j];
        }

    
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            for(int k=0;k<2;++k)
                for(int p=0;p<2;++p)
                {
                    int row=mb_row*16+i*2+k;
                    int col=mb_column*16+j*2+p;
                    if(row>=curPicture->height||col>=curPicture->width)
                        continue;
                    curPicture->data[row*curPicture->width+col].cb=macroblock[4][i*8+j];
                    curPicture->data[row*curPicture->width+col].cr=macroblock[5][i*8+j];
                }
        }
}

void macroblockToPicture(int macroblock[6][64],int macroblock_address,yCbCr pel[256])
{
    int mb_row = macroblock_address /curPicture->mb_width;
    int mb_column = macroblock_address % curPicture->mb_width;
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i;
            int col=mb_column*16+j;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[0][i*8+j]+pel[i*16+j].y;
            if(curPicture->data[row*curPicture->width+col].y<0)
                curPicture->data[row*curPicture->width+col].y=0;
            if(curPicture->data[row*curPicture->width+col].y>255)
                curPicture->data[row*curPicture->width+col].y=255;
        }

    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i;
            int col=mb_column*16+j+8;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[1][i*8+j]+pel[i*16+j+8].y;
            if(curPicture->data[row*curPicture->width+col].y<0)
                curPicture->data[row*curPicture->width+col].y=0;
            if(curPicture->data[row*curPicture->width+col].y>255)
                curPicture->data[row*curPicture->width+col].y=255;
        }

    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i+8;
            int col=mb_column*16+j;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[2][i*8+j]+pel[(i+8)*16+j].y;
            if(curPicture->data[row*curPicture->width+col].y<0)
                curPicture->data[row*curPicture->width+col].y=0;
            if(curPicture->data[row*curPicture->width+col].y>255)
                curPicture->data[row*curPicture->width+col].y=255;
        }

    
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            int row=mb_row*16+i+8;
            int col=mb_column*16+j+8;
            if(row>=curPicture->height||col>=curPicture->width)
                continue;
            curPicture->data[row*curPicture->width+col].y=macroblock[3][i*8+j]+pel[(i+8)*16+j+8].y;
            if(curPicture->data[row*curPicture->width+col].y<0)
                curPicture->data[row*curPicture->width+col].y=0;
            if(curPicture->data[row*curPicture->width+col].y>255)
                curPicture->data[row*curPicture->width+col].y=255;
        }

    
    for(int i=0;i<8;++i)
        for(int j=0;j<8;++j)
        {
            for(int k=0;k<2;++k)
                for(int p=0;p<2;++p)
                {
                    int row=mb_row*16+i*2+k;
                    int col=mb_column*16+j*2+p;
                    if(row>=curPicture->height||col>=curPicture->width)
                        continue;
                    curPicture->data[row*curPicture->width+col].cb=macroblock[4][i*8+j]+pel[(i*2+k)*16+j*2+p].cb;
                    curPicture->data[row*curPicture->width+col].cr=macroblock[5][i*8+j]+pel[(i*2+k)*16+j*2+p].cr;
                    if(curPicture->data[row*curPicture->width+col].cr<0)
                        curPicture->data[row*curPicture->width+col].cr=0;
                    if(curPicture->data[row*curPicture->width+col].cr>255)
                        curPicture->data[row*curPicture->width+col].cr=255;
                    if(curPicture->data[row*curPicture->width+col].cb<0)
                        curPicture->data[row*curPicture->width+col].cb=0;
                    if(curPicture->data[row*curPicture->width+col].cb>255)
                        curPicture->data[row*curPicture->width+col].cb=255;
                }
        }
}


void intraMacroBlockDecode(int blocks[6][64],int quantizer_scale,int macroblock_address,int &past_intra_address ,int dc_past[6])
{
    deQuantize(blocks,intraQuantizer,quantizer_scale,true);
    if ( ( macroblock_address - past_intra_address > 1) ) 
    {
        blocks[0][0]=128*8+blocks[0][0];
        blocks[4][0]=128*8+blocks[4][0];
        blocks[5][0]=128*8+blocks[5][0];
    }
    else
    {
        blocks[0][0]=dc_past[0]+blocks[0][0];
        blocks[4][0]=dc_past[1]+blocks[4][0];
        blocks[5][0]=dc_past[2]+blocks[5][0];
    }
    dc_past[0]=blocks[0][0];
    dc_past[1]=blocks[4][0];
    dc_past[2]=blocks[5][0];

    for(int i=1;i<4;++i)
    {
        blocks[i][0]=dc_past[0]+blocks[i][0];
        dc_past[0]=blocks[i][0];
    }
    past_intra_address = macroblock_address ;
}


void initialCur(int vertical_size,int horizotal_size)
{
    curPicture=(picture *)malloc(sizeof(picture));
    curPicture->data=(yCbCr*)malloc(sizeof(yCbCr)*vertical_size*horizotal_size);
    curPicture->height=vertical_size;
    curPicture->width=horizotal_size;
    curPicture->mb_height=(vertical_size+15)/16;
    curPicture->mb_width=(horizotal_size+15)/16;
}
yCbCr getPictureData(picture *p,int row,int col)
{
    yCbCr result;
    result.y=0;
    result.cb=0;
    result.cr=0;
    if(row<(p->height)&&col<(p->width))
    {
        result.y=p->data[row*p->width+col].y;
        result.cb=p->data[row*p->width+col].cb;
        result.cr=p->data[row*p->width+col].cr;
    }
    return result;
}


void getPels(int recon_right,int recon_down,yCbCr pels[256] ,int macroblock_address,picture *refPicture)
{

    int mb_row=macroblock_address/(refPicture->mb_width);
    int mb_column=macroblock_address%(refPicture->mb_width);
    int right = recon_right >> 1 ; 
    int down = recon_down >> 1 ; 
    int right_half = recon_right - 2*right ; 
    int down_half = recon_down- 2*down ;

    
    int right_chr = (recon_right/2) >> 1 ; 
    int down_chr = (recon_down/2) >> 1 ; 
    int right_half_chr = recon_right/2 - 2*right_chr ; 
    int down_half_chr = recon_down/2- 2*down_chr ;

    int row,col;
    for(int i=0;i<16;++i)
        for(int j=0;j<16;++j)
        {
            pels[i*16+j].y=0;
            pels[i*16+j].cb=0;
            pels[i*16+j].cr=0;

            row=mb_row*16+i+down;
            col=mb_column*16+j+right;
            if ( !right_half && !down_half )
                pels[i*16+j].y=getPictureData(refPicture,row,col).y;
            if ( !right_half && down_half )
                pels[i*16+j].y=(getPictureData(refPicture,row,col).y+getPictureData(refPicture,row+1,col).y)/2;
            if ( right_half && !down_half )
                pels[i*16+j].y=(getPictureData(refPicture,row,col).y+getPictureData(refPicture,row,col+1).y)/2;
            if ( right_half && down_half )
                pels[i*16+j].y=(getPictureData(refPicture,row,col).y+getPictureData(refPicture,row+1,col).y+
                getPictureData(refPicture,row,col+1).y+getPictureData(refPicture,row+1,col+1).y)/4;

            row=mb_row*16+i+down_chr;
            col=mb_column*16+j+right_chr;
            if ( !right_half_chr && !down_half_chr )
            {    
                pels[i*16+j].cb=getPictureData(refPicture,row,col).cb;
                pels[i*16+j].cr=getPictureData(refPicture,row,col).cr;
            }
            if ( !right_half_chr && down_half_chr)
            {
                pels[i*16+j].cb=(getPictureData(refPicture,row,col).cb+getPictureData(refPicture,row+1,col).cb)/2;
                pels[i*16+j].cr=(getPictureData(refPicture,row,col).cr+getPictureData(refPicture,row+1,col).cr)/2;
            }
            if ( right_half_chr&& ! down_half_chr)
            {
                pels[i*16+j].cb=(getPictureData(refPicture,row,col).cb+getPictureData(refPicture,row,col+1).cb)/2;
                pels[i*16+j].cr=(getPictureData(refPicture,row,col).cr+getPictureData(refPicture,row,col+1).cr)/2;
            }
            if ( right_half_chr&& down_half_chr)
            {
                pels[i*16+j].cb=(getPictureData(refPicture,row,col).cb+getPictureData(refPicture,row+1,col).cb+
                getPictureData(refPicture,row,col+1).cb+getPictureData(refPicture,row+1,col+1).cb)/4;
                pels[i*16+j].cr=(getPictureData(refPicture,row,col).cr+getPictureData(refPicture,row+1,col).cr+
                getPictureData(refPicture,row,col+1).cr+getPictureData(refPicture,row+1,col+1).cr)/4;
            }

        }

}


void motionVectorFor(pictureHeader pHeader,int motion_horizontal_forward_code,
    int motion_horizontal_forward_r,int motion_vertical_forward_code,int motion_vertical_forward_r,int &recon_right_for,int &recon_down_for)
{
    int complement_horizontal_forward_r,complement_vertical_forward_r;
    if (pHeader.forward_f == 1 || motion_horizontal_forward_code == 0) 
    {
        complement_horizontal_forward_r = 0;
    } 
    else 
    {
        complement_horizontal_forward_r = pHeader.forward_f - 1 - motion_horizontal_forward_r;
    }
    if (pHeader.forward_f == 1 || motion_vertical_forward_code == 0) 
    {   
        complement_vertical_forward_r = 0;
    } 
    else 
    {
        complement_vertical_forward_r = pHeader.forward_f - 1 - motion_vertical_forward_r;
    }
    int right_little = motion_horizontal_forward_code *pHeader.forward_f,right_big;
    if (right_little == 0) 
        right_big = 0; 
    else 
    {
        if (right_little > 0) 
        {
            right_little = right_little - complement_horizontal_forward_r ;
            right_big = right_little - 32 * pHeader.forward_f;
        } 
        else 
        {
            right_little = right_little + complement_horizontal_forward_r ;
            right_big = right_little + 32 * pHeader.forward_f;
        }
    }


    int down_little = motion_vertical_forward_code * pHeader.forward_f,down_big;
    if (down_little == 0) 
        down_big = 0;
    else 
    {
        if (down_little > 0) 
        {
            down_little = down_little - complement_vertical_forward_r ;
            down_big = down_little - 32 * pHeader.forward_f;
        }  
        else 
        {
            down_little = down_little + complement_vertical_forward_r ;
            down_big = down_little + 32 * pHeader.forward_f;
        }
    }
    int max = ( 16 *pHeader.forward_f ) - 1 ;
    int min = ( -16 *pHeader.forward_f ) ;
    
    int new_vector = recon_right_for_prev + right_little ;
    if ( new_vector <= max && new_vector >= min )
        recon_right_for = recon_right_for_prev + right_little ;
    else
        recon_right_for = recon_right_for_prev + right_big ;
    recon_right_for_prev = recon_right_for ;
    if (pHeader. full_pel_forward_vector ) recon_right_for = recon_right_for << 1 ; 
        new_vector = recon_down_for_prev + down_little ;
    if ( new_vector <= max && new_vector >= min )
        recon_down_for = recon_down_for_prev + down_little ;
    else
        recon_down_for = recon_down_for_prev + down_big ;
    recon_down_for_prev = recon_down_for ;
    if (pHeader.full_pel_forward_vector ) 
        recon_down_for = recon_down_for << 1 ;
}

void skipMacroBlock(pictureHeader pHeader,int macroblock_address)
{
    int recon_right_for=0,recon_down_for=0;
    int recon_right_back=0,recon_down_back=0;
    yCbCr pels[256],pels_back[256];
    int macroblock[6][64];
    if(pHeader.picture_coding_type==P_picture_type)
        getPels(recon_right_for,recon_down_for,pels,macroblock_address,forPicture);
    if(pHeader.picture_coding_type==B_picture_type)
    {
        recon_right_for=recon_down_for_prev;
        recon_down_for=recon_down_for_prev;
        recon_down_back=recon_down_back_prev;
        recon_right_back=recon_right_back_prev;
        getPels(recon_right_for,recon_down_for,pels,macroblock_address,forPicture);
        getPels(recon_right_back,recon_down_back,pels_back,macroblock_address,backPicture);
        for(int i=0;i<256;++i)
        {
            pels[i].y= (pels[i].y+pels_back[i].y)/2;
            pels[i].cb=(pels[i].cb+pels_back[i].cb)/2;
            pels[i].cr= (pels[i].cr+pels_back[i].cr)/2;
        }

    }
    for(int i=0;i<6;++i)
        for(int j=0;j<64;++j)
            macroblock[i][j]=0;

    macroblockToPicture(macroblock,macroblock_address,pels);
}


void motionVectorBack(pictureHeader pHeader,int motion_horizontal_backward_code,
    int motion_horizontal_backward_r,int motion_vertical_backward_code,int motion_vertical_backward_r,int &recon_right_back,int &recon_down_back)
{
    int complement_horizontal_backward_r,complement_vertical_backward_r;
    if (pHeader.backward_f == 1 || motion_horizontal_backward_code == 0) 
    {
        complement_horizontal_backward_r = 0;
    } 
    else 
    {
        complement_horizontal_backward_r = pHeader.backward_f - 1 - motion_horizontal_backward_r;
    }
    if (pHeader.backward_f == 1 || motion_vertical_backward_code == 0) 
    {   
        complement_vertical_backward_r = 0;
    } 
    else 
    {
        complement_vertical_backward_r = pHeader.backward_f - 1 - motion_vertical_backward_r;
    }
    int right_little = motion_horizontal_backward_code *pHeader.backward_f,right_big;
    if (right_little == 0) 
        right_big = 0; 
    else 
    {
        if (right_little > 0) 
        {
            right_little = right_little - complement_horizontal_backward_r ;
            right_big = right_little - 32 * pHeader.backward_f;
        } 
        else 
        {
            right_little = right_little + complement_horizontal_backward_r ;
            right_big = right_little + 32 * pHeader.backward_f;
        }
    }


    int down_little = motion_vertical_backward_code * pHeader.backward_f,down_big;
    if (down_little == 0) 
        down_big = 0;
    else 
    {
        if (down_little > 0) 
        {
            down_little = down_little - complement_vertical_backward_r ;
            down_big = down_little - 32 * pHeader.backward_f;
        }  
        else 
        {
            down_little = down_little + complement_vertical_backward_r ;
            down_big = down_little + 32 * pHeader.backward_f;
        }
    }
    int max = ( 16 *pHeader.backward_f ) - 1 ;
    int min = ( -16 *pHeader.backward_f ) ;
    
    int new_vector = recon_right_back_prev + right_little ;
    if ( new_vector <= max && new_vector >= min )
        recon_right_back = recon_right_back_prev + right_little ;
    else
        recon_right_back = recon_right_back_prev + right_big ;
    recon_right_back_prev = recon_right_back ;
    if (pHeader. full_pel_backward_vector ) recon_right_back = recon_right_back << 1 ; 
        new_vector = recon_down_back_prev + down_little ;
    if ( new_vector <= max && new_vector >= min )
        recon_down_back = recon_down_back_prev + down_little ;
    else
        recon_down_back = recon_down_back_prev + down_big ;
    recon_down_back_prev = recon_down_back ;
    if (pHeader.full_pel_backward_vector ) 
        recon_down_back = recon_down_back << 1 ;
}

bool videoIsEnd()
{
    return isEnd;
}

void pictureToBuffer(picture *inputPicture)
{
    Mat *buffer=new Mat(inputPicture->height,inputPicture->width,CV_8UC3);
    YCbCrtoRGB(inputPicture,buffer);
    displayBuffer->push(buffer);  
}

sequenceHeader getSeqHeader()
{
    return seqHeader;
}
#include"video.h"
#include"code.h"
#include"vlc.h"
#include<cstdlib>
#include"decoder.h"



#define lum_motion_vector 1
#define chr_motion_vector 2

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

static picture *forPicture,*backPicture,*curPicture;
int group_of_pictures(dataStream &stream,sequenceHeader header,gopHeader *gop,int &pictureNumber,int limit ,picture &tmp)
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
        get_picture(stream,header,&pHeader);
        if(pictureNumber>=limit)
        {
            tmp=*curPicture;
            break;
        }
    }
    return 0;
}

int get_picture(dataStream &stream,sequenceHeader seqHeader,pictureHeader *header)
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
 //   printf("temporal_reference %d\n",header->temporal_reference);
   // printf("picture_coding_type %d\n",header->picture_coding_type);
   // printf("vbv_delay %d\n",header->vbv_delay);


    initialCur(seqHeader.vertical_size,seqHeader.horizotal_size);

    if(header->picture_coding_type==I_picture_type||header->picture_coding_type==P_picture_type)
    {
        forPicture=backPicture;
        if(forPicture)
        {
            //display();
            free(forPicture->data);
            free(forPicture);
            forPicture=NULL;
        }
    }


    sliceHeader sHeader;
    while(stream.next_start_code>=slice_start_code_low&&stream.next_start_code<=slice_start_code_up)
    {
        get_slice(stream,&sHeader,header);
    }

    if(header->picture_coding_type==I_picture_type||header->picture_coding_type==P_picture_type)
        backPicture=curPicture;
    else if(header->picture_coding_type==B_picture_type)
    {
        //display();
        free(curPicture->data);
        free(curPicture);
        curPicture=NULL;
    }

    return 0;
}

static int recon_right_for_prev,recon_down_for_prev;
int get_slice(dataStream & stream,sliceHeader *header,pictureHeader *pHeader)
{
 //   printf("slice code  %x \n",stream.next_start_code);
    header->slice_vertical_position=(unsigned char)(stream.next_start_code&0x000000FF);
    header->quantizer_scale=stream.getBits(5)>>(32-5);

    while(stream.getFirstBit())
    {
        stream.getBits(8);
    }
 //   printf("quantizer_scale %d\n",header->quantizer_scale);
    int past_intra_address=-2;
    int dc_past[3]={128*8,128*8,128*8};
    int previous_macroblock_address=(header->slice_vertical_position-1)*curPicture->mb_width-1;
    recon_right_for_prev=0,recon_down_for_prev=0;
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
 //   printf("macroblock  %d \n",macroblock_address+1);
 //   printf("address increment  %d\n",addressIncre);
    macroblock_type type;
    type=vlcDecoder.getMacroType(stream,pHeader->picture_coding_type);
 //   printf("type.quant %d\n",type.quant);
 //   printf("type.motion_forward %d\n",type.motion_forward);
 //   printf("type.motion_backward %d\n",type.motion_backward);
//    printf("type.pattern %d\n",type.pattern);
//    printf("type.intra %d\n",type.intra);
    if(type.quant)
    {
        sHeader->quantizer_scale=stream.getBits(5)>>(32-5);
 //       printf("quantizer_scale %d\n");
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
  ///      printf("motion_horizontal_forward_code %d\n",motion_horizontal_forward_code);
 //       printf("motion_vertical_forward_code %d\n",motion_vertical_forward_code);

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
    //printf("pattern code \n");
    for(int i=0;i<6;++i)
    {
        pattern_code[i] = 0;
        if ( cbp & (1<<(5-i)) ) 
            pattern_code[i] = 1;
        if (type.intra)pattern_code[i] = 1 ;
       // printf("%d ", pattern_code[i]);
    }
   // printf("\n");

    int blocks[6][64]={0};
/*    for(int i=0;i<6;++i)
        blocks[i]=(int *)malloc(sizeof(int)*64);*/
   // printf("macr %d\n",macroblock_address);
    for(int i=0;i<6;++i)
    {
      //  printf("blocks %d\n" ,i);
        get_block(stream,pattern_code,i,type,pHeader,blocks);
      /*  for(int k=0;k<8;++k)
        {
            for(int z=0;z<8;++z)
                printf("%d ",blocks[i][k*8+z]);
            printf("\n");
        }*/
    }
    if(type.intra)
        intraMacroBlockDecode(blocks,sHeader->quantizer_scale,macroblock_address,past_intra_address ,dc_past);

   
    //printf("macr %d\n",macroblock_address);
    zigZag(blocks);
    IDCT(blocks);
/*
        for(int k=0;k<8;++k)
        {
            
            for(int z=0;z<8;++z)
                printf("%d ",blocks[i][k*8+z]);
            printf("\n");
        }   */
    levelShift(blocks);
    macroblockToPicture(blocks,macroblock_address);

    

    if(pHeader->picture_coding_type==D_picture_type)
        stream.getFirstBit();
   /* for(int i=0;i<6;++i)
        free(blocks[i]);
*/
    previous_macroblock_address=macroblock_address;
}


int get_block(dataStream & stream,bool pattern_code[6],int i,macroblock_type type,pictureHeader *pHeader,int blocks[6][64])
{
   // printf("block %d \n",i);
    unsigned char dct_dc_size;
    unsigned char dct_dc_differential;
    dct_coeff dctCoeff;
    int index=0;
    for(int k=0;k<64;++k)
    {
        blocks[i][k]=0;
 
    }
    if(pattern_code[i])
    {
        if(type.intra)
        {
            if (i<4)
            {
                dct_dc_size=vlcDecoder.getDctDcSizeLum(stream);
              //  printf("dct_dc_size lum  %d\n",dct_dc_size);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                  //  printf("dct_dc diff %d\n",dct_dc_differential);
                    if ( dct_dc_differential & ( 1 << (dct_dc_size-1)) ) blocks[i][0] =dct_dc_differential ;
                        else blocks[i][0] = ( -1 << (dct_dc_size) ) | (dct_dc_differential+1) ;

                }

            }
            else
            {
                dct_dc_size=vlcDecoder.getDctDcSizeChr(stream);
             //    printf("dct_dc_size chr  %d\n",dct_dc_size);
                if(dct_dc_size!=0)
                {
                    dct_dc_differential=stream.getBits(dct_dc_size)>>(32-dct_dc_size);
                  //  printf("dct_dc diff %d\n",dct_dc_differential);
                    if ( dct_dc_differential & ( 1 << (dct_dc_size-1)) ) blocks[i][0] =dct_dc_differential ;
                        else blocks[i][0] = ( -1 << (dct_dc_size) ) | (dct_dc_differential+1) ;
                }
            }
        }
        else
        {
            dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_first);
         //   printf("dct_coeff_first run : %d level : %d \n ",dctCoeff.run,dctCoeff.level);
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
            //    printf("dct_coeff_next : %d level : %d \n",dctCoeff.run,dctCoeff.level);
                dctCoeff=vlcDecoder.getDctCoeff(stream,dct_coeff_next);
            }
         //   printf("EOB\n");
        }
    }
}


int video_sequence(dataStream & stream,int pictureLimit,picture &tmp)
{
    stream.getNextStartCode();
    sequenceHeader seqHeader;
    unsigned char intraQuantizationMatrix[64];
    unsigned char nonIntraQuantizationMatrix[64];
    gopHeader gHeader;
    int pictureNum=0;
    forPicture =NULL,backPicture=NULL,curPicture=NULL;
    while(stream.next_start_code==sequence_header_code)
    {
        get_sequence_header(stream,&seqHeader);
        while(stream.next_start_code==group_start_code)
        {
            group_of_pictures(stream,seqHeader,&gHeader,pictureNum,pictureLimit, tmp);
            if(pictureNum>=pictureLimit) 
                return 0;
        }
    }
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


void intraMacroBlockDecode(int blocks[6][64],int quantizer_scale,int macroblock_address,int &past_intra_address ,int dc_past[6])
{
    deQuantize(blocks,intraQuantizer,quantizer_scale);
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


void motionVectorP(pictureHeader pHeader,int motion_horizontal_forward_code,
    int motion_horizontal_forward_r,int motion_vertical_forward_code,int motion_vertical_forward_r,yCbCr pels[256]
    ,int mb_row, int mb_column )
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
    int recon_right_for,recon_down_for;
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

    int right_for = recon_right_for >> 1 ; 
    int down_for = recon_down_for >> 1 ; 
    int right_half_for = recon_right_for - 2*right_for ; 
    int down_half_for = recon_down_for- 2*down_for ;
    int right_for_chr = (recon_right_for/2) >> 1 ; 
    int down_for_chr = (recon_down_for/2) >> 1 ; 
    int right_half_for_chr = recon_right_for/2 - 2*right_for ; 
    int down_half_for_chr = recon_down_for/2- 2*down_for ;

    int row,col;
    for(int i=0;i<16;++i)
        for(int j=0;j<16;++j)
        {
            pels[i*16+j].y=0;
            pels[i*16+j].cb=0;
            pels[i*16+j].cr=0;

            row=mb_row*16+i+down_for;
            col=mb_column*16+j+right_for;
            if ( ! right_half_for && ! down_half_for )
                pels[i*16+j].y=getPictureData(forPicture,row,col).y;
            if ( ! right_half_for && down_half_for )
                pels[i*16+j].y=(getPictureData(forPicture,row,col).y+getPictureData(forPicture,row+1,col).y)/2;
            if ( right_half_for && ! down_half_for )
                pels[i*16+j].y=(getPictureData(forPicture,row,col).y+getPictureData(forPicture,row,col+1).y)/2;
            if ( right_half_for && down_half_for )
                pels[i*16+j].y=(getPictureData(forPicture,row,col).y+getPictureData(forPicture,row+1,col).y+
                getPictureData(forPicture,row,col+1).y+getPictureData(forPicture,row+1,col+1).y)/2;

            if ( ! right_half_for_chr && ! down_half_for_chr )
            {    
                pels[i*16+j].cb=getPictureData(forPicture,row,col).cb;
                pels[i*16+j].cr=getPictureData(forPicture,row,col).cr;
            }
            if ( ! right_half_for_chr && down_half_for_chr)
            {
                pels[i*16+j].cb=(getPictureData(forPicture,row,col).cb+getPictureData(forPicture,row+1,col).cb)/2;
                pels[i*16+j].cr=(getPictureData(forPicture,row,col).cr+getPictureData(forPicture,row+1,col).cr)/2;
            }
            if ( right_half_for_chr&& ! down_half_for_chr)
            {
                pels[i*16+j].cb=(getPictureData(forPicture,row,col).cb+getPictureData(forPicture,row,col+1).cb)/2;
                pels[i*16+j].cr=(getPictureData(forPicture,row,col).cr+getPictureData(forPicture,row,col+1).cr)/2;
            }
            if ( right_half_for_chr&& down_half_for_chr)
            {
                pels[i*16+j].cb=(getPictureData(forPicture,row,col).cb+getPictureData(forPicture,row+1,col).cb+
                getPictureData(forPicture,row,col+1).cb+getPictureData(forPicture,row+1,col+1).cb)/2;
                pels[i*16+j].cr=(getPictureData(forPicture,row,col).cr+getPictureData(forPicture,row+1,col).cr+
                getPictureData(forPicture,row,col+1).cr+getPictureData(forPicture,row+1,col+1).cr)/2;
            }

        }

}
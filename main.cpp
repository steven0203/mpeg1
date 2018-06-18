#include"dataStream.h"
#include<cstdio>
#include"decoder.h"



int main(int argc,char ** argv)
{
    FILE * inputFile=fopen("MPEG_Files/IPB_ALL.M1V","rb");
    if(inputFile==NULL)
    {
        fputs ("Input file error",stderr); 
        return 0;
    }
    dataStream stream(inputFile);
    video_sequence(stream,4);
   /* printf("%x\n",stream.next_start_code);
    pictureHeader tmp2;
    if(stream.next_start_code==picture_start_code)
    {
        get_picture(stream,&tmp2);
        printf("%x\n",stream.next_start_code);
    }
    sliceHeader tmp3;
    if(stream.next_start_code>=slice_start_code_low&&stream.next_start_code<=slice_start_code_up)
    {
        get_slice(stream,&tmp3,stream.next_start_code,&tmp2);
        printf("%x\n",stream.next_start_code);
    }*/
    fclose(inputFile);
}
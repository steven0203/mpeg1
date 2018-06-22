#include"dataStream.h"
#include<cstdio>
#include"video.h"
#include <opencv2/opencv.hpp>
#include"decoder.h"
using namespace cv;

int main(int argc,char ** argv)
{
    FILE * inputFile=fopen("MPEG_Files/I_ONLY.M1V","rb");
    if(inputFile==NULL)
    {
        fputs ("Input file error",stderr); 
        return 0;
    }
    picture tmp;
    dataStream stream(inputFile);
    int limit=1;
    video_sequence(stream,5,tmp);

    Mat *img=new Mat();
    img->create(tmp.height,tmp.width,CV_8UC3);

    YCbCrtoRGB(tmp,img);

    while(true){
        imshow("Display window", *img);     
        if(cvWaitKey(10)==27){                      
            break;
        }
    }
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
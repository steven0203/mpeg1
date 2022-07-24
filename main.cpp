#include"dataStream.h"
#include<cstdio>
#include"video.h"
#include <opencv2/opencv.hpp>
#include"decoder.h"
#include<ctime>
#include<pthread.h>
#include<queue>
//#include<windows.h>
#include <unistd.h>

#include<cstdlib>
#include"code.h"

using namespace cv;
using namespace std;

double pictue_rate_define[8]={
    23.976,24,25,29.97,30,50,59.94,60
};

typedef struct
{
    dataStream inputStream;
    queue<Mat *>*videoBuffer;
}videoArgs;


void* videoThread(void *);



int main(int argc,char ** argv)
{
    //Check the arguments and whether the file can open.
    if(argc<2)
    {
        printf("Too few arguments\n");
        return 0;
    }
    FILE * inputFile=fopen(argv[1],"rb");
    if(inputFile==NULL)
    {
        fputs ("Input file error",stderr); 
        return 0;
    }
    dataStream stream(inputFile);
    
    time_t t1=time(NULL);

    //Create thread for decoding the video.
    queue<Mat *>*videoDisplayBuffer=new queue<Mat *>(); //Buffer used for store the deocded data
    videoArgs *vArgs=(videoArgs*)malloc(sizeof(vArgs));
    vArgs->inputStream=stream;
    vArgs->videoBuffer=videoDisplayBuffer;
    pthread_t vThread;    
    pthread_create(&vThread, NULL, videoThread,(void*)vArgs); 


    usleep(800000); 
    sequenceHeader videoHeader=getSeqHeader();
    int delay=1000/pictue_rate_define[videoHeader.picture_rate]-10;
    Mat *display;

    while(true){
        //If display buffer is empty and video decoding haven't ended,
        // it need to wait util video have enough data 
        while(videoDisplayBuffer->empty()&&!videoIsEnd())
        {
            printf("xx\n");
            sleep(100);
        }
        if(videoDisplayBuffer->empty()&&videoIsEnd())
            break;
        display=videoDisplayBuffer->front();
        videoDisplayBuffer->pop();
        imshow("Display window", *display); 
        delete display;

        if(waitKey(delay)==27){                      
            break;
        }
    }

    pthread_join(vThread, NULL);
    printf("%d\n",time(NULL)-t1);
    fclose(inputFile);
}


void* videoThread(void * args)
{
    video_sequence(((videoArgs*)args)->inputStream,((videoArgs*)args)->videoBuffer);
    pthread_exit(NULL); 
}

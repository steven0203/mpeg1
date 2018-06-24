#ifndef VIEDO_H
#define VIEDO_H
#include"dataStream.h"
#include"code.h"
#include"vlc.h"
#include<queue>
#include <opencv2/opencv.hpp>
using namespace cv;


void getExtensionData(dataStream &);
void getUserData(dataStream &);

using namespace std;


/*Get sequence header and quantization matrix. */
int get_sequence_header(dataStream &,sequenceHeader *);
int group_of_pictures(dataStream & ,gopHeader *);
int get_picture(dataStream &,pictureHeader *);
int get_slice(dataStream &,sliceHeader *,pictureHeader *);
int get_macroblock(dataStream &,pictureHeader*,sliceHeader*,int &,int &,int [3]);
int get_block(dataStream &,bool [6],int ,macroblock_type,pictureHeader *,int [6][64]);
int video_sequence(dataStream &,queue<Mat *> *);
void macroblockToPicture(int [6][64],int );
void intraMacroBlockDecode(int [6][64],int ,int ,int & ,int [6]);
void initialCur(int,int);
yCbCr getPictureData(picture *,int,int,int,int);
void getPels(int ,int ,yCbCr [256] ,int ,picture*);
void motionVectorFor(pictureHeader ,int ,int ,int ,int ,int &,int &);
void skipMacroBlock(pictureHeader ,int );
void motionVectorBack(pictureHeader ,int ,int ,int ,int ,int &,int &);

void macroblockToPicture(int [6][64],int ,yCbCr [256]);
bool videoIsEnd();
sequenceHeader getSeqHeader();
void pictureToBuffer(picture *);

#endif
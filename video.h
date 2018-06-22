#ifndef VIEDO_H
#define VIEDO_H
#include"dataStream.h"
#include"code.h"
#include"vlc.h"


void getExtensionData(dataStream &);
void getUserData(dataStream &);



/*Get sequence header and quantization matrix. */
int get_sequence_header(dataStream &,sequenceHeader *);
int group_of_pictures(dataStream &,sequenceHeader,gopHeader *,int &,picture &);
int get_picture(dataStream &,sequenceHeader,pictureHeader *);
int get_slice(dataStream &,sliceHeader *,pictureHeader *);
int get_macroblock(dataStream &,pictureHeader*,sliceHeader*,int &,int &,int [3]);
int get_block(dataStream &,bool [6],int ,macroblock_type,pictureHeader *,int [6][64]);
int video_sequence(dataStream &,int,picture &);
void macroblockToPicture(int [6][64],int );
void intraMacroBlockDecode(int [6][64],int ,int ,int & ,int [6]);
void initialCur(int,int);
yCbCr getPictureData(picture *,int,int);

#endif
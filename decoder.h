#ifndef DECODER_H
#define DECODER_H
#include "code.h" 
#include <opencv2/opencv.hpp>



using namespace cv;
void zigZag(int [6][64]);

void deQuantize(int [6][64],int [64],unsigned char );

void IDCT(int [6][64]);

void levelShift(int [6][64]);
void zigZag(int [64]);

void deQuantize(int [64],int [64],unsigned char );

void IDCT(int [64]);

void levelShift(int [64]);

void YCbCrtoRGB(picture &,Mat *);



#endif
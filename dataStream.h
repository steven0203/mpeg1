#ifndef DATA_STREAM_H
#define DATA_STREAM_H
#include<cstdio>



class dataStream
{
    public:
        dataStream(FILE *);
        unsigned short getFirstBit();
        unsigned int getBits(unsigned char);
        bool isEnd();
        unsigned char currentData;
        unsigned char currentBit;
        unsigned int next_start_code;
        void getNextStartCode();
        bool bytealigned();
        void seekBack(int);
    private:
        FILE *path;
        bool end;
};






#endif





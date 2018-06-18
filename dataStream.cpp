#include"dataStream.h"
#include<cstdio>
#include<cstdlib>



dataStream::dataStream(FILE *path)
{
    this->path=path;
    fread(&(this->currentData),1,1,path);
    currentBit=0;
    end=false;
}

unsigned short dataStream::getFirstBit()
{
    unsigned short result;
    result=(currentData>>(7-currentBit))&1;
    currentBit+=1;
    if(currentBit==8)
    {
        currentBit=0;
        fread(&currentData,1,1,path);
    }
    return result;
}

unsigned int dataStream::getBits(unsigned char num)
{
    unsigned int result=0,bit=0;
    for(int i=0;i<num;++i)
    {   
        bit=(unsigned int)getFirstBit();

        result=result^(bit<<31-i);

    }
    return result;
}



bool dataStream::isEnd()
{
    return end;
}

void dataStream::getNextStartCode()
{
    if(!bytealigned())
    {
        currentBit=0;
        fread(&currentData,1,1,path);
    }
    unsigned int bits=getBits(24);
    while(bits!=256)
    {
        bits=(bits<<8)^(getBits(8)>>16);
    }
    next_start_code=bits^(getBits(8)>>24);
}

bool dataStream::bytealigned()
{
    if(currentBit==0)
        return 1;
    return 0; 
}

void dataStream::seekBack(int bit)
{
    for(;bit>0;bit--)
    {
        if(currentBit==0)
        {
            fseek(path,-1,SEEK_CUR);    
            currentBit=8;
        }currentBit-=1;
    }
    fseek(path,-1,SEEK_CUR);    
    fread(&currentData,1,1,path);


}
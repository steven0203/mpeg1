#ifndef VLC_H
#define VLC_H
#include<map>
#include"dataStream.h"
using namespace std;
#include "code.h"


typedef struct
{
    bool quant;
    bool motion_forward;
    bool motion_backward;
    bool pattern;
    bool intra;
}macroblock_type;


class vlc
{
    public:
        vlc();
        unsigned char getMacroAddress(dataStream &);
        macroblock_type getMacroType(dataStream &,unsigned char);
        int getMotionCode(dataStream &);
        unsigned short getCBP(dataStream &);
        unsigned char getDctDcSizeLum(dataStream &);
        unsigned char getDctDcSizeChr(dataStream &);
        dct_coeff getDctCoeff(dataStream &,unsigned short );
    private:
        map<unsigned short,unsigned char>macroblockAddressTable;
        map<unsigned short,unsigned char>macroblockAddressLength;
        map<unsigned char,macroblock_type>macroblockTypeTableI;
        map<unsigned char,macroblock_type>macroblockTypeTableP;
        map<unsigned short,unsigned char>macroblockTypeLengthP;
        map<unsigned char,macroblock_type>macroblockTypeTableB;
        map<unsigned short,unsigned char>macroblockTypeLengthB;
        macroblock_type macroblockTypeD;
        map<unsigned short,int>motionVectorTable;
        map<unsigned short,unsigned char>motionVectorLength;
        map<unsigned short,unsigned char>codedBlockTable;
        map<unsigned short,unsigned char>codedBlockLength;
        map<unsigned char,unsigned char>dctDcSizeLumTable;
        map<unsigned char,unsigned char>dctDcSizeChrTable;
        map<unsigned short,dct_coeff>dctCoeffTable;
        map<unsigned short,unsigned char>dctCoeffLength;

        dct_coeff getDctCoeffFixed(dataStream &);
};



#endif
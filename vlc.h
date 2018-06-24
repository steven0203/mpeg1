#ifndef VLC_H
#define VLC_H
#include<map>
#include"dataStream.h"
using namespace std;
#include "code.h"



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
        macroblock_type macroblockTypeD;
        map<unsigned char,unsigned char>dctDcSizeLumTable;
        map<unsigned char,unsigned char>dctDcSizeChrTable;
        dct_coeff getDctCoeffFixed(dataStream &);
};



#endif
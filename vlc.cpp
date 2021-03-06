#include<map>
#include"vlc.h"
#include"code.h"


using namespace std;
unsigned short macroblock_address_increment_code[35]={
    0b1<<15,0b011<<13,0b010<<13,0b0011<<12,0b0010<<12,0b00011<<11,0b00010<<11,
    0b0000111<<9,0b0000110<<9,0b00001011<<8,0b00001010<<8,0b00001001<<8,
    0b00001000<<8,0b00000111<<8,0b00000110<<8,0b0000010111<<6,0b0000010110<<6,
    0b0000010101<<6,0b0000010100<<6,0b0000010011<<6,0b0000010010<<6,
    0b00000100011<<5,0b00000100010<<5,0b00000100001<<5,0b00000100000<<5,
    0b00000011111<<5,0b00000011110<<5,0b00000011101<<5,0b00000011100<<5,
    0b00000011011<<5,0b00000011010<<5,0b00000011001<<5,
    0b00000011000<<5,0b00000001111<<5,0b00000001000<<5
};

unsigned char address_code_length[35]={
    1,3,3,4,4,5,5,7,7,8,8,8,8,8,8,10,10,10,10,10,10,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11
};

unsigned short motion_vector_code[33]={
    0b00000011001<<5, 
    0b00000011011<<5, 
    0b00000011101<<5, 
    0b00000011111<<5, 
    0b00000100001<<5, 
    0b00000100011<<5, 
    0b0000010011<<6, 
    0b0000010101<<6, 
    0b0000010111<<6,
    0b00000111<<8, 
    0b00001001<<8,
    0b00001011<<8,
    0b0000111<<9,
    0b00011<<11,
    0b0011<<12,
    0b011<<13,
    0b1<<15,
    0b010<<13,
    0b0010<<12,
    0b00010<<11,
    0b0000110<<9,
    0b00001010<<8,
    0b00001000<<8,
    0b00000110<<8,
    0b0000010110<<6,
    0b0000010100<<6,
    0b0000010010<<6,
    0b00000100010<<5,
    0b00000100000<<5,
    0b00000011110<<5,
    0b00000011100<<5, 
    0b00000011010<<5, 
    0b00000011000<<5
};

unsigned char motion_vector_length[33]={
    11,11,11,11,11,11,10,10,10,8,8,8,7,5,4,3,1,
    3,4,5,7,8,8,8,10,10,10,11,11,11,11,11,11
};

unsigned char cbp[63]=
{
    60,4,8,16,32,12,48,20,40,28,44,52,56,1,61,2,62,24,36,3,
    63,5,9,17,33,6,10,18,34,7,11,19,35,13,49,21,41,14,50,22,
    42,15,51,23,43,25,37,26,38,29,45,53,57,30,46,54,58,31,47,55,59,27,39
};

unsigned short coded_block_pattern_code[63]={
    0b111<<13,
    0b1101<<12,
    0b1100<<12,
    0b1011<<12,
    0b1010<<12,
    0b10011<<11,
    0b10010<<11,
    0b10001<<11,
    0b10000<<11,
    0b01111<<11,
    0b01110<<11,
    0b01101<<11,
    0b01100<<11,
    0b01011<<11,
    0b01010<<11,
    0b01001<<11,
    0b01000<<11,
    0b001111<<10,
    0b001110<<10,
    0b001101<<10,
    0b001100<<10,
    0b0010111<<9,
    0b0010110<<9,
    0b0010101<<9,
    0b0010100<<9,
    0b0010011<<9,
    0b0010010<<9,
    0b0010001<<9,
    0b0010000<<9,
    0b00011111<<8,
    0b00011110<<8,
    0b00011101<<8,
    0b00011100<<8,
    0b00011011<<8,
    0b00011010<<8,
    0b00011001<<8,
    0b00011000<<8,
    0b00010111<<8,
    0b00010110<<8,
    0b00010101<<8,
    0b00010100<<8,
    0b00010011<<8,
    0b00010010<<8,
    0b00010001<<8,
    0b00010000<<8,
    0b00001111<<8,
    0b00001110<<8,
    0b00001101<<8,
    0b00001100<<8,
    0b00001011<<8,
    0b00001010<<8,
    0b00001001<<8,
    0b00001000<<8,
    0b00000111<<8,
    0b00000110<<8,
    0b00000101<<8,
    0b00000100<<8,
    0b000000111<<7,
    0b000000110<<7,
    0b000000101<<7,
    0b000000100<<7,
    0b000000011<<7,
    0b000000010<<7,


};

unsigned char cbp_code_length[63]={
    3,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,
    7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9
};

unsigned char macroblock_type_code_I[2]={
    0b1<<7,0b01<<6
};






macroblock_type  typeI[2]={
    {0,0,0,0,1},
    {1,0,0,0,1}
};

macroblock_type typeP[7]={
    {0,1,0,1,0},
    {0,0,0,1,0},
    {0,1,0,0,0},
    {0,0,0,0,1},
    {1,1,0,1,0},
    {1,0,0,1,0},
    {1,0,0,0,1}
};

macroblock_type typeB[11]={
    {0,1,1,0,0},
    {0,1,1,1,0},
    {0,0,1,0,0},
    {0,0,1,1,0},
    {0,1,0,0,0},
    {0,1,0,1,0},
    {0,0,0,0,1},
    {1,1,1,1,0},
    {1,1,0,1,0},
    {1,0,1,1,0},
    {1,0,0,0,1}
};

unsigned char dct_dc_size_luminace_code[9]={
    0b100,0b00,0b01,0b101,0b110,0b1110,
    0b11110,0b111110,0b1111110
};

unsigned char dct_dc_size_chrominance_code[9]={
    0b00,0b01,0b10,0b110,0b1110,0b11110,
    0b111110,0b1111110,0b11111110
};


unsigned short dct_coeff_code[111]={
0b011<<13
,0b0100<<12
,0b0101<<12
,0b00101<<11
,0b00111<<11
,0b00110<<11
,0b000110<<10
,0b000111<<10
,0b000101<<10
,0b000100<<10
,0b0000110<<9
,0b0000100<<9
,0b0000111<<9
,0b0000101<<9
,0b000001<<10
,0b00100110<<8
,0b00100001<<8
,0b00100101<<8
,0b00100100<<8
,0b00100111<<8
,0b00100011<<8
,0b00100010<<8
,0b00100000<<8
,0b0000001010<<6
,0b0000001100<<6
,0b0000001011<<6
,0b0000001111<<6
,0b0000001001<<6
,0b0000001110<<6
,0b0000001101<<6
,0b0000001000<<6
,0b000000011101<<4
,0b000000011000<<4
,0b000000010011<<4
,0b000000010000<<4
,0b000000011011<<4
,0b000000010100<<4
,0b000000011100<<4
,0b000000010010<<4
,0b000000011110<<4
,0b000000010101<<4
,0b000000010001<<4
,0b000000011111<<4
,0b000000011010<<4
,0b000000011001<<4
,0b000000010111<<4
,0b000000010110<<4
,0b0000000011010<<3
,0b0000000011001<<3
,0b0000000011000<<3
,0b0000000010111<<3
,0b0000000010110<<3
,0b0000000010101<<3
,0b0000000010100<<3
,0b0000000010011<<3
,0b0000000010010<<3
,0b0000000010001<<3
,0b0000000010000<<3
,0b0000000011111<<3
,0b0000000011110<<3
,0b0000000011101<<3
,0b0000000011100<<3
,0b0000000011011<<3
,0b00000000011111<<2
,0b00000000011110<<2
,0b00000000011101<<2
,0b00000000011100<<2
,0b00000000011011<<2
,0b00000000011010<<2
,0b00000000011001<<2
,0b00000000011000<<2
,0b00000000010111<<2
,0b00000000010110<<2
,0b00000000010101<<2
,0b00000000010100<<2
,0b00000000010011<<2
,0b00000000010010<<2
,0b00000000010001<<2
,0b00000000010000<<2
,0b000000000011000<<1
,0b000000000010111<<1
,0b000000000010110<<1
,0b000000000010101<<1
,0b000000000010100<<1
,0b000000000010011<<1
,0b000000000010010<<1
,0b000000000010001<<1
,0b000000000010000<<1
,0b000000000011111<<1
,0b000000000011110<<1
,0b000000000011101<<1
,0b000000000011100<<1
,0b000000000011011<<1
,0b000000000011010<<1
,0b000000000011001<<1
,0b0000000000010011
,0b0000000000010010
,0b0000000000010001
,0b0000000000010000
,0b0000000000010100
,0b0000000000011010
,0b0000000000011001
,0b0000000000011000
,0b0000000000010111
,0b0000000000010110
,0b0000000000010101
,0b0000000000011111
,0b0000000000011110
,0b0000000000011101
,0b0000000000011100
,0b0000000000011011

};

dct_coeff dctCoeff[111]={
    {1,1},{0,2},{2,1},{0,3},{3,1},{4,1},{1,2},{5,1},{6,1},{7,1},
{0,4},{2,2},{8,1},{9,1},{escape_run,0},{0,5},{0,6},{1,3},{3,2},{10,1},
{11,1},{12,1},{13,1},{0,7},{1,4},{2,3},{4,2},{5,2},{14,1},{15,1},
{16,1},{0,8},{0,9},{0,10},{0,11},{1,5},{2,4},{3,3},{4,3},{6,2},
{7,2},{8,2},{17,1},{18,1},{19,1},{20,1},{21,1},{0,12},{0,13},{0,14},
{0,15},{1,6},{1,7},{2,5},{3,4},{5,3},{9,2},{10,2},{22,1},{23,1},
{24,1},{25,1},{26,1},{0,16},{0,17},{0,18},{0,19},{0,20},{0,21},
{0,22},{0,23},{0,24},{0,25},{0,26},{0,27},{0,28},{0,29},{0,30},{0,31},
{0,32},{0,33},{0,34},{0,35},{0,36},{0,37},{0,38},{0,39},{0,40},{1,8},
{1,9},{1,10},{1,11},{1,12},{1,13},{1,14},{1,15},{1,16},{1,17},{1,18},
{6,3},{11,2},{12,2},{13,2},{14,2},{15,2},{16,2},{27,1},{28,1},{29,1},
{30,1},{31,1}
};

unsigned char dct_coeff_length[111]={
    3,4,4,5,5,5,6,6,6,6,7,7,7,7,6,8,8,8,8,8,8,8,8,
    10,10,10,10,10,10,10,10,12,12,12,12,12,12,12,12,
    12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,
    14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16
};

int macroblockAddressIncre[35];


int motionVectorData[33];

vlc::vlc()
{
    unsigned short code;
    unsigned char shortCode;
    for(int i=0;i<35;++i)
    {
        macroblockAddressIncre[i]=i+1;
    }

    macroblockTypeD={0,0,0,0,1};
    for(int i=0,data=-16;i<33;++i,++data)
        motionVectorData[i]=data;
    for(int i=0;i<9;++i)
    {
        shortCode=dct_dc_size_luminace_code[i];
        dctDcSizeLumTable[shortCode]=i;
        shortCode=dct_dc_size_chrominance_code[i];
        dctDcSizeChrTable[shortCode]=i;
    }
}


unsigned char vlc::getMacroAddress(dataStream &stream)
{
    unsigned short code=0;
    for(int i=0;i<11;++i)
    {
        code=(code)|(stream.getFirstBit()<<(15-i));

        switch(i)
        {
            case(0):
                if(code==macroblock_address_increment_code[0])
                    return macroblockAddressIncre[0];
                break;
            case(1):
                break;
            case(2):
                for(int j=1;j<3;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(3):
                for(int j=3;j<5;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(4):
                for(int j=5;j<7;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(5):
                break;
            case(6):
                for(int j=7;j<9;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(7):
                for(int j=9;j<15;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(8):
                break;
            case(9):
                for(int j=15;j<21;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            case(10):
                for(int j=21;j<35;++j)
                    if(code==macroblock_address_increment_code[j])
                        return macroblockAddressIncre[j];
                break;
            default:
                break;
        };

    }
    return 0;
}





macroblock_type vlc::getMacroType(dataStream &stream,unsigned char type)
{
    unsigned char code=0;
    if(type==I_picture_type)
    {
        for(int i=0;i<6;++i)
        {
            code=(code)|(stream.getFirstBit()<<(7-i));
            switch(code)
            {
                case(0b10000000):
                    return typeI[0];
                case(0b01000000):
                    return typeI[1];
                default:
                    break;
            }
        }
    }

    if(type==P_picture_type)
    {
        for(int i=0;i<6;++i)
        {
            code=(code)|(stream.getFirstBit()<<(7-i));
            if(i==3)
                continue;
            switch(code)
            {
                case(0b10000000):
                    return typeP[0];
                case(0b01000000):
                    return typeP[1];
                case(0b00100000):
                    return typeP[2];
                case(0b00011000):
                    return typeP[3];
                case(0b00010000):
                    return typeP[4];
                case(0b00001000):
                    return typeP[5];
                case(0b00000100):
                    return typeP[6];
                default:
                    break;
            }
        }
    }

    if(type==B_picture_type)
    {
        for(int i=0;i<6;++i)
        {
            code=(code)|(stream.getFirstBit()<<(7-i));
            //First check the code length i,next check the code .
            switch(i)
            {
                case(0):
                    break;
                case(1):
                    switch(code)
                    {
                        case(0b10000000):
                            return typeB[0];
                        case(0b11000000):
                            return typeB[1];
                    };
                    break;
                case(2):
                    switch(code)
                    {
                        case(0b01000000):
                            return typeB[2];
                        case(0b1100000):
                            return typeB[3];
                    };
                    break;
                case(3):
                    switch(code)
                    {
                        case(0b00100000):
                            return typeB[4];
                        case(0b00110000):
                            return typeB[5];

                    };
                    break;
                case(4):
                    switch(code)
                    {
                        case(0b00011000):
                            return typeB[6];
                        case(0b00010000):
                            return typeB[7];
                    };
                    break;
                case(5):
                    switch(code)
                    {
                        case(0b00001100):
                            return typeB[8];
                        case(0b00001000):
                            return typeB[9];
                        case(0b00000100):
                            return typeB[10];
                    };
                    break;
                default:
                    break;
            };
        }
    }




    if(type==D_picture_type&&stream.getFirstBit())
            return macroblockTypeD;

}



int vlc::getMotionCode(dataStream &stream)
{
    unsigned short code=0;
    for(int i=0;i<11;++i)
    {
        code=(code)|(stream.getFirstBit()<<(15-i));
        //First check the code length i,next check the code .
        switch(i)
        {
            case(0):
                if(code==motion_vector_code[16])
                    return 0;
                break;
            case(1):
                break;
            case(2):
                if(code==motion_vector_code[15])
                    return motionVectorData[15];
                if(code==motion_vector_code[17])
                    return motionVectorData[17];
                break;
            case(3):
                if(code==motion_vector_code[14])
                    return motionVectorData[14];
                if(code==motion_vector_code[18])
                    return motionVectorData[18];
                break;
            case(4):
                if(code==motion_vector_code[13])
                    return motionVectorData[13];
                if(code==motion_vector_code[19])
                    return motionVectorData[19];
                break;
            case(5):
                break;
            case(6):
                if(code==motion_vector_code[12])
                    return motionVectorData[12];
                if(code==motion_vector_code[20])
                    return motionVectorData[20];
                break;
            case(7):
                for(int j=9;j<12;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                for(int j=21;j<24;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                break;
            case(8):
                break;
            case(9):
                for(int j=6;j<9;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                for(int j=24;j<27;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                break;
            case(10):
                for(int j=0;j<6;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                for(int j=27;j<33;j++)
                {
                    if(code==motion_vector_code[j])
                        return motionVectorData[j];
                }
                break;
            default:
                break;
        };
    }
}


unsigned short vlc::getCBP(dataStream &stream)
{
    unsigned short code=0;
    for(int i=0;i<9;++i)
    {
        code=(code)|(stream.getFirstBit()<<(15-i));
        //First check the code length i,next check the code .
        switch(i)
        {
            case(0):
            case(1):
                break;
            case(2):
                if(code==coded_block_pattern_code[0])
                    return cbp[0];
                break;
            case(3):
                for(int j=1;j<5;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            case(4):
                for(int j=5;j<17;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            case(5):
                for(int j=17;j<21;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            case(6):
                for(int j=21;j<29;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            case(7):
                for(int j=29;j<57;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            
            case(8):
                for(int j=57;j<63;++j)
                {
                    if(code==coded_block_pattern_code[j])
                        return cbp[j];
                }
                break;
            
            default:
                break;
        };



    }
}


unsigned char vlc::getDctDcSizeLum(dataStream &stream)
{
    unsigned char code=0;
    code=stream.getFirstBit();
    for(int i=1;i<7;++i)
    {
        code=(code<<1)|stream.getFirstBit();
        if(dctDcSizeLumTable.count(code)>0)
        {
            return dctDcSizeLumTable[code];
        }
    }
}
unsigned char vlc::getDctDcSizeChr(dataStream &stream)
{
    unsigned char code=0;
    code=stream.getFirstBit();
    for(int i=1;i<8;++i)
    {
        code=(code<<1)|stream.getFirstBit();
        if(dctDcSizeChrTable.count(code)>0)
            return dctDcSizeChrTable[code];
    }
}

dct_coeff vlc::getDctCoeff(dataStream &stream,unsigned short tag)
{
    unsigned short code=0;
    dct_coeff result;
    bool negative;
    result.run=0;
    result.level=1;
    code=stream.getFirstBit();
    if(tag==dct_coeff_first&&code==dct_coeff_first)
    {
        negative=stream.getFirstBit();
        if(negative)
            result.level=result.level*-1;
        return result;
    }
    code=(code<<1)|stream.getFirstBit();
    if(tag==dct_coeff_next&&code==dct_coeff_next)
    {
        negative=stream.getFirstBit();
        if(negative)
            result.level=result.level*(-1);
        return result;
    }
    if(code==end_of_block)
    {
        result.run=end_of_block_run;
        return result;
    }


    code=code<<14;
    for(int i=0;i<14;++i)
    {
        code=(code)|(stream.getFirstBit()<<(13-i));
        //First check the code length i,next check the code .

        switch(i)
        {
            case(0):
                if(code==dct_coeff_code[0])
                {
                    result=dctCoeff[0];
                    negative=stream.getFirstBit();
                    if(negative)
                        result.level=result.level*(-1);
                    return result;
                }
                break;
            case(1):
                for(int j=1;j<3;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(2):
                for(int j=3;j<6;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(3):
                for(int j=6;j<10;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                //When the run of dct_coeff is escape,it need to do fixed length code decoding.  
                if(code==dct_coeff_code[14])
                {    
                    result=getDctCoeffFixed(stream);
                    return result;
                }
                break;
            case(4):
                for(int j=10;j<14;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(5):
                for(int j=15;j<23;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(6):
                break;
            case(7):
                for(int j=23;j<31;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(8):
                break;
            case(9):
                for(int j=31;j<47;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(10):
                for(int j=47;j<63;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(11):
                for(int j=63;j<79;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(12):
                for(int j=79;j<95;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            case(13):
                for(int j=95;j<111;++j)
                    if(code==dct_coeff_code[j])
                    {
                        result=dctCoeff[j];
                        negative=stream.getFirstBit();
                        if(negative)
                            result.level=result.level*(-1);
                        return result;
                    }
                break;
            default:
                break;
        };
    }       

}



dct_coeff vlc::getDctCoeffFixed(dataStream &stream)
{
    dct_coeff result;
    result.run=stream.getBits(6)>>(32-6);
    result.level=stream.getBits(8)>>(32-8);
    if(result.level==0)
        result.level=stream.getBits(8)>>(32-8);
    else if(result.level==128)
    {
        result.level=stream.getBits(8)>>(32-8);
        result.level=result.level-256;
    }
    else if(result.level>128)
        result.level=result.level-256;
    return result;
    
}

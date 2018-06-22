#include"decoder.h"


int sign(int input)
{
    if(input>0)
        return 1;
    if(input==0)
        return 0;
    return -1;
}



static double cosData[8][8]={
    1.000000,0.980785,0.923880,0.831470,0.707107,0.555570,0.382683,0.195090,
    1.000000,0.831470,0.382683,-0.195090,-0.707107,-0.980785,-0.923880,-0.555570,
    1.000000,0.555570,-0.382683,-0.980785,-0.707107,0.195090,0.923880,0.831470,
    1.000000,0.195090,-0.923880,-0.555570,0.707107,0.831470,-0.382683,-0.980785,
    1.000000,-0.195090,-0.923880,0.555570,0.707107,-0.831470,-0.382684,0.980785,
    1.000000,-0.555570,-0.382683,0.980785,-0.707107,-0.195090,0.923880,-0.831470,
    1.000000,-0.831470,0.382683,0.195090,-0.707107,0.980785,-0.923879,0.555570,
    1.000000,-0.980785,0.923880,-0.831470,0.707107,-0.555570,0.382683,-0.195090
};



void deQuantize(int data[64],int quantizer[64],unsigned char quantizer_scale)
{
    for(int i=1;i<64;++i)
    {
        data[i]=(2*data[i]*quantizer[i]*quantizer_scale)/16;
        if((data[i]&1)==0)
            data[i]=data[i]-sign(data[i]);
        if(data[i]>2047)
            data[i]=2047;
        if(data[i]<-2048)
            data[i]=-2048;
    }
    data[0]=data[0]*8;
}

void deQuantize(int data[6][64],int quantizer[64],unsigned char quantizer_scale)
{
    for(int index=0;index<6;++index)
    {
        for(int i=1;i<64;++i)
        {
            data[index][i]=(2*data[index][i]*quantizer[i]*quantizer_scale)/16;
            if((data[index][i]&1)==0)
                data[index][i]=data[index][i]-sign(data[index][i]);
            if(data[index][i]>2047)
                data[index][i]=2047;
            if(data[index][i]<-2048)
                data[index][i]=-2048;
        }
        data[index][0]=data[index][0]*8;
    }

}

void zigZag(int block[64])
{
    int tmp[64];
    for(int i=0;i<64;++i)
        tmp[i]=block[i];
    for (int i =0, n = 0; i < 8 * 2; i++)
		for (int j = (i < 8) ? 0 : i-8+1; j <= i && j < 8; j++)
			block[(i&1)? j*(8-1)+i : (i-j)*8+j ] = tmp[n++];
}

void zigZag(int block[6][64])
{
    int tmp[64];
    for(int index=0;index<6;++index)
    {
        for(int i=0;i<64;++i)
        tmp[i]=block[index][i];
        for (int i =0, n = 0; i < 8 * 2; i++)
		    for (int j = (i < 8) ? 0 : i-8+1; j <= i && j < 8; j++)
			    block[index][(i&1)? j*(8-1)+i : (i-j)*8+j ] = tmp[n++];
    }

}




void IDCT(int block[64])
{
    int originBlock[64],N=8;
    for(int i=0;i<64;++i)
        originBlock[i]=block[i];
    double coefficient1,coefficient2;
    for(int i=0;i<N;i++)
    {
        for(int j=0;j<N;j++)
        {
            double tmp=0.0;
            for(int p=0;p<N;p++)
                for(int q=0;q<N;q++)
                {
                    if(p==0) coefficient1=0.353553391;
                    else coefficient1=0.5;
                    if(q==0) coefficient2=0.353553391;
                    else coefficient2=0.5;
                    tmp+=coefficient1*coefficient2*originBlock[p*N+q]*cosData[i][p]*cosData[j][q];
                }
            block[i*N+j]=(int)tmp;
        } 
    }

}
void IDCT(int block[6][64])
{
    int originBlock[64],N=8;
    for(int index=0;index<6;++index)
    {
        for(int i=0;i<64;++i)
            originBlock[i]=block[index][i];
        double coefficient1,coefficient2;
        for(int i=0;i<N;i++)
        {
            for(int j=0;j<N;j++)
            {
                double tmp=0.0;
                for(int p=0;p<N;p++)
                    for(int q=0;q<N;q++)
                    {
                        if(p==0) coefficient1=0.353553391;
                        else coefficient1=0.5;
                        if(q==0) coefficient2=0.353553391;
                        else coefficient2=0.5;
                        tmp+=coefficient1*coefficient2*originBlock[p*N+q]*cosData[i][p]*cosData[j][q];
                    }
                block[index][i*N+j]=(int)tmp;
            } 
        }
    }

}


void levelShift(int block[64])
{
    for(int i=0;i<64;++i)
    {
        //block[i]+=128;
        if(block[i]<0)
            block[i]=0;
        if(block[i]>255)
            block[i]=255;
    }
}

void levelShift(int block[6][64])
{
    for(int index=0;index<6;++index)
    {
        for(int i=0;i<64;++i)
        {
            //block[i]+=128;
            if(block[index][i]<0)
                block[index][i]=0;
            if(block[index][i]>255)
                block[index][i]=255;
        }
    }

}



void YCbCrtoRGB(picture &input,Mat *result)
{
    double r,g,b,y,Cb,Cr;
    for(int i=0;i<input.height;++i)
        for(int j=0;j<input.width;++j)
        {
            y=(double)input.data[i*input.width+j].y;
            Cb=(double)input.data[i*input.width+j].cb;
            Cr=(double)input.data[i*input.width+j].cr;
            r=y+1.402*(Cr-128.0);
            g=y-0.34414*(Cb-128.0)-0.71414*(Cr-128.0);
            b=y+1.774*(Cb-128.0);
            r=r>255?255:r;
            g=g>255?255:g;
            b=b>255?255:b;
            r=r<0?0:r;
            g=g<0?0:g;
            b=b<0?0:b;

            result->at<Vec3b>(i,j)[0]=b;
            result->at<Vec3b>(i,j)[1]=g;
            result->at<Vec3b>(i,j)[2]=r;

        }
}

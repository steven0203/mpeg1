#include<cmath>
#include<cstdio>

#define PI 3.14159262

int main(){
    double cosData[8][8];
    int N=8;
    for(int i=0;i<8;++i)
    {
        for(int j=0;j<8;++j)
        {
            cosData[i][j]=cos((2*i+1)*PI*j/(2*N));
            printf("%f ",cosData[i][j]);
        }
    }
}
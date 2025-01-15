#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bmp_img_processor.h"
#define PI 3.14159265359
IMG *exp_kernel(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[1][1]=1;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=6;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *exp_pool(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=3;
   IMG *r=MaxPooling(img,&kernel);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
int main()
{
    char *file="images\\doom.bmp";
    IMG *img=getImage(file);
    renderAsciiArt_terminal(exp_pool(img));
    printf("Success");
    return 0;
}

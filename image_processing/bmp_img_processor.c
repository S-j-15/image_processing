#include <stdio.h>
#include <stdlib.h>
#include "bmp_img_processor.h"
int errMsg(int err,char *default_msg)
{
  //err==1 NOT BMP //err==2 INVALID BITS PER PIXEL //err==3 invalid BMP Header //err==4 invalid DIB header //err==5 Fail to ofen file
  //default Crash
  if(err)
  {
    switch(err)
    {
      case 1:
      printf("STATUS CODE: %d -> NOT A BMP FILE...\n",err);
      break;

      case 2:
      printf("STATUS CODE: %d -> BITS PER PX IS NOT 24 (8+8+8)...\n",err); 
      break;

      case 3:
      printf("STATUS CODE: %d -> INVALID BMP HEADER...\n",err);
      break;

      case 4:
      printf("STATUS CODE: %d -> INVALID DIB HEADER...\n",err);
      break;

      case 5:
      printf("STATUS CODE: %d -> FAIL TO OPEN FILE...\n",err);
      break;

      default:
      printf("STATUS CODE: %d -> %s...\n",err,default_msg);
      break;
    }
    return 1;
  }
  return 0;
}
int isBMP(char *file)
{
    FILE *fp=fopen(file,"rb");
    char ck[2];
    fread(ck,1,2,fp);
    fclose(fp);
    if(ck[0]=='B' && ck[1]=='M')return 1;
    return 0;
}
BMP_HEADER* getBMPHeader(char *file)
{
    FILE *fp=fopen(file,"rb");
    if(!fp){errMsg(5," ");return NULL;}
    if (!isBMP(file)){errMsg(1," ");return NULL;}
    BMP_HEADER *h_buff=calloc(1,sizeof(BMP_HEADER));
    if(!h_buff){errMsg(9,"MEM ALLOC CRASH HEADER");return NULL;}
    fread(h_buff->bm,1,2,fp);
    fread(&h_buff->size,1,4,fp);
    fread(h_buff->garb,1,4,fp);
    fread(&h_buff->addr,1,4,fp);
    fclose(fp);
    return h_buff;
}

DIB_HEADER* getDIBHeader(char *file)
{
  FILE *fp=fopen(file,"rb");
  if(!fp){errMsg(5," ");return NULL;}
  if (!isBMP(file))return NULL;
  fseek(fp,14,SEEK_SET);
  DIB_HEADER *h_buff=calloc(1,sizeof(DIB_HEADER));
  if(!h_buff){errMsg(9,"MEM ALLOC CRASH HEADER");return NULL;}
  fread(&h_buff->h_size,1,4,fp);
  fread(&h_buff->width_px,1,4,fp);
  fread(&h_buff->height_px,1,4,fp);
  fread(&h_buff->colour_plains,1,2,fp);
  fread(&h_buff->no_of_bits_per_px,1,2,fp);
  fread(&h_buff->compression_method,1,4,fp);
  fread(&h_buff->img_size,1,4,fp);
  fread(&h_buff->horizontal_resolution,1,4,fp);
  fread(&h_buff->vertical_resolution,1,4,fp);
  fread(&h_buff->no_of_colors,1,4,fp);
  fread(&h_buff->no_of_colors,1,4,fp);
  fclose(fp);
  return h_buff;
}
int isValidFormat(char *file)
{
    FILE *fp=fopen(file,"rb");
    if(!fp){errMsg(5," ");return 0;}
    if(!isBMP(file)){fclose(fp);return 0;}
    DIB_HEADER *h=getDIBHeader(file);
    if(h->compression_method!=0 || h->no_of_bits_per_px!=24){
      free(h);
      fclose(fp);
      return 0;  
    }
    free(h);
    fclose(fp);
    return 1;
}
IMG *getImage(char *file)
{
     FILE *fp=fopen(file,"rb");
     if(!fp){errMsg(5," ");return NULL;}
     BMP_HEADER *bmph=getBMPHeader(file);
     if(!bmph){errMsg(3," ");return NULL;}
     DIB_HEADER *dibh=getDIBHeader(file);
     if(!dibh){errMsg(4," ");return NULL;}
     fseek(fp,bmph->addr,SEEK_SET);
     IMG *img=calloc(1,sizeof(IMG));
     if(!img){errMsg(11,"MEM ALLOC CRASH IMG");return NULL;}
    unsigned int height=dibh->height_px;
    unsigned int width=dibh->width_px;
    img->height=dibh->height_px;
    img->width=dibh->width_px;
    img->rgb=calloc(height,sizeof(RGB*));
    if(!(img->rgb)){errMsg(10,"MEM ALLOC CRASH RGB");return NULL;}
    //take care of padding here... 
    unsigned int pad=(int)(((dibh->no_of_bits_per_px*dibh->width_px)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    for(int i=height-1;i>=0;i--)
    {
      img->rgb[i]=calloc(rgb_px,sizeof(RGB));
      if(!(img->rgb[i])){errMsg(12,"MEM ALLOC CRASH RGB[i]");return NULL;}
      fread(img->rgb[i],1,pad,fp);
    }
    fclose(fp);
    free(bmph);
    free(dibh);
    return img;
}
IMG* greyScale(IMG *img)
{
  unsigned int bits_per_px=24;
  IMG* gimg=calloc(1,sizeof(IMG));
  if(!gimg){errMsg(11,"MEM ALLOC CRASH IMG");return NULL;}
  gimg->height=img->height;
  gimg->width=img->width;
  gimg->rgb=calloc(gimg->height,sizeof(RGB*));
  unsigned int pad=(int)(((bits_per_px*gimg->width)+31)/32)*4;
  unsigned int rgb_px=pad/sizeof(RGB)+1;
  for(int i=0;i<gimg->height;i++)
  {
    gimg->rgb[i]=calloc(rgb_px,sizeof(RGB));
     for(int j=0;j<gimg->width;j++)
     {
       gimg->rgb[i][j].r=gimg->rgb[i][j].g=gimg->rgb[i][j].b=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b));
     }
  }
  return gimg;
}
IMG* bwImg(IMG *img)
{
  unsigned int bits_per_px=24;
  IMG* gimg=calloc(1,sizeof(IMG));
  if(!img){errMsg(11,"MEM ALLOC CRASH IMG");return NULL;}
  gimg->height=img->height;
  gimg->width=img->width;
  gimg->rgb=calloc(gimg->height,sizeof(RGB*));
  unsigned int pad=(int)(((bits_per_px*gimg->width)+31)/32)*4;
  unsigned int rgb_px=pad/sizeof(RGB)+1;
  for(int i=0;i<gimg->height;i++)
  {
    gimg->rgb[i]=calloc(rgb_px,sizeof(RGB));
     for(int j=0;j<gimg->width;j++)
     {
       if((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b)>=255/2)gimg->rgb[i][j].r=gimg->rgb[i][j].g=gimg->rgb[i][j].b=255;
       else gimg->rgb[i][j].r=gimg->rgb[i][j].g=gimg->rgb[i][j].b=0;
     }
  }
  return gimg;
}

void renderAsciiArt(IMG *img)
{
  FILE *fp=fopen("ascii.txt","w"); 
  char *apx="@#N0a-. ";
  int size=8;
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      fprintf(fp,"%c",apx[7-ind]);
    }
    fprintf(fp,"\n");
  }
}
void renderAsciiArt_space(IMG *img)
{
  FILE *fp=fopen("ascii_space.txt","w"); 
  char *apx="@#N0a-. ";
  int size=8;
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      fprintf(fp,"%c ",apx[7-ind]);
    }
    fprintf(fp,"\n");
  }
}
void renderAsciiArt_char_render_twice(IMG *img)
{
  FILE *fp=fopen("ascii_2char_render.txt","w"); 
  char *apx="@#N0a-. ";
  int size=8;
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      fprintf(fp,"%c%c",apx[7-ind],apx[7-ind]);
    }
    fprintf(fp,"\n");
  }
}
void renderAsciiArt_color(IMG *img)
{
 // FILE *fp=fopen("ascii_color.txt","w"); 
  char *apx="@#N0a-. ";
  int size=8;
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      unsigned int r=img->rgb[i][j].r;
      unsigned int g=img->rgb[i][j].g;
      unsigned int b=img->rgb[i][j].b;
      printf("\033[38;2;%d;%d;%dm", r, g, b);
      fprintf(stdout,"%c ",apx[7-ind]);
    }
    fprintf(stdout,"\n");
  }
  printf("\033[0m");
}

void render_terminal(IMG *img)
{
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      unsigned int r=img->rgb[i][j].r;
      unsigned int g=img->rgb[i][j].g;
      unsigned int b=img->rgb[i][j].b;
      printf("\033[38;2;%d;%d;%dm", r, g, b);
      fprintf(stdout,"<>");
    }
    fprintf(stdout,"\n");
  }
  printf("\033[0m");
}

void freeIMG(IMG *img)
{
  for(int i=0;i<img->height;i++)free(img->rgb[i]);
  free(img->rgb);
  free(img);
}
void createIMG(char *file,IMG *img,BMP_HEADER *bmph,DIB_HEADER *dibh)
{
  FILE *fp=fopen(file,"wb");

  char *buf=calloc(bmph->size,1);
  fwrite(buf,1,bmph->size,fp);
  fseek(fp,0,SEEK_SET);
  fwrite(bmph->bm,1,2,fp);
  fwrite(&bmph->size,1,4,fp);
  fwrite(bmph->garb,1,4,fp);
  fwrite(&bmph->addr,1,4,fp);
  fwrite(&dibh->h_size,1,4,fp);
  fwrite(&dibh->width_px,1,4,fp);
  fwrite(&dibh->height_px,1,4,fp);
  fwrite(&dibh->colour_plains,1,2,fp);
  fwrite(&dibh->no_of_bits_per_px,1,2,fp);
  fwrite(&dibh->compression_method,1,4,fp);
  fwrite(&dibh->img_size,1,4,fp);
  fwrite(&dibh->horizontal_resolution,1,4,fp);
  fwrite(&dibh->vertical_resolution,1,4,fp);
  fwrite(&dibh->no_of_colors,1,4,fp);
  fwrite(&dibh->no_of_imp_colors,1,4,fp);
  
  fseek(fp,bmph->addr,SEEK_SET);

   unsigned int pad=(int)(((dibh->no_of_bits_per_px*dibh->width_px)+31)/32)*4;
   unsigned int rgb_px=pad/sizeof(RGB)+1;
   for(int i=dibh->height_px-1;i>=0;i--)
    {
      fwrite(img->rgb[i],1,pad,fp);
    }
  free(buf);
  fclose(fp);
}
IMG* edgeDetect_gradient(IMG *img,unsigned int bits_per_px,int threshold)
{
  IMG* gimg=calloc(1,sizeof(IMG));
  gimg->height=img->height;
  gimg->width=img->width;
  gimg->rgb=calloc(gimg->height,sizeof(RGB*));
  unsigned int pad=(int)(((bits_per_px*gimg->width)+31)/32)*4;
  unsigned int rgb_px=pad/sizeof(RGB)+1;
  for(int i=0;i<gimg->height;i++)
  {
    gimg->rgb[i]=calloc(rgb_px,sizeof(RGB));
     for(int j=0;j<gimg->width;j++)
     {
        int intensity=(0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b);
        if(i==0||j==0){gimg->rgb[i][j].r=0;gimg->rgb[i][j].g=0;gimg->rgb[i][j].b=0;}
        else
        {
            int prev_intensityX=(0.3*img->rgb[i-1][j].r)+(0.6*img->rgb[i-1][j].g)+(0.1*img->rgb[i-1][j].b);
            int prev_intensityY=(0.3*img->rgb[i][j-1].r)+(0.6*img->rgb[i][j-1].g)+(0.1*img->rgb[i][j-1].b);
            int perv_intensityD=(0.3*img->rgb[i-1][j-1].r)+(0.6*img->rgb[i-1][j-1].g)+(0.1*img->rgb[i-1][j-1].b);
            int x=intensity-prev_intensityX;
            if(x<0)x=-1*x;
            int y=intensity-prev_intensityY;
            if(y<0)y=-1*y;
            int d=intensity-perv_intensityD;
            if(d<0)d=-1*d;
            if(x>=threshold||y>=threshold||d>=threshold){gimg->rgb[i][j].r=255;gimg->rgb[i][j].g=255;gimg->rgb[i][j].b=255;}
            else {gimg->rgb[i][j].r=0;gimg->rgb[i][j].g=0;gimg->rgb[i][j].b=0;}
        }
     }
  }
  return gimg;
}

void edgeRenderer(IMG *img,int threshold)
{
    for(int i=0;i<img->height;i++)
  {
     for(int j=0;j<img->width;j++)
     {
        int intensity=(0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b);
        if(i==0||j==0){printf("  ");}
        else
        {
            int prev_intensityX=(0.3*img->rgb[i-1][j].r)+(0.6*img->rgb[i-1][j].g)+(0.1*img->rgb[i-1][j].b);
            int prev_intensityY=(0.3*img->rgb[i][j-1].r)+(0.6*img->rgb[i][j-1].g)+(0.1*img->rgb[i][j-1].b);
            int perv_intensityD=(0.3*img->rgb[i-1][j-1].r)+(0.6*img->rgb[i-1][j-1].g)+(0.1*img->rgb[i-1][j-1].b);
            int x=intensity-prev_intensityX;
            int negd=0;
            if(x<0)x=-1*x;
            int y=intensity-prev_intensityY;
            if(y<0)y=-1*y;
            int d=intensity-perv_intensityD;
            if(d<0){d=-1*d;negd=1;}
            if(x>=threshold && x>=y && x>=d){printf("- ");}
            else if(y>=threshold && y>=x && y>=d)printf("| ");
            else if(d>=threshold && d>=x && d>=y && negd==0)printf("/ ");
            else if(d>=threshold && d>=x && d>=y && negd==1)printf("\\ ");
            else {printf("  ");}
        }
     }
     printf("\n");
  }
}
void createImgDirect(char *file,IMG *img)
{
   FILE *fp=fopen(file,"wb");
   BMP_HEADER bmph;
   DIB_HEADER dibh;
   bmph.bm[0]='B';
   bmph.bm[1]='M';
   bmph.garb[0]=0;bmph.garb[1]=0;bmph.garb[2]=0;bmph.garb[3]=0;
   bmph.addr=138;
   bmph.size=0;

   dibh.h_size=124;
   dibh.width_px=img->width;
   dibh.height_px=img->height;
   dibh.colour_plains=1;
   dibh.no_of_bits_per_px=24;
   dibh.compression_method=0;
    unsigned int pad=(int)(((dibh.no_of_bits_per_px*img->width)+31)/32)*4;
    unsigned int bitNo=pad/sizeof(RGB)+1;
    dibh.img_size=bitNo*dibh.height_px*3;
    bmph.size=dibh.img_size+138;
    dibh.horizontal_resolution=3779;
    dibh.vertical_resolution=3779;
    dibh.no_of_colors=0;
    dibh.no_of_imp_colors=0;
    createIMG(file,img,&bmph,&dibh);
}
IMG *concatIMGX(IMG *img1,IMG *img2,int bits_per_px,unsigned int padding,unsigned int padr,unsigned int padg,unsigned int padb)
{
    IMG *newImg=calloc(1,sizeof(IMG));
    if(!newImg){errMsg(11,"MEM ALLOC CRASH IMG");return NULL;}
    if(img1->height>=img2->height)newImg->height=img1->height;
    else newImg->height=img2->height;
    newImg->width=img1->width+padding+img2->width;
    newImg->rgb=calloc(newImg->height,sizeof(RGB*));
    if(!newImg->rgb){errMsg(10,"MEM ALLOC CRASH RGB");return NULL;}
    unsigned int pad=(int)(((bits_per_px*newImg->width)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    //printf("success!");
    for(int i=0;i<newImg->height;i++)
    {
        newImg->rgb[i]=calloc(rgb_px,sizeof(RGB));
        if(!newImg->rgb){errMsg(10,"MEM ALLOC CRASH RGB");return NULL;}
        //printf("\nsuccess!");
        int k=0;
        if(i<img1->height){
        for(int j=0;j<img1->width;j++)
        {
          //printf("a");
          newImg->rgb[i][k].r=img1->rgb[i][j].r;
          newImg->rgb[i][k].g=img1->rgb[i][j].g;
          newImg->rgb[i][k].b=img1->rgb[i][j].b;
          k++;
        }
        }
        else{for(int j=0;j<img1->width;j++)
        {
          //printf("a2");
          newImg->rgb[i][k].r=padr;
          newImg->rgb[i][k].g=padg;
          newImg->rgb[i][k].b=padb;
          k++;
        }}
         //printf("success 1");
        for(int j=0;j<padding;j++)
        {
          //printf("b");
          newImg->rgb[i][k].r=padr;
          newImg->rgb[i][k].g=padg;
          newImg->rgb[i][k].b=padb;
          k++;
        }
        //printf("success pad");
        if(i<img2->height){
         for(int j=0;j<img2->width;j++)
        {
          //printf("c");
          newImg->rgb[i][k].r=img2->rgb[i][j].r;
          newImg->rgb[i][k].g=img2->rgb[i][j].g;
          newImg->rgb[i][k].b=img2->rgb[i][j].b;
          k++;
        }
        }
        else{for(int j=0;j<img2->width;j++)
        {
         // printf("c2");
          newImg->rgb[i][k].r=padr;
          newImg->rgb[i][k].g=padg;
          newImg->rgb[i][k].b=padb;
          k++;
        }}
         //printf("success 2");

    }
    //printf("success! FINALLY!");
    return newImg;
}

IMG *concatIMGY(IMG *img1,IMG *img2,int bits_per_px,unsigned int padding,unsigned int padr,unsigned int padg,unsigned int padb)
{
    IMG *newImg=calloc(1,sizeof(IMG));
    if(!newImg){errMsg(11,"MEM ALLOC CRASH IMG");return NULL;}
    if(img1->width>=img2->width)newImg->width=img1->width;
    else newImg->width=img2->width;
    newImg->height=img1->height+padding+img2->height;
    newImg->rgb=calloc(newImg->height,sizeof(RGB*));
    if(!newImg->rgb){errMsg(10,"MEM ALLOC CRASH RGB");return NULL;}
    unsigned int pad=(int)(((bits_per_px*newImg->width)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    //printf("success!");
    int i1=0,i2=0;
    for(int i=0;i<newImg->height;i++)
    {
        newImg->rgb[i]=calloc(rgb_px,sizeof(RGB));
        if(!newImg->rgb){errMsg(10,"MEM ALLOC CRASH RGB");return NULL;}
        //printf("\nsuccess!");
        int j1=0,j2=0;
        for(int j=0;j<newImg->width;j++)
        {
          if(i<img1->height)
          {
            if(j<img1->width)
            {
              newImg->rgb[i][j].r=img1->rgb[i1][j1].r;
              newImg->rgb[i][j].g=img1->rgb[i1][j1].g;
              newImg->rgb[i][j].b=img1->rgb[i1][j1].b;
              j1++;
            }
            else
            {
              newImg->rgb[i][j].r=padr;
              newImg->rgb[i][j].g=padg;
              newImg->rgb[i][j].b=padb;
            }
          }
          else if(i<padding+img1->height)
          {
              newImg->rgb[i][j].r=padr;
              newImg->rgb[i][j].g=padg;
              newImg->rgb[i][j].b=padb;
          }
          else{
              if(j<img2->width)
            {
              newImg->rgb[i][j].r=img2->rgb[i2][j2].r;
              newImg->rgb[i][j].g=img2->rgb[i2][j2].g;
              newImg->rgb[i][j].b=img2->rgb[i2][j2].b;
              j2++;
            }
            else
            {
              newImg->rgb[i][j].r=padr;
              newImg->rgb[i][j].g=padg;
              newImg->rgb[i][j].b=padb;
            }
          }
        }
        if(i<img1->height)i1++;
        else if(i<padding+img1->height);
        else if(i<img2->height+img1->height+padding)i2++;

    }
    //printf("success! FINALLY!");
    return newImg;
}

int convolution_size(int img_size,int kernel_size,int stride)
{
  int output_size=((unsigned int)(img_size-kernel_size)/stride)+1;
  return output_size;
}
unsigned int rgbRange(int val) {
    if (val<=0) {
    return 0;
    } 
    else if (val>=255) {
    return 255;
    }
    return val;
}
float kernelWt(KERNEL *kernel)
{
  float sum=0.0;
  for(int i=0;i<kernel->height;i++)
      {
        for(int j=0;j<kernel->width;j++)
        {
          int d=kernel->matrix[i][j];
          if(d<0)d=d*-1;
            sum+=d;
        }
      }
      return sum;
}
float kernelPosWt(KERNEL *kernel)
{
  float sum=0.0;
  for(int i=0;i<kernel->height;i++)
      {
        for(int j=0;j<kernel->width;j++)
        {
          int d=kernel->matrix[i][j];
          if(d>0)sum+=d;
        }
      }
      return sum;
}
float kernelNegWt(KERNEL *kernel)
{
  float sum=0.0;
  for(int i=0;i<kernel->height;i++)
      {
        for(int j=0;j<kernel->width;j++)
        {
          int d=kernel->matrix[i][j];
          if(d<0)sum+=d;
        }
      }
      return sum;
}
void weighted_kernel_correction(int *r,int *g, int* b,KERNEL *kernel)
{
  float poswt=kernelPosWt(kernel);
  //printf("%f ",poswt);
  float negwt=kernelNegWt(kernel);
  //printf("%f ",negwt);
  if((*r)>=0)(*r)=(*r)/poswt;
  else (*r)=(*r)/negwt;
  if((*g)>=0)(*g)=(*g)/poswt;
  else (*g)=(*g)/negwt;
  if((*b)>=0)(*b)=(*b)/poswt;
  else (*b)=(*b)/negwt;
  *r=rgbRange(*r);
  *g=rgbRange(*g);
  *b=rgbRange(*b);
}
IMG *convolution(IMG *img, KERNEL *kernel,void (*correction)(int *r,int *g, int* b,KERNEL *kernel))
{
    IMG *gimg = calloc(1, sizeof(IMG));
    gimg->height = convolution_size(img->height,kernel->height,kernel->strideY);
    gimg->width = convolution_size(img->width,kernel->width,kernel->strideX);
    gimg->rgb=calloc(gimg->height,sizeof(RGB*));
    unsigned int pad=(int)(((24*gimg->width)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    //printf("enter");
    int a1=0;
    
    for(int i=0;a1<gimg->height && i<img->height;i+=kernel->strideY,a1++)
    {
      gimg->rgb[a1]=calloc(rgb_px,sizeof(RGB));
      int b1=0;
      for(int j=0;b1<gimg->width;j+=kernel->strideX,b1++)
      {
        int r,g,b;
        r=g=b=0;
        for(int i1=i;i1<i+kernel->height;i1++)
        {
          for(int j1=j;j1<j+kernel->width;j1++)
          {
            r=r+img->rgb[i1][j1].r*kernel->matrix[i1-i][j1-j];
            g=g+img->rgb[i1][j1].g*kernel->matrix[i1-i][j1-j];
            b=b+img->rgb[i1][j1].b*kernel->matrix[i1-i][j1-j];
          }
        }
        correction(&r,&g,&b,kernel);
        gimg->rgb[a1][b1].r=r;
        gimg->rgb[a1][b1].g=g;
        gimg->rgb[a1][b1].b=b;
        //printf(".");
      }
     // printf("\n");
    }
    //printf("exit");
    return gimg;
}

void printKernel(KERNEL* kernel)
{
  for(int i=0;i<kernel->height;i++)
      {
        for(int j=0;j<kernel->width;j++)
        {
            printf("%.2f ",kernel->matrix[i][j]);
        }
        printf("\n");
      }
}
void printImgMatrix(IMG *img)
{
   for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      unsigned int r=img->rgb[i][j].r;
      unsigned int g=img->rgb[i][j].g;
      unsigned int b=img->rgb[i][j].b;
      printf("\033[38;2;%d;%d;%dm", r, g, b);
      printf("%d ",(int)(r/3+g/3+b/3));
    }
    printf("\n");
  }

}
IMG * sobelsEdgeDectorHorizontal(IMG *img)
{
    KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
    kernel.matrix[0][0]=1;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=-1;
   kernel.matrix[0][1]=2;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=-2;
   kernel.matrix[0][2]=1;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=-1;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}

IMG * sobelsEdgeDectorVertical(IMG *img)
{
    KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
    kernel.matrix[0][0]=1;
   kernel.matrix[1][0]=2;
   kernel.matrix[2][0]=1;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=-1;
   kernel.matrix[1][2]=-2;
   kernel.matrix[2][2]=-1;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *blr1(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=1;
   kernel.matrix[1][0]=1;
   kernel.matrix[2][0]=1;
   kernel.matrix[0][1]=1;
   kernel.matrix[1][1]=1;
   kernel.matrix[2][1]=1;
   kernel.matrix[0][2]=1;
   kernel.matrix[1][2]=1;
   kernel.matrix[2][2]=1;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *blr2(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=1;
   kernel.matrix[1][0]=2;
   kernel.matrix[2][0]=1;
   kernel.matrix[0][1]=2;
   kernel.matrix[1][1]=4;
   kernel.matrix[2][1]=2;
   kernel.matrix[0][2]=1;
   kernel.matrix[1][2]=2;
   kernel.matrix[2][2]=1;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_1(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=1;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_2(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=1;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_3(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=1;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_4(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=1;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_5(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=1;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_6(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=1;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_7(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=1;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_8(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=1;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *identity_kernel_9(IMG *img)
{
  KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=0;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=0;
   kernel.matrix[1][1]=0;
   kernel.matrix[2][1]=0;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=0;
   kernel.matrix[2][2]=1;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG *cropRight(IMG *img,int px)
{
  px=px/2+1;
  IMG *id=identity_kernel_8(img);
    for(int i=0;i<px;i++)
    {
      IMG *t1=id;
      id=identity_kernel_8(id);
      freeIMG(t1);
    }
    return id;
}
IMG *cropLft(IMG *img,int px)
{
  px=px/2+1;
  IMG *id=identity_kernel_8(img);
    for(int i=0;i<px;i++)
    {
      IMG *t1=id;
      id=identity_kernel_3(id);
      freeIMG(t1);
    }
    return id;
}
IMG *sharpen(IMG *img)
{
    KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=-1;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=-1;
   kernel.matrix[1][1]=100;
   kernel.matrix[2][1]=-1;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=-1;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=1;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}
IMG* brighten(IMG *img, float val)
{
  if(val<0){errMsg(101,"INVALID INPUT");return NULL;}
    IMG *gimg=calloc(1, sizeof(IMG));
    gimg->height=img->height;
    gimg->width=img->width;
    gimg->rgb=calloc(gimg->height,sizeof(RGB*));
    unsigned int pad=(int)(((24*gimg->width)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    printf("in");
  for(int i=0;i<img->height;i++)
  {
    gimg->rgb[i]=calloc(rgb_px,sizeof(RGB));
    for(int j=0;j<img->width;j++)
    {
      gimg->rgb[i][j].r=(unsigned int)rgbRange((int)(val*img->rgb[i][j].r));
      gimg->rgb[i][j].g=(unsigned int)rgbRange((int)(val*img->rgb[i][j].g));
      gimg->rgb[i][j].b=(unsigned int)rgbRange((int)(val*img->rgb[i][j].b));
      //printf("\033[38;2;%d;%d;%dm",gimg->rgb[i][j].r,gimg->rgb[i][j].g,gimg->rgb[i][j].b);
      //fprintf(stdout,"<>");
    }
    //fprintf(stdout,"\n");
  }
  printf("out");
  return gimg;
}
IMG *convolutionEdgeDetection(IMG *img)
{
    KERNEL kernel;
    kernel.height=kernel.width=3;
    kernel.matrix=calloc(kernel.height,sizeof(float*));
    for(int i=0;i<kernel.height;i++)kernel.matrix[i]=calloc(kernel.width,sizeof(double));
   kernel.matrix[0][0]=0;
   kernel.matrix[1][0]=-1;
   kernel.matrix[2][0]=0;
   kernel.matrix[0][1]=-1;
   kernel.matrix[1][1]=4;
   kernel.matrix[2][1]=-1;
   kernel.matrix[0][2]=0;
   kernel.matrix[1][2]=-1;
   kernel.matrix[2][2]=0;
   kernel.strideX=kernel.strideY=3;
   IMG *r=convolution(img,&kernel,weighted_kernel_correction);
    for(int i=0;i<kernel.height;i++)free(kernel.matrix[i]);
    free(kernel.matrix);
   return r;
}

IMG *MaxPooling(IMG *img, KERNEL *kernel)
{
    IMG *gimg = calloc(1, sizeof(IMG));
    gimg->height = convolution_size(img->height,kernel->height,kernel->strideY);
    gimg->width = convolution_size(img->width,kernel->width,kernel->strideX);
    gimg->rgb=calloc(gimg->height,sizeof(RGB*));
    unsigned int pad=(int)(((24*gimg->width)+31)/32)*4;
    unsigned int rgb_px=pad/sizeof(RGB)+1;
    //printf("enter");
    int a1=0;
    
    for(int i=0;a1<gimg->height && i<img->height;i+=kernel->strideY,a1++)
    {
      gimg->rgb[a1]=calloc(rgb_px,sizeof(RGB));
      int b1=0;
      for(int j=0;b1<gimg->width && j<img->width;j+=kernel->strideX,b1++)
      {
        int r,g,b;
        r=img->rgb[i][j].r;g=img->rgb[i][j].g;b=img->rgb[i][j].b;
        int greymax=(0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b);
        for(int i1=i;i1<i+kernel->height;i1++)
        {
          for(int j1=j;j1<j+kernel->width;j1++)
          {
            //pooling code
            int grey=(0.3*img->rgb[i1][j1].r)+(0.6*img->rgb[i1][j1].g)+(0.1*img->rgb[i1][j1].b);
            if(grey>=greymax){
              greymax=grey;
              r=img->rgb[i1][j1].r;
              g=img->rgb[i1][j1].g;
              b=img->rgb[i1][j1].b;

            }
          }
        }
        //printf(".");
        gimg->rgb[a1][b1].r=r;
        gimg->rgb[a1][b1].g=g;
        gimg->rgb[a1][b1].b=b;
        
        //printf("\033[38;2;%d;%d;%dm",gimg->rgb[a1][b1].r,gimg->rgb[a1][b1].g,gimg->rgb[a1][b1].b);
      //fprintf(stdout,"<>");
      }
      //printf("\n");
    }
    //printf("exit");
    return gimg;
}

IMG *borderImg(IMG *img,int bpx,int rb,int gb, int bb)
{
  IMG *gimg=calloc(1,sizeof(IMG));
  unsigned int bits_per_px=24;
  gimg->height=img->height+2*bpx;
  gimg->width=img->width+2*bpx;
  gimg->rgb=calloc(gimg->height,sizeof(RGB*));
  unsigned int pad=(int)(((bits_per_px*gimg->width)+31)/32)*4;
  unsigned int rgb_px=pad/sizeof(RGB)+1;
  for(int i=0;i<gimg->height;i++)
  {
    gimg->rgb[i]=calloc(rgb_px,sizeof(RGB));
     for(int j=0;j<gimg->width;j++)
     {
       if(i<bpx || i>=gimg->height-bpx||j<bpx || j>=gimg->width-bpx)
       {
          gimg->rgb[i][j].r=rb;
          gimg->rgb[i][j].g=gb;
          gimg->rgb[i][j].b=bb;
       }
       else
       {
          gimg->rgb[i][j].r=img->rgb[i-bpx][j-bpx].r;
          gimg->rgb[i][j].g=img->rgb[i-bpx][j-bpx].g;
          gimg->rgb[i][j].b=img->rgb[i-bpx][j-bpx].b;
       }
     }
  }
  return gimg;
}


IMG *uncropFilter(IMG *img,IMG *(filter)(IMG *image),int th)
{
  IMG* gimg=borderImg(img,th,0,0,0);
  IMG *t=gimg;
  gimg=filter(gimg);
  free(t);
  return gimg;
}
void renderAsciiArt_terminal(IMG *img)
{
  char apx[8]="@#N-.   ";
  int size=8;
  for(int i=0;i<img->height;i++)
  {
    for(int j=0;j<img->width;j++)
    {
      int ind=((0.3*img->rgb[i][j].r)+(0.6*img->rgb[i][j].g)+(0.1*img->rgb[i][j].b))/32;
      printf("%c%c",apx[7-ind],apx[7-ind]);
    }
    printf("\n");
  }
}

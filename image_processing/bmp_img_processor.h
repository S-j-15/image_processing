typedef struct BMPheader{
char bm[2];
unsigned int size;
char garb[4];
unsigned int addr;
}BMP_HEADER;

typedef struct DIBheader
{
  unsigned int h_size;
  unsigned int width_px;
  unsigned int height_px;
  unsigned int colour_plains;
  unsigned int no_of_bits_per_px;
  unsigned int compression_method;
  unsigned int img_size;
  unsigned int horizontal_resolution;
  unsigned int vertical_resolution;
  unsigned int no_of_colors;
  unsigned int no_of_imp_colors;
}DIB_HEADER;

typedef struct rgb
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
}RGB;

typedef struct img
{
  unsigned int height;
  unsigned int width;
  RGB **rgb;
}IMG;

typedef struct kernel
{
  unsigned int height;
  unsigned int width;
  unsigned int strideY;
  unsigned int strideX;
  double **matrix;
}KERNEL;

int errMsg(int err,char *default_msg);
int isBMP(char *file);
BMP_HEADER* getBMPHeader(char *file);
DIB_HEADER* getDIBHeader(char *file);
int isValidFormat(char *file);
IMG *getImage(char *file);
IMG* greyScale(IMG *img);
IMG* bwImg(IMG *img);
void renderAsciiArt(IMG *img);
void renderAsciiArt_space(IMG *img);
void renderAsciiArt_char_render_twice(IMG *img);
void renderAsciiArt_color(IMG *img);
void freeIMG(IMG *img);
void createIMG(char *file,IMG *img,BMP_HEADER *bmph,DIB_HEADER *dibh);
void render_terminal(IMG *img);
IMG* edgeDetect_gradient(IMG *img,unsigned int bits_per_px,int threshold);
void edgeRenderer(IMG *img,int threshold);
void createImgDirect(char *file,IMG *img);
IMG *concatIMGX(IMG *img1,IMG *img2,int bits_per_px,unsigned int padding,unsigned int padr,unsigned int padg,unsigned int padb);
IMG *concatIMGY(IMG *img1,IMG *img2,int bits_per_px,unsigned int padding,unsigned int padr,unsigned int padg,unsigned int padb);
int convolution_size(int img_size,int kernel_size,int stride);
IMG *convolution(IMG *img,KERNEL *kernel,void (*correction)(int *r,int *g, int* b,KERNEL *kernel));
void printKernel(KERNEL* kernel);
unsigned int rgbRange(int val);
void printImgMatrix(IMG *img);
float kernelWt(KERNEL *kernel);
float kernelPosWt(KERNEL *kernel);
float kernelNegWt(KERNEL *kernel);
void weighted_kernel_correction(int *r,int *g, int* b,KERNEL *kernel);
IMG * sobelsEdgeDectorHorizontal(IMG *img);
IMG * sobelsEdgeDectorVertical(IMG *img);
IMG *blr1(IMG *img);
IMG *blr2(IMG *img);
IMG *identity_kernel_1(IMG *img);
IMG *identity_kernel_2(IMG *img);
IMG *identity_kernel_3(IMG *img);
IMG *identity_kernel_4(IMG *img);
IMG *identity_kernel_5(IMG *img);
IMG *identity_kernel_6(IMG *img);
IMG *identity_kernel_7(IMG *img);
IMG *identity_kernel_8(IMG *img);
IMG *identity_kernel_9(IMG *img);
IMG *cropLft(IMG *img,int px);
IMG *cropRight(IMG *img,int px);
IMG *sharpen(IMG *img);
IMG* brighten(IMG *img, float val);
IMG *convolutionEdgeDetection(IMG *img);
IMG *borderImg(IMG *img,int bpx,int rb,int gb, int bb);
IMG *uncropFilter(IMG *img,IMG *(filter)(IMG *image),int th);
void renderAsciiArt_terminal(IMG *img);
IMG *MaxPooling(IMG *img, KERNEL *kernel);

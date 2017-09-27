#ifndef _BITMAP_H 
#define _BITMAP_H 

#include "basetype.h" 

/* constants for the biCompression field */
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

/* 
 * get the image width that the image will be storage in
 * width: the real width of the image
 * BitCount: bit count that to storage a pixel needs
 */
#define GETIMAGESTORAGEWIDTH(width, BitCount) ((((width) * (BitCount) / 8) + 3) & ~3)

#pragma pack(2) 

typedef struct _tagBitmapFileHeader {
    USHORT usType;       /* 2bytes, 文件类型，必须是BM即0x4D42 */
    ULONG  ulSize;      /* 4bytes, BMP文件大小，字节数 */
    USHORT usReserved1;  /* 2bytes, 保留位 */
    USHORT usReserved2;  /* 2bytes, 保留位 */
    ULONG  ulOffBits;   /* 4bytes, BMP实际数据，*/
    
} BitmapFileHeader;

typedef struct _tagBitmapInfoHeader {
    ULONG   ulSize;          /* 本结构的大小， 字节数 */
    LONG     lWidth;          /* bitmap的宽度，像素 */
    LONG     lHeight;         /* bitmap的高度，像素 */
    USHORT usPlanes;         /* 目标设备的调色板数目，必须是1 */
    USHORT usBitCount;       /* 为表示每个像素所需要的位：0,1,4,8,16,24,32 */
    
    /* 
     * 自底向上的bitmap的压缩类型，（自顶向下的bitmap不能被压缩）。
     * BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS，BI_JPEG， BI_PNG
     */
    ULONG  ulCompression;
    ULONG  ulSizeImage;     /* 图片的大小，字节数， BI_RGB时，可能为0 */
    LONG    lXPelsPerMeter;  /* 水平分辨率 */
    LONG    lYPelsPerMeter;  /* 垂直分辨率 */
    
    /* 
     * Bitmap真正使用的颜色表中的颜色的索引值，
     * 如果是0，则使用nBitCount所代表的最大的颜色值。如1->1, 4->15, 8->255。
     * 如果非0，且nBitCount < 16，则ulClrUsed指定真实的颜色值。
     * 如果非0，且nBitCount >= 16，则指定颜色表的大小来优化系统的调色板，
     * 如果nBitCount == 16 或者32，则最佳调色板在3个DWORD标识之后立即启动。
     */
    ULONG  ulClrUsed;
    ULONG  ulClrImportant;  /* 用来显示位图的颜色索引值，0，则所有颜色都需要 */
    
} BitmapInfoHeader;

/*
 *调色板Palette(对那些需要调色板的位图文件而言的, 24位和32位是不需要调色板的)
 *调色板结构体个数等于使用的颜色数
 */
typedef struct _tagRGBQuad {
    BYTE bRed;      //  该颜色的红色分量
    BYTE bGreen;    //  该颜色的绿色分量
    BYTE bBlue;     //  该颜色的蓝色分量
    BYTE bReserved; //  保留值
} RGBQuad;

typedef struct _tagBitmap {
  BitmapFileHeader    stBmpFileHeader;
  BitmapInfoHeader   stBmpInfoHeader;
  VOID *                    pColorData;
} Bitmap;

#pragma pack()

UINT LoadBitmapFromFile(IN CONST CHAR *filename, OUT Bitmap *pstBitmap); 
UINT SaveBitmapToFile(IN CONST CHAR *filename, IN CONST Bitmap *pstBitmap);
UINT ReleaseBitmap(IN Bitmap *pstBitmap); 
UINT CloneBitmap(CONST IN Bitmap *pstBitmap, OUT Bitmap *pstNewBitmap);

#endif

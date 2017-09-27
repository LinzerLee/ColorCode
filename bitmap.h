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
    USHORT usType;       /* 2bytes, �ļ����ͣ�������BM��0x4D42 */
    ULONG  ulSize;      /* 4bytes, BMP�ļ���С���ֽ��� */
    USHORT usReserved1;  /* 2bytes, ����λ */
    USHORT usReserved2;  /* 2bytes, ����λ */
    ULONG  ulOffBits;   /* 4bytes, BMPʵ�����ݣ�*/
    
} BitmapFileHeader;

typedef struct _tagBitmapInfoHeader {
    ULONG   ulSize;          /* ���ṹ�Ĵ�С�� �ֽ��� */
    LONG     lWidth;          /* bitmap�Ŀ�ȣ����� */
    LONG     lHeight;         /* bitmap�ĸ߶ȣ����� */
    USHORT usPlanes;         /* Ŀ���豸�ĵ�ɫ����Ŀ��������1 */
    USHORT usBitCount;       /* Ϊ��ʾÿ����������Ҫ��λ��0,1,4,8,16,24,32 */
    
    /* 
     * �Ե����ϵ�bitmap��ѹ�����ͣ����Զ����µ�bitmap���ܱ�ѹ������
     * BI_RGB��BI_RLE8��BI_RLE4��BI_BITFIELDS��BI_JPEG�� BI_PNG
     */
    ULONG  ulCompression;
    ULONG  ulSizeImage;     /* ͼƬ�Ĵ�С���ֽ����� BI_RGBʱ������Ϊ0 */
    LONG    lXPelsPerMeter;  /* ˮƽ�ֱ��� */
    LONG    lYPelsPerMeter;  /* ��ֱ�ֱ��� */
    
    /* 
     * Bitmap����ʹ�õ���ɫ���е���ɫ������ֵ��
     * �����0����ʹ��nBitCount�������������ɫֵ����1->1, 4->15, 8->255��
     * �����0����nBitCount < 16����ulClrUsedָ����ʵ����ɫֵ��
     * �����0����nBitCount >= 16����ָ����ɫ��Ĵ�С���Ż�ϵͳ�ĵ�ɫ�壬
     * ���nBitCount == 16 ����32������ѵ�ɫ����3��DWORD��ʶ֮������������
     */
    ULONG  ulClrUsed;
    ULONG  ulClrImportant;  /* ������ʾλͼ����ɫ����ֵ��0����������ɫ����Ҫ */
    
} BitmapInfoHeader;

/*
 *��ɫ��Palette(����Щ��Ҫ��ɫ���λͼ�ļ����Ե�, 24λ��32λ�ǲ���Ҫ��ɫ���)
 *��ɫ��ṹ���������ʹ�õ���ɫ��
 */
typedef struct _tagRGBQuad {
    BYTE bRed;      //  ����ɫ�ĺ�ɫ����
    BYTE bGreen;    //  ����ɫ����ɫ����
    BYTE bBlue;     //  ����ɫ����ɫ����
    BYTE bReserved; //  ����ֵ
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

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h> 
#include <math.h>
#include "colorcode.h"
#include "error.h"

#define PI              3.1415926  
#define EDGE_VALUE      235  
#define NON_EDGE_VALUE  16 
#define BASIC_COLOR_NUM 4

enum { HOR = 0, VER, POS45, NEG45 };  
STATIC CONST INT ROW_TABLE[4] = {0, 1, 1, 1};  
STATIC CONST INT COL_TABLE[4] = {1, 0, 1, -1};

STATIC CONST CHAR *COLOR_NAME[] = {
	"红",
	"绿",
	"蓝",
	"黑"
};

STATIC CONST CHAR *COLOR_CODE[] = {
	"00",
	"01",
	"10",
	"11"
};

STATIC RGBQuad BASIC_COLOR[BASIC_COLOR_NUM] = {
	{255, 0, 0, 0},
    {0, 255, 0, 0},
	{0, 0, 255, 0},
	{0, 0, 0, 0}
};

STATIC DOUBLE SIMILARITY_THRESHOLD = 0.33f;

STATIC CONST DOUBLE ROW_PIXEL_VAILD_THRESHOLD = 0.3f; 
STATIC CONST DOUBLE ROW_VAILD_THRESHOLD = 0.2f;
STATIC CONST DOUBLE ROW_INVAILD_THRESHOLD = 0.02f;

STATIC CONST DOUBLE COL_PIXEL_VAILD_THRESHOLD = 0.3f; 
STATIC CONST DOUBLE COL_VAILD_THRESHOLD = 0.2f;
STATIC CONST DOUBLE COL_INVAILD_THRESHOLD = 0.02f;

STATIC BYTE g_bThreshold = 160;
STATIC ColorCodeProcessType g_ProcessStepQueue[PROCESS_STEP_QUEUE_MAX] = { PT_MIN };
STATIC UINT g_QueueStartPos = 0, g_QueueEndPos = 0;

STATIC RGBQuad *g_ProcessDataStack[PROCESS_DATA_STACK_MAX] = { NULL };
STATIC UINT g_StackBottomPos = 0, g_StackTopPos = 0;

ColorCodeCallback_PF ColorCodeCallback[PT_MAX] = {
	NULL,
	ColorCodeSaveData, 
	ColorCodeSimilarity, 
	ColorCodeGraying,
    ColorCodeBinaryzation,
    ColorCodeFlipBinaryzation,
    ColorCodeEdgeDeteRoberts, 
    ColorCodeEdgeDeteCanny,  
	ColorCodeScanBorder, 
	ColorCodeClipBorder,
    ColorCodeConnectedRegion, 
    ColorCodeStrengthenPartition,
    ColorCodeMaskData, 
    ColorCodeRecoverData, 
    ColorCodeRecognitionColorCode,
};
 
CONST CHAR *ProcessError[PT_MAX] = {
	"PT_MIN",
	"PT_SAVE_DATA",                  // 保存数据 
	"PT_SIMILARITY",                 // 相似度 
	"PT_GRAYING",                    // 灰度化 
	"PT_BINARYZATION",               // 二值化 
	"PT_FLIP_BINARYZATION",          // 二值化反转 
	"PT_EDGE_DETE_ROBERTS",          // Roberts边缘检测 
	"PT_EDGE_DETE_CANNY",            // Canny边缘检测 
	"PT_SCAN_BORDER",                // 扫描边框 
	"PT_CLIP_BORDER",                // 边框裁剪(操作前提:扫描边框)
	"PT_CONNECTED_REGION",           // 连通区域 
	"PT_STRENGTHEN_PARTITION",       // 分区
	"PT_MASK_DATA",                  // 蒙版 
	"PT_RECOVER_DATA",               // 恢复数据 
	"PT_RECOGNITION_COLORCODE",      // 识别彩码 
};

UINT InitColorCodeBuffer(IN CONST Bitmap *pstBitmap, OUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	RGBQuad *dataOfBmp = NULL;
	LONG lWidth = 0L, l_width = 0L;
	LONG lHeight = 0L;
	LONG lSize = 0L;
	UINT k = 0;
    INT i = 0, j = 0, index = 0;
	BYTE *pColorData = NULL;
	 
	assert(pstBitmap != NULL);
	assert(pstBuffer != NULL);
	
	if(NULL == pstBitmap->pColorData) {
		uiErrCode = ERROR_FAILD;
	} else {
		//将位图数据转化为RGB数据
		lWidth = pstBitmap->stBmpInfoHeader.lWidth;
		lHeight = pstBitmap->stBmpInfoHeader.lHeight;
		l_width = GETIMAGESTORAGEWIDTH(pstBitmap->stBmpInfoHeader.lWidth, 
		                                        pstBitmap->stBmpInfoHeader.usBitCount);
		lSize = lWidth * lHeight * sizeof(RGBQuad); 
		dataOfBmp = (RGBQuad *)malloc(lSize);
		if(NULL == dataOfBmp) {
			uiErrCode = ERROR_MALLOC_FAILD;
		} else {
			memset(dataOfBmp, 0, lSize);
			
			pColorData = (BYTE *)pstBitmap->pColorData; 
			for(i=0; i<lHeight; i++) {
		        for(j=0; j<lWidth; j++) {
		        	index = (lHeight-1 - i) * lWidth + j;
		            k = i*l_width + j*pstBitmap->stBmpInfoHeader.usBitCount / 8 ;
		            dataOfBmp[index].bRed = pColorData[k+2];
		            dataOfBmp[index].bGreen = pColorData[k+1];
		            dataOfBmp[index].bBlue = pColorData[k];
		        }
		    }
		     
		    pstBuffer->pstData = dataOfBmp;
		    pstBuffer->ulWidth = lWidth;
			pstBuffer->ulHeight = lHeight;
			pstBuffer->ulSize = lSize;
			pstBuffer->usBitCount = pstBitmap->stBmpInfoHeader.usBitCount;
			// 重置彩码数据 
			
			uiErrCode = ERROR_SUCCESS; 
		}
	}
	
	return uiErrCode; 
}

UINT ConvertCCB2Bitmap(IN CONST ColorCodeBuffer *pstBuffer, OUT Bitmap *pstBitmap) {
	UINT uiErrCode = ERROR_SUCCESS;
	UINT l_width = 0;
	UINT height = 0;
	UINT k = 0;
    INT i = 0, j = 0, index = 0;
	BYTE *pColorData = NULL;
		 
	assert(pstBuffer != NULL);
	assert(pstBitmap != NULL);
	
	l_width = GETIMAGESTORAGEWIDTH(pstBuffer->ulWidth, pstBuffer->usBitCount);
	height = pstBuffer->ulHeight;
	// 填充BitmapFileHeader
	pstBitmap->stBmpFileHeader.usType = 0x4d42;
	pstBitmap->stBmpFileHeader.usReserved1 = 0;
	pstBitmap->stBmpFileHeader.usReserved2 = 0;
	pstBitmap->stBmpFileHeader.ulOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
	pstBitmap->stBmpFileHeader.ulSize = pstBitmap->stBmpFileHeader.ulOffBits + height * l_width;
	// 填充BitmapInfoHeader
	pstBitmap->stBmpInfoHeader.ulSize = sizeof(BitmapInfoHeader);
	pstBitmap->stBmpInfoHeader.lWidth = pstBuffer->ulWidth;
	pstBitmap->stBmpInfoHeader.lHeight = pstBuffer->ulHeight;
	pstBitmap->stBmpInfoHeader.usPlanes = 1;
	pstBitmap->stBmpInfoHeader.usBitCount = pstBuffer->usBitCount;
	pstBitmap->stBmpInfoHeader.ulCompression = 0;
	pstBitmap->stBmpInfoHeader.ulSizeImage = height * l_width;
	pstBitmap->stBmpInfoHeader.lXPelsPerMeter = 96;
	pstBitmap->stBmpInfoHeader.lYPelsPerMeter = 96;
	pstBitmap->stBmpInfoHeader.ulClrUsed = 0;
	pstBitmap->stBmpInfoHeader.ulClrImportant = 0;
         // 填充数据
	pstBitmap->pColorData = (BYTE *)malloc(pstBitmap->stBmpInfoHeader.ulSizeImage);
         memset(pstBitmap->pColorData, 0, pstBitmap->stBmpInfoHeader.ulSizeImage);
      
	pColorData = (BYTE *)pstBitmap->pColorData; 
    for(i=0; i<pstBuffer->ulHeight; i++) {
	    for(j=0; j<pstBuffer->ulWidth; j++) {
	        k = i * l_width + j * (pstBuffer->usBitCount / 8) ;
	        index = (pstBuffer->ulHeight-1 - i) * pstBuffer->ulWidth + j;
		    pColorData[k+2] = pstBuffer->pstData[index].bRed;
            pColorData[k+1] = pstBuffer->pstData[index].bGreen;
		    pColorData[k] = pstBuffer->pstData[index].bBlue;
	    }
    }
		
	return uiErrCode; 
}

UINT ReleaseColorCodeBuffer(INOUT ColorCodeBuffer *pstBuffer) {
	if(NULL != pstBuffer->pstData) {
		free(pstBuffer->pstData);
		pstBuffer->pstData = NULL; 
	}
	
	return ERROR_SUCCESS;
} 

DOUBLE pixel_similarity (IN CONST RGBQuad *x, IN CONST RGBQuad *y) {
	return pow(pow(x->bBlue - y->bBlue, 2.0f) + pow(x->bGreen - y->bGreen, 2.0f) + pow(x->bRed - y->bRed, 2.0f), 0.5f);
}

RGBQuad *CloneColorCodeData(CONST IN ColorCodeBuffer *pstBuffer) {
	UINT i = 0, j = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
	RGBQuad *pstTempData = NULL;
	UINT offset = 0; 
	
	pstTempData = (RGBQuad *)malloc(pstBuffer->ulSize);
	
	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			pstTempData[offset] = pstData[offset];
		}
	}
	
	return pstTempData;
} 

UINT ColorCodeSaveData (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
	RGBQuad *pstTempData = NULL;
	
	pstTempData = CloneColorCodeData(pstBuffer);
	if(NULL!=pstTempData) {
		PushProcessDataStack(pstTempData);
	} else {
		uiErrCode = ERROR_FAILD;
	}
	
	return uiErrCode;
}

UINT ColorCodeSimilarity (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	DOUBLE similarity = 0.0f, min_similarity = -1.0f;
	UINT k = 0;
    UINT i = 0, j = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    UINT offset = 0; 
    BYTE bReserved = 0;
    
	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			similarity = -1.0f;
			min_similarity = -1.0f;
			bReserved = 0;
			for(k=0; k<BASIC_COLOR_NUM; ++k) {
				similarity = pixel_similarity(&pstData[offset], &BASIC_COLOR[k]) / 1000;
			    // printf("similarity : %f\n", similarity); 
				if(min_similarity<0 || similarity<min_similarity) {
			    	min_similarity = similarity;
			    	bReserved = k + 1;
				}
			}
			
			if(min_similarity<=SIMILARITY_THRESHOLD) {
			    pstData[offset].bReserved = bReserved; // max_similarity * 255; 
			} else {
				pstData[offset].bRed = 255;
			    pstData[offset].bGreen = 255;
			    pstData[offset].bBlue = 255;
			    pstData[offset].bReserved = 0;
			}
			// printf("%3d ", pstData[offset].bReserved); 
		}
		// printf("\n"); 
	}
	
	uiErrCode = ERROR_SUCCESS;
	return uiErrCode;
} 

UINT ColorCodeGraying (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
    UINT i = 0, j = 0, index = 0;
    BYTE bGray, bRed, bGreen, bBlue; 
    
    for(i=0; i<pstBuffer->ulHeight; i++) {
	    for(j=0; j<pstBuffer->ulWidth; j++) {
		    bRed = pstBuffer->pstData[index].bRed;
            bGreen = pstBuffer->pstData[index].bGreen;
		    bBlue = pstBuffer->pstData[index].bBlue;
		    bGray = bRed * 0.299f + bGreen * 0.587f + bBlue * 0.114;
		    pstBuffer->pstData[index].bRed = bGray;
            pstBuffer->pstData[index].bGreen = bGray;
		    pstBuffer->pstData[index].bBlue = bGray;
	        index++;
	    }
    }
    
    uiErrCode = ERROR_SUCCESS; 
    
    return uiErrCode;
}

UINT ColorCodeBinaryzation (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0, j = 0, index = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    BYTE bGray; 
    
    for(i=0; i<ulHeight; i++) {
	    for(j=0; j<ulWidth; j++) {
	    	index = i * ulWidth + j;
		    bGray = pstData[index].bRed;
		    if(bGray >= g_bThreshold) {
		    	bGray = 255;
			} else {
				bGray = 0;
			}
			pstData[index].bRed = bGray;
            pstData[index].bGreen = bGray;
		    pstData[index].bBlue = bGray;
		    pstData[index].bReserved = 255 - bGray;
	        // printf("(%3d,%3d,%3d) ", i, j, pstBuffer->pstData[index].bReserved);
	    }
	    // printf("\n");
    }
    
    uiErrCode = ERROR_SUCCESS; 
    
    return uiErrCode;
}

UINT ColorCodeFlipBinaryzation (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0, j = 0, index = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    BYTE bGray; 
    
    for(i=0; i<ulHeight; i++) {
	    for(j=0; j<ulWidth; j++) {
	    	index = i * ulWidth + j;
		    bGray = pstData[index].bRed;
		    if(bGray >= g_bThreshold) {
		    	bGray = 0;
			} else {
				bGray = 255;
			}
			pstData[index].bRed = bGray;
            pstData[index].bGreen = bGray;
		    pstData[index].bBlue = bGray;
		    pstData[index].bReserved = 255 - bGray;
	    }
    }
    
    uiErrCode = ERROR_SUCCESS; 
    
    return uiErrCode;
} 

UINT ColorCodeEdgeDeteRoberts (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
    UINT i = 0, j = 0, index = 0;
    BYTE bValue = 0; 
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    
    for(i=0; i<ulHeight; i++) {
	    for(j=0; j<ulWidth; j++) {
	    	index = i * ulWidth + j;
		    bValue = abs(pstData[index].bRed - pstData[index+1+ulWidth].bRed) +
			         abs(pstData[index+1].bRed - pstData[index+ulWidth].bRed);
		    if(bValue >= 255) {
		    	bValue = 255;
			}
			
			pstData[index].bRed = bValue;
            pstData[index].bGreen = bValue;
		    pstData[index].bBlue = bValue;
	    }
    }
    
    uiErrCode = ERROR_SUCCESS; 
    
    return uiErrCode;
} 

UINT ColorCodeGaussianFilter(INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	INT i = 0, j = 0, k = 0;  // 索引可以为负数 
    INT index = 0; 
    UINT uiValue = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
	CONST INT RADIUS = 3;
	
	BYTE filter[7] = {1, 6, 19, 27, 19, 6, 1};
	BYTE weight = 79;
	BYTE *fltp = filter + 3;
	
    BYTE *tmp = NULL;
	
	tmp = (BYTE *)malloc(sizeof(BYTE) * ulWidth);
	if(NULL != tmp) {
		for(i=0; i<ulHeight; i++) {
			memset(tmp, 0, sizeof(BYTE) * ulWidth);
			
			for(j=RADIUS; j<ulWidth-RADIUS; ++j) {
				uiValue = 0;
				index = i*ulWidth+j;
				
				for(k=-RADIUS; k<=RADIUS; ++k) {
					uiValue += pstData[index+k].bRed * fltp[k];
				}
				tmp[j] = uiValue / weight;
			}
			
			for(j=RADIUS; j<ulWidth-RADIUS; ++j) {
				index = i*ulWidth+j;
				pstData[index].bRed = tmp[j];
	            pstData[index].bGreen = tmp[j];
			    pstData[index].bBlue = tmp[j];
			}
	    }
	    
	    free(tmp);
	} else {
		uiErrCode = ERROR_MALLOC_FAILD;
		return uiErrCode; 
	}
	
	tmp = (BYTE *)malloc(sizeof(BYTE) * ulHeight);
	if(NULL != tmp) {
		for(j=0; j<ulWidth; ++j) {
			memset(tmp, 0, sizeof(BYTE) * ulHeight);
			
			for(i=RADIUS; i<ulHeight-RADIUS; ++i) {
				uiValue = 0;
				index = i*ulWidth+j;
				
				for(k=-RADIUS; k<=RADIUS; ++k) {
					uiValue += pstData[index+k*ulWidth].bRed * fltp[k];
				}
				tmp[i] = uiValue / weight;
			} 
			
			for(i=RADIUS; i<ulHeight-RADIUS; ++i) {
				index = i*ulWidth+j;
				pstData[index].bRed = tmp[i];
		        pstData[index].bGreen = tmp[i];
				pstData[index].bBlue = tmp[i];
			}
		}
			
		free(tmp);
	} else {
		uiErrCode = ERROR_MALLOC_FAILD;
		return uiErrCode; 
	}
	
	uiErrCode = ERROR_SUCCESS;
	return uiErrCode; 
}

UINT ColorCodeCalculateGrad(INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	UINT i = 0, j = 0; 
    UINT index = 0; 
	CONST DOUBLE tan225 = tan(PI / 8);
	CONST DOUBLE tan675 = tan(PI * 3 / 8);
	ULONG ulOffset = 0;
	BYTE *tmp = NULL;
	ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    INT diff_x, diff_y;
    UINT dir;
    INT gradient;
	
    tmp = (BYTE *)malloc(sizeof(BYTE) * ulWidth);
    if(NULL != tmp) {
    	for(i=0; i<ulHeight; i++) {
		    for(j=0; j<ulWidth; j++) {
				ulOffset = i * ulWidth + j;
				diff_x = pstData[ulOffset + 1].bRed - pstData[ulOffset].bRed;
				diff_y = pstData[ulOffset + ulWidth].bRed - pstData[ulOffset].bRed;
	
				DOUBLE tanv = diff_y / (diff_x + 0.0001);
				dir = NEG45;
				if(tanv < tan225 && tanv >= -tan225) {
					dir = HOR;
				} else if(tanv < tan675 && tanv >= tan225) {
				    dir = POS45;
				} else if(tanv < -tan675 || tanv >= tan675) {
				    dir = VER;
				}
	
				gradient = (UINT)sqrt((DOUBLE)(diff_x * diff_x + diff_y * diff_y));
				tmp[j] = (gradient << 2) + dir; // 低2位放方向，高位放梯度值
			}
			
			for(j=0; j<ulWidth; j++) {
				index = i*ulWidth+j;
				pstData[index].bRed = tmp[j];
		        pstData[index].bGreen = tmp[j];
				pstData[index].bBlue = tmp[j];
			} 
		}
    } else {
    	uiErrCode = ERROR_MALLOC_FAILD;
		return uiErrCode; 
	}
	
	uiErrCode = ERROR_SUCCESS;
	return uiErrCode; 
}

UINT ColorCodeSuppressNonMax(INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	UINT i = 0, j = 0; 
	ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    UINT dir;
    INT row, col;
    INT offset, offset1, offset2;
    INT grad0, grad1, grad2;
     
	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			dir = pstData[offset].bRed & 0x3;
			row = ROW_TABLE[dir];
		    col = COL_TABLE[dir];

			offset1 = (i + row) * ulWidth + j + col;
			offset2 = (i - row) * ulWidth + j - col;
			grad0 = pstData[offset].bRed >> 2;
			grad1 = pstData[offset1].bRed >> 2;
			grad2 = pstData[offset2].bRed >> 2;
			
			if(grad0 >= grad1 && grad0 >= grad2) {
				pstData[offset].bRed = grad0;
				pstData[offset].bGreen = grad0;
				pstData[offset].bBlue = grad0;
			}
		}
	}
	
	uiErrCode = ERROR_SUCCESS;
	return uiErrCode; 
} 

UINT ColorCodeDoubleThreshold(INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_FAILD;
	UINT i = 0, j = 0; 
	ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
	BYTE low_thres = 7, high_thres = 14;
	INT offset;
    INT row, col; 
	
	BYTE *pbData = (BYTE *)malloc(sizeof(BYTE) * ulHeight * ulWidth); 
	 
	memset(pbData, NON_EDGE_VALUE, sizeof(BYTE) * ulHeight * ulWidth);

	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			if(pstData[offset].bRed >= high_thres) { 
				pbData[offset] = EDGE_VALUE;
			}
			else if(pstData[offset].bRed >= low_thres) {
				for(row = -1; row <= 1; ++row) {
					for(col = -1; col <= 1; ++col) {
						if(pstData[(i + row) * ulWidth + j + col].bRed >= high_thres) {
							pbData[offset] = EDGE_VALUE;
							break;
						}
					}
				}
			}
		}
	}
	
	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			pstData[offset].bRed = pbData[offset];
			pstData[offset].bGreen = pbData[offset];
			pstData[offset].bBlue = pbData[offset];
		}
	}
	
	free(pbData);
	
	uiErrCode = ERROR_SUCCESS;
	return uiErrCode; 
} 

UINT ColorCodeEdgeDeteCanny (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
	
	uiErrCode = (ERROR_SUCCESS==uiErrCode) ? ColorCodeGaussianFilter(pstBuffer) : uiErrCode;
	uiErrCode = (ERROR_SUCCESS==uiErrCode) ? ColorCodeCalculateGrad(pstBuffer) : uiErrCode;
	uiErrCode = (ERROR_SUCCESS==uiErrCode) ? ColorCodeSuppressNonMax(pstBuffer) : uiErrCode;
	uiErrCode = (ERROR_SUCCESS==uiErrCode) ? ColorCodeDoubleThreshold(pstBuffer) : uiErrCode;
	
	return uiErrCode;
} 

UINT ColorCodeScanBorder (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
	UINT i = 0, j = 0; 
	ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    INT offset;
    UINT row_vaild_pixel; 
	UINT row_vaild_count, row_invaild_count;
	UINT col_vaild_pixel; 
	UINT col_vaild_count, col_invaild_count;
	
    INT top = -1, bottom = -1, left = -1, right = -1; 
	 
    row_vaild_count = 0;
    row_invaild_count = 0; 
    for(i=0; i<ulHeight; i++) {
    	row_vaild_pixel = 0; 
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			if(0<pstData[offset].bReserved) {
				++row_vaild_pixel;
			}
			
			// 连续有效行统计 
			if(row_vaild_pixel>=ulWidth*ROW_PIXEL_VAILD_THRESHOLD) {
				++ row_vaild_count;
				// 连续有效行的出现可以使无效行统计重置 
				if(row_vaild_count>=ulHeight*ROW_INVAILD_THRESHOLD) {
					row_invaild_count = 0; 
				} 
				break; 
			}
		}
		// 无效行统计
		if(row_vaild_pixel<ulWidth*ROW_PIXEL_VAILD_THRESHOLD) {
			++ row_invaild_count;
			// 连续无效行的出现可以使有效行统计重置 
			if(row_invaild_count>=ulHeight*ROW_INVAILD_THRESHOLD) {
				row_vaild_count = 0; 
			}  
		}
		
		// 首次发现 
		if(-1==top && row_vaild_count>=ulHeight*ROW_VAILD_THRESHOLD) {
			top = i + 1 - row_vaild_count;
		} 
		// 已经找到了有效行 
		if(-1!=top && row_invaild_count>=ulHeight*ROW_INVAILD_THRESHOLD) {
		    bottom = i + 1 - row_invaild_count;
		    row_invaild_count = 0; 
		}	
	}
	
	if(-1==top) {
		top = 0; 
	} 
	
	if(-1==bottom) {
		bottom = ulHeight-1; 
	}
	
	col_vaild_count = 0;
    col_invaild_count = 0; 
    for(j=0; j<ulWidth; j++) {
    	col_vaild_pixel = 0; 
        for(i=0; i<ulHeight; i++) {
			offset = i * ulWidth + j;
			if(0<pstData[offset].bReserved) {
				++col_vaild_pixel;
			}
			
			// 连续有效列统计 
			if(col_vaild_pixel>=ulHeight*COL_PIXEL_VAILD_THRESHOLD) {
				++ col_vaild_count;
				// 连续有效列的出现可以使无效行统计重置 
				if(col_vaild_count>=ulHeight*COL_INVAILD_THRESHOLD) {
					col_invaild_count = 0; 
				} 
				break; 
			}
		}
		// 无效列统计
		if(col_vaild_pixel<ulHeight*COL_PIXEL_VAILD_THRESHOLD) {
			++ col_invaild_count;
			// 连续无效列的出现可以使有效行统计重置 
			if(col_invaild_count>=ulWidth*COL_INVAILD_THRESHOLD) {
				col_vaild_count = 0; 
			} 
		}
		
		// 首次发现 
		if(-1==left && col_vaild_count>=ulWidth*COL_VAILD_THRESHOLD) {
			left = j + 1 - col_vaild_count;
		} 
		// 已经找到了有效行 
		if(-1!=left && col_invaild_count>=ulWidth*COL_INVAILD_THRESHOLD) {
		    right = j + 1 - col_invaild_count;
			col_invaild_count = 0; 
		}	
	}
	
	if(-1==left) {
		left = 0;
	}
	
	if(-1==right) {
		right = ulWidth-1;
	}
	
	if(top!=-1 && bottom!=-1 && left!=-1 && right!=-1) {
		pstBuffer->auAnchorPoint[ANCHOR_TOP] = top;
		pstBuffer->auAnchorPoint[ANCHOR_BOTTOM] = bottom;
		pstBuffer->auAnchorPoint[ANCHOR_LEFT] = left;
		pstBuffer->auAnchorPoint[ANCHOR_RIGHT] = right;
		
		for(i=0; i<ulHeight; i++) {
			for(j=0; j<ulWidth; j++) {
				offset = i * ulWidth + j;
				if(!(i>=top && i<=bottom && j>=left && j<=right)) {
					pstData[offset].bRed = 255;
				    pstData[offset].bGreen = 255;
				    pstData[offset].bBlue = 255;
				    pstData[offset].bReserved = 0;
				}
			}
		}
	} else {
		uiErrCode = INFO_NOT_RECOGNITION; 
	}
	 
	return uiErrCode;
} 

UINT ColorCodeClipBorder (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
	UINT i = 0, j = 0; 
	ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstData = pstBuffer->pstData;
    INT offset;
    INT top, bottom, left, right;
    
    top = pstBuffer->auAnchorPoint[ANCHOR_TOP];
	bottom = pstBuffer->auAnchorPoint[ANCHOR_BOTTOM];
	left = pstBuffer->auAnchorPoint[ANCHOR_LEFT];
	right = pstBuffer->auAnchorPoint[ANCHOR_RIGHT];
	
	for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			if(!(i>=top && i<=bottom && j>=left && j<=right)) {
				pstData[offset].bRed = 255;
				pstData[offset].bGreen = 255;
				pstData[offset].bBlue = 255;
				pstData[offset].bReserved = 0;
			}
		}
	}
	
	return uiErrCode; 
}
/************************************************
 *
 *          *******       1.行扫描填充按照 
 *          *1*2*3*         2、4、1、3的顺 
 *          *******         序寻找连通区域
 *          *4*#*5*       2.列扫描填充按照
 *          *******         4、2、1、6的顺
 *          *6*7*8*         序寻找连通区域
 *          *******       3.FillTag采用行 
 *                          扫描填充算法
 ************************************************/ 
UINT FillTag(RGBQuad *pstData, ULONG ulWidth, ULONG ulHeight, 
             UINT top, UINT bottom, UINT left, UINT right) {
    UINT uiErrCode = ERROR_SUCCESS;
    INT i = 0, j = 0, xPos = 0, yPos = 0;
	INT offset, index;
	BYTE bTag = 0;
	
	for(i=top; i<=bottom; i++) {
		for(j=left; j<=right; j++) {
			offset = i * ulWidth + j;
			if(0!=pstData[offset].bReserved) {
				// 2
				xPos = i - 1;
				yPos = j;
				index = xPos * ulWidth + yPos;
				if(xPos>=top && 0!=pstData[index].bReserved) {
					pstData[offset].bReserved = pstData[index].bReserved;
					// printf("(%3d,%3d,%3d) ", i, j, pstData[offset].bReserved); 
					continue;
				}
				// 4
				xPos = i;
				yPos = j - 1;
				index = xPos * ulWidth + yPos;
				if(yPos>=left && 0!=pstData[index].bReserved) {
					pstData[offset].bReserved = pstData[index].bReserved;
					// printf("(%3d,%3d,%3d) ", i, j, pstData[offset].bReserved);
					continue;
				}
				// 1
				xPos = i - 1;
				yPos = j - 1;
				index = xPos * ulWidth + yPos;
				if(xPos>=top && yPos>=left && 0!=pstData[index].bReserved) {
					pstData[offset].bReserved = pstData[index].bReserved;
					// printf("(%3d,%3d,%3d) ", i, j, pstData[offset].bReserved);
					continue;
				}
				// 3
				xPos = i - 1;
				yPos = j + 1;
				index = xPos * ulWidth + yPos;
				if(xPos>=top && yPos>=left && 0!=pstData[index].bReserved) {
					pstData[offset].bReserved = pstData[index].bReserved;
					// printf("(%3d,%3d,%3d) ", i, j, pstData[offset].bReserved);
					continue;
				}
				
				pstData[offset].bReserved = ++bTag; 
			} 
			// printf("(%3d,%3d,%3d) ", i, j, pstData[offset].bReserved);
		}
		// printf("\n"); 
	}
	
	return uiErrCode;
} 

// 最大支持225个区域(编号为1~225, 15X15) 
UINT ColorCodeConnectedRegion (INOUT ColorCodeBuffer *pstBuffer) {
    UINT uiErrCode = ERROR_SUCCESS;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    INT offset;
    BYTE bTag = 1;
	UINT uiConnPixelCnt = 0;
	UINT top = pstBuffer->auAnchorPoint[ANCHOR_TOP],
	     bottom = pstBuffer->auAnchorPoint[ANCHOR_BOTTOM],
	     left = pstBuffer->auAnchorPoint[ANCHOR_LEFT],
	     right = pstBuffer->auAnchorPoint[ANCHOR_RIGHT];
	     
    uiErrCode = FillTag(pstData, ulWidth, ulHeight, top, bottom, left, right);
	
    return uiErrCode; 
}

UINT CleanRow(INOUT ColorCodeBuffer *pstBuffer, UINT row, BYTE bValue) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT j = 0;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    INT offset = row * ulWidth;
    
    for(j=0; j<ulWidth; j++) {
    	pstData[offset].bRed = bValue;
		pstData[offset].bGreen = bValue;
		pstData[offset].bBlue = bValue;
		pstData[offset].bReserved = 255 - bValue; 
		
		++offset; 
	} 
    
    return uiErrCode; 
} 

UINT CleanCol(INOUT ColorCodeBuffer *pstBuffer, UINT col, BYTE bValue) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    INT offset = col;
    
    for(i=0; i<ulHeight; i++) {
    	pstData[offset].bRed = bValue;
		pstData[offset].bGreen = bValue;
		pstData[offset].bBlue = bValue;
		pstData[offset].bReserved = 255 - bValue; 
		
		offset += ulWidth; 
	} 
    
    return uiErrCode; 
} 

UINT FillPartition(INOUT ColorCodeBuffer *pstBuffer, BYTE bValue, 
                   UINT top, UINT bottom, UINT left, UINT right) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0, j = 0;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
	INT offset;
	// printf("top : %d, bottom : %d, left = %d, right = %d\r\n", top, bottom, left, right);
	for(i=top; i<=bottom; ++i) {
		for(j=left; j<=right; ++j) {
			offset = i * ulWidth + j;
			pstData[offset].bRed = bValue;
			pstData[offset].bGreen = bValue;
			pstData[offset].bBlue = bValue;
			pstData[offset].bReserved = 255 - bValue;
		} 
	} 
	
	return uiErrCode; 
}

VOID PrintReserved(ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0, j = 0, m = 0, n = 0;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    INT offset;
    for(i=0; i<ulHeight; i++) {
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			printf("%-3d ", pstData[offset].bReserved);
		} 
		printf("\n"); 
	}		
} 

CONST CHAR *ColorCodeToString(BYTE color_index) {
	return COLOR_CODE[color_index]; 
}

VOID SetSimilarityThreshold(DOUBLE threshold) {
	SIMILARITY_THRESHOLD = threshold; 
} 

UINT CrossPoint(DOUBLE x1, DOUBLE y1, DOUBLE x2, DOUBLE y2, 
                DOUBLE x3, DOUBLE y3, DOUBLE x4, DOUBLE y4, 
                UINT *x, UINT *y) {
	DOUBLE b1, b2, k1, k2, xx, yy;
	UINT uiErrCode = ERROR_SUCCESS;
	// b1 = (y2x1-y1x2) / (x1-x2)
	// b2 = (y4x3-y3x4) / (x3-x4)
	// k1 = (y1-b1) / x1
	// k2 = (y3-b2) / x3
	// x = (b2-b1) / (k1 -k2)
	// y = k1x + b1
	
	b1 = (y2*x1-y1*x2) / (x1-x2);
	b2 = (y4*x3-y3*x4) / (x3-x4);
	k1 = (y1-b1) / x1;
	k2 = (y3-b2) / x3;
	xx = (b2-b1) / (k1 -k2);
	yy = k1*xx + b1;
	
	*x = xx;
	*y = yy;
	
	printf("(%f, %f) (%f, %f) (%f, %f) (%f, %f)== (%d, %d)\n", 
	       x1, y1, x2, y2, x3, y3, x4, y4,
	       *x, *y);
	return uiErrCode; 
}

UINT ColorCodeStrengthenPartition (INOUT ColorCodeBuffer *pstBuffer) {
    UINT uiErrCode = ERROR_SUCCESS;
    UINT i = 0, j = 0, m = 0, n = 0;
    RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    INT offset;
    UINT row_vaild_pixel, col_vaild_pixel; 
    INT row_top = -1, row_bottom = -1, col_left = -1, col_right = -1; 
    BYTE bCount = 0; 
    /*
    // 整齐切割 
    for(i=0; i<ulHeight; i++) {
    	row_vaild_pixel = 0; 
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			if(0<pstData[offset].bReserved) {
				++row_vaild_pixel;
			}
		}
		
		if(row_vaild_pixel<ulWidth*ROW_PIXEL_VAILD_THRESHOLD) {
			uiErrCode = CleanRow(pstBuffer, i, 255); 
		}
	} 
    
    for(j=0; j<ulWidth; j++) {
    	col_vaild_pixel = 0; 
		for(i=0; i<ulHeight; i++) {
			offset = i * ulWidth + j;
			if(0<pstData[offset].bReserved) {
				++col_vaild_pixel;
			}
		}
		
		if(col_vaild_pixel<ulHeight*COL_PIXEL_VAILD_THRESHOLD) {
			uiErrCode = CleanCol(pstBuffer, j, 255); 
		}
	} 
	// 区块补全 
	for(i=0; i<ulHeight; i++) {
    	row_vaild_pixel = 0; 
		for(j=0; j<ulWidth; j++) {
			offset = i * ulWidth + j;
			if(0<pstData[offset].bReserved) {
				++row_vaild_pixel;
			}
			
			if(-1==row_top && row_vaild_pixel>=ulWidth*ROW_PIXEL_VAILD_THRESHOLD) {
				row_top = i;
				break; 
			} 
		}
		
		if(-1!=row_top && -1==row_bottom && row_vaild_pixel<ulWidth*ROW_PIXEL_VAILD_THRESHOLD) {
			 row_bottom = i - 1;
		}
		
		if(-1!=row_top && -1!=row_bottom) {
			for(m=0; m<ulWidth; m++) {
				col_vaild_pixel = 0; 
				for(n=0; n<ulHeight; n++) {
					offset = n * ulWidth + m;
					if(0<pstData[offset].bReserved) {
						++col_vaild_pixel;
					}
					
					if(-1==col_left && col_vaild_pixel>=ulHeight*COL_PIXEL_VAILD_THRESHOLD) {
						col_left = m; 
						break; 
					} 
			    }
			    
				if(-1!=col_left && -1==col_right && col_vaild_pixel<ulHeight*COL_PIXEL_VAILD_THRESHOLD) {
					col_right = m - 1;
				} 
				
				if(-1!=col_left && -1!=col_right) {
					uiErrCode = FillPartition(pstBuffer, 0, row_top, row_bottom, col_left, col_right);
					++bCount; 
					col_left = -1; 
					col_right = -1;
				} 
			} 
			row_top = -1;
		    row_bottom = -1; 
		}
	} 
	
	bCount = pow(bCount, 0.5); 
	*/
	bCount = 5; /* 固定规格 */
	
	pstBuffer->bRowSize = bCount;
	pstBuffer->bColSize = bCount;
	
	ulHeight =  pstBuffer->auAnchorPoint[ANCHOR_BOTTOM] -  
	            pstBuffer->auAnchorPoint[ANCHOR_TOP];
	ulWidth  =  pstBuffer->auAnchorPoint[ANCHOR_RIGHT] -  
	            pstBuffer->auAnchorPoint[ANCHOR_LEFT];
	
	for(i=0; i<=bCount; ++i) {
		row_top = ulHeight / bCount * i + pstBuffer->auAnchorPoint[ANCHOR_TOP];
		for(j=0; j<=bCount; ++j) {
			col_left = ulWidth / bCount * j + pstBuffer->auAnchorPoint[ANCHOR_LEFT];
			if(row_top>=pstBuffer->ulHeight) {
				row_top = pstBuffer->ulHeight - 1;
			}
			
			if(col_left>=pstBuffer->ulWidth) {
				col_left = pstBuffer->ulWidth - 1;
			}
			pstBuffer->abGridPoint[i][j].y = row_top;
			pstBuffer->abGridPoint[i][j].x = col_left;
			
			// printf("(%3d, %3d) ", col_left, row_top);
		}
		// printf("\n");
	}
	/* 
	for(i=0; i<bCount; ++i) {
		for(j=0; j<bCount; ++j) {
			row_top = pstBuffer->abGridPoint[i][j].y;
			col_left = pstBuffer->abGridPoint[i][j].x;
			row_bottom = pstBuffer->abGridPoint[i+1][j].y;
			col_right = pstBuffer->abGridPoint[i][j+1].x;			
		}
	}
	*/ 
    return uiErrCode; 
} 

UINT ColorCodeMaskData (INOUT ColorCodeBuffer *pstBuffer) {
	UINT uiErrCode = ERROR_SUCCESS;
	UINT i = 0, j = 0;
    ULONG ulHeight = pstBuffer->ulHeight;
    ULONG ulWidth = pstBuffer->ulWidth;
    RGBQuad *pstMaskData = pstBuffer->pstData;
	UINT offset = 0; 
	
	RGBQuad *pstData = PopProcessDataStack();
	
	if(NULL!=pstData) {
		for(i=0; i<ulHeight; i++) {
			for(j=0; j<ulWidth; j++) {
				offset = i * ulWidth + j;
				if(pstMaskData[offset].bReserved>0) {
					pstData[offset].bReserved = pstMaskData[offset].bReserved;
				} else {
					pstData[offset].bReserved = 0;
					pstData[offset].bRed = 255;
					pstData[offset].bGreen = 255;
					pstData[offset].bBlue = 255;
				}
				// printf("(%3d, %3d, %3d) ", pstData[offset].bRed, pstData[offset].bGreen, pstData[offset].bBlue); 
				//printf("(%3d, %3d, %3d) ", pstMaskData[offset].bRed, pstMaskData[offset].bGreen, pstMaskData[offset].bBlue); 
			}
			// printf("\n"); 
		}
		
		pstBuffer->pstData = pstData;
		free(pstMaskData);
	} else {
	    uiErrCode = ERROR_FAILD; 
	} 
	
	return uiErrCode;  
} 

UINT ColorCodeRecoverData (INOUT ColorCodeBuffer *pstBuffer) {
    UINT uiErrCode = ERROR_SUCCESS;
	RGBQuad *pstMaskData = pstBuffer->pstData;
	RGBQuad *pstData = PopProcessDataStack();
	
	if(NULL!=pstData) {
		pstBuffer->pstData = pstData;
		free(pstMaskData);
	} else {
	    uiErrCode = ERROR_FAILD; 
	}
	
	return uiErrCode;
}

BYTE CalculateColorCode (INOUT ColorCodeBuffer *pstBuffer,
                         IN UINT row_top, IN UINT row_bottom, 
						 IN UINT col_left, IN UINT col_right) {
	UINT i = 0, j = 0, k = 0;
	RGBQuad *pstData = pstBuffer->pstData;
    ULONG ulWidth = pstBuffer->ulWidth;
	INT offset;
	UINT index;
	INT iMaxValue = 0, iMaxIndex = 0;
	UINT uiColorStat[BASIC_COLOR_NUM] = {0};
	
	for(i=row_top; i<=row_bottom; ++i) {
		for(j=col_left; j<=col_right; ++j) {
			offset = i * ulWidth + j;
			if(pstData[offset].bReserved>0) {
				index = pstData[offset].bReserved - 1;
				++uiColorStat[index];
			}
		}
	}
	// printf("-"); 
	for(k=0; k<BASIC_COLOR_NUM; ++k) {
		// printf("%d-", uiColorStat[k]); 
		if(uiColorStat[k]>iMaxValue) {
			iMaxValue = uiColorStat[k];
			iMaxIndex = k + 1;
		}
	}
	
	return iMaxIndex;
}

UINT ColorCodeRecognitionColorCode (INOUT ColorCodeBuffer *pstBuffer) {
    UINT uiErrCode = ERROR_SUCCESS;
	UINT i = 0, j = 0;
	INT row_top = -1, row_bottom = -1, col_left = -1, col_right = -1;
	
	for(i=0; i<pstBuffer->bRowSize; ++i) {
		for(j=0; j<pstBuffer->bColSize; ++j) {
			row_top = pstBuffer->abGridPoint[i][j].y;
			col_left = pstBuffer->abGridPoint[i][j].x;
			row_bottom = pstBuffer->abGridPoint[i+1][j].y;
			col_right = pstBuffer->abGridPoint[i][j+1].x;
			pstBuffer->abCodePoint[i][j] = CalculateColorCode(pstBuffer, row_top, row_bottom, col_left, col_right);			
		    
			// printf("[%d][%d]=%s ", i, j, COLOR_NAME[pstBuffer->abCodePoint[i][j]-1]); 
		}
		// printf("\n"); 
	}
	
	return uiErrCode;
} 
 
UINT EnqueueProcessStepQueue(CONST IN CCPT ccpt) {
	UINT uiErrCode = ERROR_SUCCESS;
	
	UINT uiNextPos = (g_QueueEndPos + 1) % PROCESS_STEP_QUEUE_MAX;
	
	if(g_QueueEndPos < g_QueueStartPos) {
		if(uiNextPos == g_QueueStartPos) {
			return WARING_QUEUE_FULL;
		}
	}
	
	g_ProcessStepQueue[g_QueueEndPos] = ccpt;
	g_QueueEndPos = uiNextPos;
	
	return uiErrCode; 
}

CCPT PeekProcessStepQueue() {
	if(g_QueueEndPos==g_QueueStartPos) {
		return PT_MIN;
	}
	
	return g_ProcessStepQueue[g_QueueStartPos];
}

CCPT DequeueProcessStepQueue() {
	CCPT ccpt = PeekProcessStepQueue();
	
	if(PT_MIN!=ccpt) {
		g_QueueStartPos = (g_QueueStartPos + 1) % PROCESS_STEP_QUEUE_MAX;
	} 
	
	return ccpt;
}

UINT PushProcessDataStack(IN RGBQuad *pstData) {
	UINT uiErrCode = ERROR_SUCCESS;
	
	if(g_StackTopPos>=PROCESS_DATA_STACK_MAX-1) {
		return WARING_STACK_FULL; 
	} 
	
	g_ProcessDataStack[g_StackTopPos++] = pstData;
	
	return uiErrCode; 
}

RGBQuad *PeekProcessDataStack() {
	if(g_StackTopPos==g_StackBottomPos) {
		return NULL; 
	}
	
	return g_ProcessDataStack[g_StackTopPos-1];
} 

RGBQuad *PopProcessDataStack() {
	RGBQuad *pstData = PeekProcessDataStack();
	 
	if(NULL!=pstData) {
		--g_StackTopPos;
	}
	
	return pstData;
} 

UINT ProcessData(INOUT ColorCodeBuffer *pstBuffer) {
	CCPT ccpt = PT_MIN;
	UINT uiErrCode = ERROR_SUCCESS;
	
	while((ERROR_SUCCESS==uiErrCode) && 
	      (PT_MIN!=(ccpt=DequeueProcessStepQueue()))) {
	        uiErrCode = ColorCodeCallback[ccpt](pstBuffer); 
	        if(ERROR_SUCCESS!=uiErrCode) {
	        	printf("%s execute faild[code %d]!\n", ProcessError[ccpt], uiErrCode);
	        	return uiErrCode;
			}
	} 
	
	return uiErrCode; 
}

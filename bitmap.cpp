#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h> 

#include "bitmap.h"
#include "error.h" 

UINT LoadBitmapFromFile(IN CONST CHAR *filename, OUT Bitmap *pstBitmap){
	FILE* pfile = NULL;
	WORD fileType;
    UINT uiErrCode = ERROR_SUCCESS;
	 
	assert(filename != NULL);
	assert(pstBitmap != NULL);
	
	pfile = fopen(filename, "rb");
	
	if(NULL == pfile) {
		return ERROR_FILE_NOT_EXIST;
	} else {
        fread(&fileType, 1, sizeof(WORD), pfile);
        if(fileType != 0x4d42) {
            return ERROR_FILE_NOT_BITMAP;
        }
        
        pstBitmap->stBmpFileHeader.usType = fileType;
        fread(&pstBitmap->stBmpFileHeader.ulSize, 1, sizeof(BitmapFileHeader) - sizeof(WORD), pfile);

        //读取位图信息头信息
        fread(&pstBitmap->stBmpInfoHeader, 1, sizeof(BitmapInfoHeader), pfile);
    }
    
    // 仅支持24位和32位 
    assert(24==pstBitmap->stBmpInfoHeader.usBitCount || 
	       32==pstBitmap->stBmpInfoHeader.usBitCount ); 

    UINT width = pstBitmap->stBmpInfoHeader.lWidth;
    UINT height = pstBitmap->stBmpInfoHeader.lHeight;
    // 分配内存空间把源图存入内存
    // 计算位图的实际宽度并确保它为32的倍数
    UINT l_width = GETIMAGESTORAGEWIDTH(width, pstBitmap->stBmpInfoHeader.usBitCount);
    
    BYTE *pColorData = (BYTE *)malloc(height * l_width);
    memset(pColorData, 0, height * l_width);
    ULONG ulDataSize = height * l_width;

    //把位图数据信息读到数组里
    fread(pColorData, 1, ulDataSize, pfile);
   
    pstBitmap->pColorData = pColorData;
    fclose(pfile);

    return uiErrCode; 
}

STATIC UINT _file_write(const void* buffer, size_t size, FILE* stream) {
	size_t remain = size;
	size_t tmp = 0;
	UINT uiFaildCnt = 0;
	
	assert(NULL != buffer);
	assert(NULL != stream);
	
	while(0 < remain) {
		tmp = fwrite(buffer, 1, remain, stream);
		remain -= tmp;
		if(tmp <=0 ) {
			++uiFaildCnt;
		}
		
		if(uiFaildCnt>=5) {
			return ERROR_FILE_WRITE_FAILD;
		}
	}
	
	return ERROR_SUCCESS;
}

UINT SaveBitmapToFile(IN CONST CHAR *filename, IN CONST Bitmap *pstBitmap) {
    UINT uiErrCode = ERROR_FAILD;
    FILE* pfile = NULL;
    
    assert(filename != NULL);
	assert(pstBitmap != NULL);
	
	pfile = fopen(filename, "wb");
	
	if(NULL == pfile) {
		uiErrCode = ERROR_FILE_OPEN_FAILD;
	} else {
        uiErrCode = _file_write(&pstBitmap->stBmpFileHeader, sizeof(pstBitmap->stBmpFileHeader), pfile);
        
		if(ERROR_SUCCESS == uiErrCode)
		    uiErrCode = _file_write(&pstBitmap->stBmpInfoHeader, sizeof(pstBitmap->stBmpInfoHeader), pfile);
        
		if(ERROR_SUCCESS == uiErrCode)
		    uiErrCode = _file_write(pstBitmap->pColorData, pstBitmap->stBmpInfoHeader.ulSizeImage, pfile);
    }
    
    fclose(pfile);
    
    return uiErrCode;
} 

UINT ReleaseBitmap(IN Bitmap *pstBitmap) {
	if(NULL != pstBitmap->pColorData) {
		free(pstBitmap->pColorData);
		pstBitmap->pColorData = NULL; 
	}
	
	return ERROR_SUCCESS;
}

UINT CloneBitmap(CONST IN Bitmap *pstBitmap, OUT Bitmap *pstNewBitmap) {
	assert(NULL != pstBitmap); 
	assert(NULL != pstNewBitmap); 
	
	*pstNewBitmap = *pstBitmap;
	pstNewBitmap->pColorData = NULL;
	pstNewBitmap->pColorData = malloc(pstBitmap->stBmpInfoHeader.ulSizeImage);
	
	if(NULL == pstNewBitmap->pColorData)
	    return ERROR_MALLOC_FAILD;
	    
	return ERROR_SUCCESS;
}

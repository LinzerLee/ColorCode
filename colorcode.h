#ifndef _COLORCODE_H 
#define _COLORCODE_H 

#define ROW_MAX 16
#define COL_MAX 16

#define PROCESS_STEP_QUEUE_MAX    16
#define PROCESS_DATA_STACK_MAX    16

#include "basetype.h" 
#include "bitmap.h"

typedef enum _emAnchorBorderType{
	ANCHOR_TOP, 
	ANCHOR_BOTTOM, 
	ANCHOR_LEFT, 
	ANCHOR_RIGHT, 
	ABCHOR_NR, 
}AnchorBorderType; 

typedef struct _tagPoint{
    UINT x;
    UINT y;
}Point; 

typedef struct _tagColorCodeBuffer{
    RGBQuad *   pstData;
    ULONG       ulWidth;    
    ULONG       ulHeight;
    ULONG       ulSize;           /* 4bytes, pstData字节数 */ 
    USHORT      usBitCount;       /* 为表示每个像素所需要的位：0,1,4,8,16,24,32 */
    UINT        auAnchorPoint[ABCHOR_NR];
    BYTE        bRowSize;
    BYTE        bColSize;
    Point       abGridPoint[ROW_MAX + 1][COL_MAX + 1];
    BYTE        abCodePoint[ROW_MAX][COL_MAX];
    RGBQuad     abColorCode[ROW_MAX][COL_MAX];
}ColorCodeBuffer, CCB;

typedef UINT (* ColorCodeCallback_PF) (INOUT ColorCodeBuffer *pstBuffer); 

typedef enum _emColorCodeProcessType {
	PT_MIN = 0,
	PT_SAVE_DATA,                  // 保存数据 
	PT_SIMILARITY,                 // 相似度 
	PT_GRAYING,                    // 灰度化 
	PT_BINARYZATION,               // 二值化 
	PT_FLIP_BINARYZATION,          // 二值化反转 
	PT_EDGE_DETE_ROBERTS,          // Roberts边缘检测 
	PT_EDGE_DETE_CANNY,            // Canny边缘检测 
	PT_SCAN_BORDER,                // 扫描边框 
	PT_CLIP_BORDER,                // 边框裁剪(操作前提:扫描边框)
	PT_CONNECTED_REGION,           // 连通区域 
	PT_STRENGTHEN_PARTITION,       // 分区
	PT_MASK_DATA,                  // 蒙版 
	PT_RECOVER_DATA,               // 恢复数据 
	PT_RECOGNITION_COLORCODE,      // 识别彩码 
	PT_MAX,
}ColorCodeProcessType, CCPT;

UINT InitColorCodeBuffer(IN CONST Bitmap *pstBitmap, OUT ColorCodeBuffer *pstBuffer);
UINT ConvertCCB2Bitmap(IN CONST ColorCodeBuffer *pstBuffer, OUT Bitmap *pstBitmap);
UINT ReleaseColorCodeBuffer(INOUT ColorCodeBuffer *pstBuffer);
// 处理操作 
UINT ColorCodeSaveData (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeSimilarity (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeGraying (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeBinaryzation (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeFlipBinaryzation (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeEdgeDeteRoberts (INOUT ColorCodeBuffer *pstBuffer); 
UINT ColorCodeEdgeDeteCanny (INOUT ColorCodeBuffer *pstBuffer); 
UINT ColorCodeScanBorder (INOUT ColorCodeBuffer *pstBuffer); 
UINT ColorCodeClipBorder (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeConnectedRegion (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeStrengthenPartition (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeMaskData (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeRecoverData (INOUT ColorCodeBuffer *pstBuffer);
UINT ColorCodeRecognitionColorCode (INOUT ColorCodeBuffer *pstBuffer);
// 辅助函数 
VOID PrintReserved(ColorCodeBuffer *pstBuffer);
CONST CHAR *ColorCodeToString(BYTE color_index); 
VOID SetSimilarityThreshold(DOUBLE threshold); 
// 处理队列 
UINT EnqueueProcessStepQueue(CONST IN CCPT ccpt);
CCPT DequeueProcessStepQueue();
CCPT PeekProcessStepQueue();

// 数据栈 
UINT PushProcessDataStack(IN RGBQuad *pstData);
RGBQuad *PopProcessDataStack();
RGBQuad *PeekProcessDataStack();

UINT ProcessData(INOUT ColorCodeBuffer *pstBuffer); 
#endif

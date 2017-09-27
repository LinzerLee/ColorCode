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
    ULONG       ulSize;           /* 4bytes, pstData�ֽ��� */ 
    USHORT      usBitCount;       /* Ϊ��ʾÿ����������Ҫ��λ��0,1,4,8,16,24,32 */
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
	PT_SAVE_DATA,                  // �������� 
	PT_SIMILARITY,                 // ���ƶ� 
	PT_GRAYING,                    // �ҶȻ� 
	PT_BINARYZATION,               // ��ֵ�� 
	PT_FLIP_BINARYZATION,          // ��ֵ����ת 
	PT_EDGE_DETE_ROBERTS,          // Roberts��Ե��� 
	PT_EDGE_DETE_CANNY,            // Canny��Ե��� 
	PT_SCAN_BORDER,                // ɨ��߿� 
	PT_CLIP_BORDER,                // �߿�ü�(����ǰ��:ɨ��߿�)
	PT_CONNECTED_REGION,           // ��ͨ���� 
	PT_STRENGTHEN_PARTITION,       // ����
	PT_MASK_DATA,                  // �ɰ� 
	PT_RECOVER_DATA,               // �ָ����� 
	PT_RECOGNITION_COLORCODE,      // ʶ����� 
	PT_MAX,
}ColorCodeProcessType, CCPT;

UINT InitColorCodeBuffer(IN CONST Bitmap *pstBitmap, OUT ColorCodeBuffer *pstBuffer);
UINT ConvertCCB2Bitmap(IN CONST ColorCodeBuffer *pstBuffer, OUT Bitmap *pstBitmap);
UINT ReleaseColorCodeBuffer(INOUT ColorCodeBuffer *pstBuffer);
// ������� 
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
// �������� 
VOID PrintReserved(ColorCodeBuffer *pstBuffer);
CONST CHAR *ColorCodeToString(BYTE color_index); 
VOID SetSimilarityThreshold(DOUBLE threshold); 
// ������� 
UINT EnqueueProcessStepQueue(CONST IN CCPT ccpt);
CCPT DequeueProcessStepQueue();
CCPT PeekProcessStepQueue();

// ����ջ 
UINT PushProcessDataStack(IN RGBQuad *pstData);
RGBQuad *PopProcessDataStack();
RGBQuad *PeekProcessDataStack();

UINT ProcessData(INOUT ColorCodeBuffer *pstBuffer); 
#endif

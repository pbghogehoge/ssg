/*                                                                           */
/*   DD_CLIP2D.h   ２Ｄ図形描画用クリッピング                                */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DD_CLIP2D_H
#define PBGWIN_DD_CLIP2D_H		"DD_CLIP2D : Version 0.01 : Update 1999/12/03"


// 更新履歴 //

// 1999/12/03 : CLIP.C から引っぱり出してきました


// ヘッダファイル //
#include "DX_TYPE.h"


// 関数 //
int  pascal Grp_PClip(POINT *dest,POINT *src,int n);		// ポリゴン用クリッピング


#define OUTCODE_X		0x03
#define OUTCODE_LEFT	0x01
#define OUTCODE_RIGHT	0x02

#define OUTCODE_Y		0x30
#define OUTCODE_TOP		0x20
#define OUTCODE_BOTTOM	0x10


// アウトコードを取得する //
IBYTE GetLineOutcode(int x,int y)
{
	extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;
	BYTE	temp = OUTCODE_LEFT|OUTCODE_BOTTOM;

	if(CLIP_X_MIN <= x){
		temp ^= OUTCODE_X;
		if(x <= CLIP_X_MAX) temp ^= OUTCODE_RIGHT;
	}

	if(y <= CLIP_Y_MAX){
		temp ^= OUTCODE_Y;
		if(CLIP_Y_MIN <= y) temp ^= OUTCODE_TOP;
	}

	return temp;
}

// 実際にクリッピングを行う //
IBOOL GrpLineCut(int *x1,int *y1,int *x2,int *y2,BYTE outcode)
{
	extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;
	int		temp;

	// ｘ方向のクリッピングが必要な場合 //
	if(outcode&OUTCODE_X){
		temp = (outcode&OUTCODE_LEFT) ? CLIP_X_MIN : CLIP_X_MAX;
		(*x1) -= (*x2);		// dx
		if((*x1)==0) return FALSE;
		(*y1) -= (*y2);		// dy

		(*y1) = ( (temp-(*x2))*(*y1) )/(*x1) + (*y2);
		(*x1) = temp;

		if((*y1)<CLIP_Y_MIN)      outcode = OUTCODE_TOP;
		else if((*y1)>CLIP_Y_MAX) outcode = OUTCODE_BOTTOM;
		else return TRUE;
	}

	// ｙ方向のクリッピングが必要な場合 //
	if(outcode&OUTCODE_Y){
		temp = (outcode&OUTCODE_TOP) ? CLIP_Y_MIN : CLIP_Y_MAX;
		(*y1) -= (*y2);		// dy
		if((*y1)==0) return FALSE;

		(*x1) -= (*x2);		// dx

		(*x1) = ( (temp-(*y2))*(*x1) )/(*y1) + (*x2);
		(*y1) = temp;
	}

	if((*x1)>=CLIP_X_MIN && (*x1)<=CLIP_X_MAX) return TRUE;
	return FALSE;
}

// ReturnValue == TRUE  : 完全に範囲内もしくはクリッピングが正常に行われた //
// ReturnValue == FALSE : 完全に範囲外
IBOOL GrpLineClip(int *x1,int *y1,int *x2,int *y2)
{
	BYTE	Outcode1,Outcode2;

	Outcode1 = GetLineOutcode(*x1,*y1);
	Outcode2 = GetLineOutcode(*x2,*y2);

	if((Outcode1 & Outcode2) != 0) return FALSE;
	if(Outcode1==0 && Outcode2==0) return TRUE;

	if(GrpLineCut(x1,y1,x2,y2,Outcode1)==TRUE && GrpLineCut(x2,y2,x1,y1,Outcode2)==TRUE)
		return TRUE;
	else
		return FALSE;
}


#endif

/*                                                                           */
/*   DD_GRP2D.h   ２Ｄ＆８Ｂｉｔ時の図形描画関数                             */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DD_GRP2D_H
#define PBGWIN_DD_GRP2D_H		"DD_GRP2D : Version 0.03 : Update 1999/01/12"


// 更新履歴 //

// 2000/ 1/12 : α関連の関数を一部追加
// 1999/12/03 : ２Ｄ図形描画作成(αをのぞく)


// ヘッダファイル //
#include "DX_TYPE.h"


// 関数 //
FVOID _2DSetup(void);							// 描画系関数ポインタをセットする

FVOID _2DLock(void);							// 画面描画の準備をする
FVOID _2DUnlock(void);							// 画面描画を完了する

FVOID _2DSetColor(BYTE r,BYTE g,BYTE b);		// 色セット
FVOID _2DSetAlpha(BYTE a,BYTE mode);			// αセット

FVOID _2DLine(int x1,int y1,int x2,int y2);		// 直線
FVOID _2DBox(int x1,int y1,int x2,int y2);		// 長方形
FVOID _2DCircle(int x,int y,int r);				// 円
FVOID _2DCircleF(int x,int y,int r);			// 円塗りつぶし
FVOID _2DPolygon(POINT *p,int n);				// ポリゴン塗りつぶし

FVOID _2DBoxA(int x1,int y1,int x2,int y2);		// α長方形
FVOID _2DCircleA(int x,int y,int r);			// α円塗りつぶし
FVOID _2DPolygonA(POINT *p,int n);				// αポリゴン塗りつぶし

FVOID _2DGamma(BYTE r,BYTE g,BYTE b,BYTE a);	// ガンマコントロール

FVOID DisableAlphaTable(void);



// インライン関数 //
IBOOL _2DChkPoint(int x,int y)
{
	extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;

	if(x<CLIP_X_MIN||x>=CLIP_X_MAX||y<CLIP_Y_MIN||y>=CLIP_Y_MAX) return FALSE;

	return TRUE;
}

IVOID _2DPset(int x,int y,BYTE c)
{
	extern BYTE		*Grp_Vram;
	extern DWORD	Grp_AddrY[480];

	if(_2DChkPoint(x,y))
		*(Grp_Vram+x+Grp_AddrY[y]) = c;
}

// 正方形クリップ : ReturnValue TRUE:クリッピング成功 FALSE:完全に範囲外
IBOOL _2DClipRect(int *x1,int *y1,int *x2,int *y2)
{
	extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;

	// 完全判定 //
	if((*x1)>CLIP_X_MAX || (*x2)<CLIP_X_MIN) return FALSE;
	if((*y1)>CLIP_Y_MAX || (*y2)<CLIP_Y_MIN) return FALSE;

	// クリッピング //
	if((*x1)<CLIP_X_MIN) (*x1)=CLIP_X_MIN;
	if((*y1)<CLIP_Y_MIN) (*y1)=CLIP_Y_MIN;
	if((*x2)>CLIP_X_MAX) (*x2)=CLIP_X_MAX;
	if((*y2)>CLIP_Y_MAX) (*y2)=CLIP_Y_MAX;

	return TRUE;
}

#endif

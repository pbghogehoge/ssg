/*                                                                           */
/*   Lens.h   レンズエフェクト                                               */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_LENS_H
#define PBGWIN_LENS_H		"LENS_EFC : Version 0.01 : Update 2000/09/18"
#pragma message(PBGWIN_LENS_H)

#include "PBGUTY_X.h"



///// [構造体] /////

// レンズデータ定義用構造体 //
typedef struct tagLensInfo {
	DWORD		r;			// レンズの半径
	DWORD		Height;		// レンズの直径
	DWORD		*Data;		// レンズ置換テーブル

	GRP			gTemp;		// データ一時待避用Surface(できればSysMemory)
	DWORD		gPitch;		// Surface の横幅
	//BYTE		gID;		// SurfaceID (DD_UTY の関数に対する)
} LensInfo;



///// [ 関数 ] /////

// 半径:r  出っ張り:m  のレンズを作成 //
extern LensInfo *GrpCreateLensBall(int r, int m, GRP id);

// GrpLock() 系関数 : レンズボールを描画する //
extern FVOID GrpDrawLensBall(LensInfo *LInfo, int x, int y);

// レンズボール用データを破棄する //
extern FVOID GrpReleaseLensBall(LensInfo *LInfo);



#endif

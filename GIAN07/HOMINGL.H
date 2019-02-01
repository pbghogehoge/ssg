/*                                                                           */
/*   HomingL.h   長いレーザーの処理                                          */
/*                                                                           */
/*                                                                           */


#ifndef PBGWIN_HOMINGL_H
#define PBGWIN_HOMINGL_H		"HOMINGL : Ver 0.01 : Updata 2000/09/04"
//#pragma message(PBGWIN_HOMINGL_H)

#include "GIAN.h"



///// [ 定数 ] /////
#define HLASER_MAX			162
#define HLASER_LEN			7		// 描画枚数..
#define HLASER_SECTION		4		// 読み込み幅


#define HL_NONE		0			// ただ進むだけ
#define HL_TYPE1	1			// その１

#define HLS_NORM	0x00		// ホーミングレーザー通常
#define HLS_CLEAR	0x01		// ホーミングレーザー消去中
#define HLS_DEAD	0xff		// ホーミングレーザー削除要請



///// [構造体] /////

// ホーミングレーザー //
typedef struct tagHLaserData {
	int		Current;	// 現在の先頭
	int		v;			// 速度
	int		a;			// 加速度

	DWORD	Count;		// フレームカウンタ

	BYTE	Type;		// 種類(加速＆ホーミングタイプ)
	BYTE	State;		// 状態
	BYTE	c;			// 色
	BYTE	Left;		// 残りホーミング回数

	struct tagHLaserData	*Next;				// 次のレーザーへのポインタ
	DegPoint	p[HLASER_LEN*HLASER_SECTION];	// 頂点キュー(ExDef.h)
} HLaserData;

// ホーミングレーザーセット情報 //
typedef struct tagHLaserInfo {
	int		x,y;		// 中心座標

	BYTE	d;			// 角度
	BYTE	dw;			// 角度の開き
	BYTE	n;			// 本数

	BYTE	c;			// 色
	BYTE	type;		// 種類
} HLaserInfo;



///// [グローバル変数] /////
extern WORD			HLaserNow;			// ホーミングレーザーの本数
extern HLaserInfo	HLaserCmd;			// ホーミングレーザーセット用データ


///// [関数プロトタイプ] /////
FVOID HLaserInit(void);				// ホーミングレーザーの初期化を行う
FVOID HLaserSet(HLaserInfo *hinfo);	// ホーミングレーザーをセットする
FVOID HLaserMove(void);				// ホーミングレーザーを動作させる
FVOID HLaserDraw(void);				// ホーミングレーザーを描画する
FVOID HLaserClear(void);			// ホーミングレーザーに消去エフェクトをセット



#endif

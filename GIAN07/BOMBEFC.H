/*
 *   BombEfc.h   : 爆発系エフェクト
 *
 */

#ifndef PBG_BOMBEFC_VERSION
#define PBG_BOMBEFC_VERSION	"爆発系エフェクト : Version 0.01 : Update 2000/11/21"

#include "Gian.h"



/***** [ 定数 ] *****/
#define EXBOMB_MAX			3			// エフェクトの同時発生数
#define EXBOMB_STD			0			// よくあるタイプの爆発(??)
#define EXBOMB_OBJMAX		200			// エフェクト補助用オブジェクトの個数



/***** [構造体] *****/
typedef struct tagSpObj {
	int			x,y;
	int			vx,vy;
	BYTE		d;
} SpObj;

typedef struct tagBombEfcCtrl {
	int			x,y;					// エフェクトの中心座標
	BOOL		bIsUsed;				// この構造体は使用中か
	DWORD		count;					// フレームカウンタ

	SpObj		Obj[EXBOMB_OBJMAX];		// エフェクト補助用オブジェクト

	BYTE		type;					// エフェクトの種類
} BombEfcCtrl;



/***** [関数プロトタイプ] *****/
FVOID ExBombEfcInit(void);						// 爆発系エフェクトの初期化
FVOID ExBombEfcSet(int x, int y, BYTE type);	// 爆発系エフェクトをセットする
FVOID ExBombEfcDraw(void);						// 爆発系エフェクトを描画する
FVOID ExBombEfcMove(void);						// 爆発系エフェクトを動作させる



#endif

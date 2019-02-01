/*                                                                           */
/*   PRankCtrl.h   プレイランク管理                                          */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_PRANKCTRL_H
#define PBGWIN_PRANKCTRL_H		"PRANKCTRL : Version 0.01 : Update 2000/09/13"

#include "GIAN.h"



///// [構造体] /////
typedef struct tagPlayRankInfo{
	BYTE	GameLevel;		// 方向数も関係する難易度変化
	int		Rank;			// 弾の速度変化に関する値
} PlayRankInfo;



///// [グローバル変数] /////
extern PlayRankInfo		PlayRank;



///// [ 関数 ] /////
FVOID PlayRankAdd(int n);		// 難易度の許容範囲内でプレイランクを増減する
FVOID PlayRankReset(void);		// 現在の難易度に応じてプレイランクを初期化



#endif

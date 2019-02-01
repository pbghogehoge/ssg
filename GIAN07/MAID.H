/*                                                                           */
/*   Maid.h   メイドさん関連の処理                                           */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_MAID_H
#define PBGWIN_MAID_H		"めいど : Version 0.01 : Update 2000/02/17"
//#pragma message(PBGWIN_MAID_H)

#include "Gian.h"



///// [ 定数 ] /////

// サボテン定数 //
#define VIVDEAD_VAL		(300)		// びびっとの死亡時間．．．
#define VIVMUTEKI_VAL	(180)			// びびっとの無敵時間

#define MAID_MOVE_DISABLE_TIME		(250-100)		// 行動不能な時間

#define BOMBMUTEKI_VAL	60			// ボムの終端無敵時間
#define SBOPT_DX		26			// オプションのずれ幅(x64ではない)
//#define EVADE_X_START	150			// かすりカウンタの初期表示座標
//#define EVADE_X_SPD		5			// かすりカウンタの移動速度
//#define EVADE_TIME		60			// かすり許容フレーム数
#define EVADETIME_MAX	256			// かすりマックス時の待ち時間

#define SSP_WIDE		(64*9)		//
#define SSP_HOMING		(64*9)		//
#define SSP_LASER		(64*13)		//




///// [マクロ] /////



///// [構造体] /////

typedef struct
{
	int x,y;			// 現在の<表示>座標

	int vx,vy;			// オプションのズレ具合
	int opx,opy;		// 現在のオプション基本座標

	__int64 score;		// 得点カウンタ
	__int64	dscore;		// 得点増加値

	DWORD	evade_sum;		// かすり合計
	int		evadesc;		// かすり得点
	WORD	evade;			// かすり回数
	WORD	evade_c;		// 連続「かすり」の残り許容時間

	char v;				// サボテンの移動速度基本値(後で64~45倍にする)

	BYTE weapon;		// "とげ" の種類
	BYTE exp;			// サボテンの経験値？
	BYTE bomb;			// ボムの数
	BYTE left;			// 残りサボテン数
	BYTE credit;		// のこりクレジット

	BYTE GrpID;			// 表示すべきグラフィック

	WORD bomb_time;		// ボムウェイト用
	WORD exp2;			// 経験値増加抑制用
	WORD muteki;		// 無敵フラグ(0:off !0:無敵時間カウンタ)
	WORD lay_time;		// レーザーの発射タイミング用
	BYTE lay_grp;		// レーザーのグラフィック用
	BYTE toge_time;		// "とげ" の発射タイミング用
	BYTE toge_ex;		// とげ発射用特殊変数
	BYTE ShiftCounter;	// 押しっぱなし低速移動用

	BOOL bGameOver;		// ゲームオーバー判定用フラグ
	BOOL BuzzSound;		// かすった音を連続再生させないためのフラグ
} MAID;



///// [ 関数 ] /////
extern void MaidDraw(void);
extern void StateDraw(void);		// 各種ステータスを描画する
extern void MaidMove(void);
extern void MaidSet(void);			// 初期化
extern void MaidNextStage(void);	// 次のステージの準備
extern void MaidDead(void);			// 死す

extern FVOID evade_add(BYTE n);						// かすりゲージを上昇させる
//extern FVOID evade_addEx(int x,int y,BYTE n);		// かすりゲージを上昇させる

extern void score_add(int sc);		// スコアを上昇させる

extern void WideBombDraw(void);		// ワイドショット用のボム(やや例外処理)



///// [ 変数 ] /////
extern MAID			Viv;					// 麗しきメイドさん構造体
extern DWORD		EvadeRate[256];			// かすり得点レート



IVOID PowerUp(BYTE damage)
{
	// ダメージの分だけ加算する //
	Viv.exp2 += damage;

	// Viv.exp(8bit+1bit) ooo oooooo //
	switch(((WORD)Viv.exp+1)>>5){
		case(0):	if(Viv.exp2>5-3)		Viv.exp++,	Viv.exp2=0;	return;
		case(1):	if(Viv.exp2>25-15)		Viv.exp++,	Viv.exp2=0;	return;
		case(2):	if(Viv.exp2>50-20)		Viv.exp++,	Viv.exp2=0;	return;
		case(3):	if(Viv.exp2>80)		Viv.exp++,	Viv.exp2=0;	return;
		case(4):	if(Viv.exp2>120)	Viv.exp++,	Viv.exp2=0;	return;
		case(5):	if(Viv.exp2>140)	Viv.exp++,	Viv.exp2=0;	return;
		case(6):	if(Viv.exp2>160)	Viv.exp++,	Viv.exp2=0;	return;
		case(7):	if(Viv.exp2>180)	Viv.exp++,	Viv.exp2=0;	return;
		case(8):	return;		// フルパワーアップ時
	}
}

IBYTE GetRightLaserDeg(BYTE LaserDeg, int i)
{
	if(LaserDeg < 58) return 64+48 - LaserDeg*3 + i*(64-LaserDeg)/2;
	else              return 64+48 - 58*3 + i*(64-min(62,LaserDeg))/2;
}

IBYTE GetLeftLaserDeg(BYTE LaserDeg, int i)
{
	if(LaserDeg < 58) return 64-48 + LaserDeg*3 + i*(64-LaserDeg)/2;
	else              return 64-48 + 58*3 + i*(64-min(62,LaserDeg))/2;
}

IBYTE GetLaserDeg(void)
{
	return ((120-Viv.bomb_time)*3)/2;
}


#endif

/*                                                                           */
/*   LLaser.h   長いレーザーの処理                                           */
/*                                                                           */
/*                                                                           */


#ifndef PBGWIN_LLASER_H
#define PBGWIN_LLASER_H		"LLASER : Ver 0.03 : Updata 2000/02/07"
//#pragma message(PBGWIN_LLASER_H)

// 更新履歴 //
// 2000/05/29 : ８ビットモード描画関連のＢｕｇＦｉｘ
// 2000/03/22 : レーザー関数のＩＤの意味がレーザー配列のＩＤからその敵が
//            : 発射しているレーザーの何番目か、に変更された


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ENEMY.h"



//// レーザー用定数２ ////
#define LLASER_MAX		20
#define LLASER_EVADE	1		// レーザーのかすり値


//// レーザーの種類定数２
#define LLS_LONG		0x00
#define LLS_LONGY		0x01
#define LLS_SETDEG		0x02
#define LLS_LONGZ		0x03	// 自機セット


//// レーザーフラグ２ ////
#define LLF_DISABLE		0x00			// レーザーが使用されていない
#define LLF_NORM		0x01			// レーザーが完全に開ききった
#define LLF_OPEN		0x02			// レーザを開いている
#define LLF_CLOSE		0x04			// レーザーを閉じている
#define LLF_CLOSEL		0x08			// レーザーをライン状態にする
#define LLF_LINE		0x10			// レーザーはライン状態



//// レーザー発動コマンド構造体２ ////
typedef struct{
	ENEMY_DATA	*e;		// 敵データへのポインタ

	int		dx,dy;		// レーザーの発射座標ずらし値
	int		v;			// レーザーの速度

	int		w;			// レーザーの太さ最終値

	BYTE	d;			// レーザー発射角

	BYTE	c;			// レーザーの色
	BYTE	type;		// レーザーの種類
} LLASER_CMD;


//// レーザー用構造体２ ////
typedef struct{
	ENEMY_DATA	*e;			// 敵データへのポインタ(ここら辺でボスでも雑魚でも発射できるように)

	int		x,y;			// 現在の表示座標
	int		dx,dy;	 		// 敵データからのずらし値(x64)
	int		lx,ly;			// レーザー円の中心座標までのベクトル(Grp)
	int		infx,infy;		// 仮の無限遠へのベクトル(Grp)
	int		wx,wy;			// レーザー幅(Grp)

	int		w,wmax;			// 幅、最大幅(x64)
	int		v;

	DWORD	count;			// フレームカウンタ

	POINT	p[4];			// 座標保持用のポインタ(Grp)

	BYTE	d;				// レーザーの発射角
	BYTE	c;				// レーザーの色

	BYTE	flag;			// レーザーの状態
	BYTE	type;			// レーザーの種類
	BYTE	EnemyID;		// 敵から見た番号
} LLASER_DATA;



//// レーザー関数２ ////
extern BOOL LLaserSet(BYTE id);							// レーザーをセットする
extern void LLaserOpen(ENEMY_DATA *e,BYTE id);			// レーザーを開く
extern void LLaserClose(ENEMY_DATA *e,BYTE id);			// レーザーを閉じる
extern void LLaserLine(ENEMY_DATA *e,BYTE id);			// レーザーをライン状態にする
extern void LLaserDegA(ENEMY_DATA *e,BYTE d,BYTE id);	// レーザーを角度絶対で回転
extern void LLaserDegR(ENEMY_DATA *e,char d,BYTE id);	// レーザーを角度相対で回転

extern void LLaserForceClose(ENEMY_DATA *e);	// 敵に関連づけられたレーザーを強制クローズ

extern void LLaserMove(void);				// レーザーを動かす
extern void LLaserDraw(void);				// レーザーを描画する
extern void LLaserClear(void);				// 無限遠レーザーを全クローズ

extern void LLaserSetup(void);				// レーザー配列の初期化をする



//// レーザー変数２ ////
extern LLASER_DATA	LLaser[LLASER_MAX];
extern LLASER_CMD	LLaserCmd;



#endif

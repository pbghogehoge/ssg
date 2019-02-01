/*                                                                           */
/*   GIAN.h   ゲーム全体の管理                                               */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_GIAN_H
#define PBGWIN_GIAN_H		"GIAN : Version 0.62 : Update 2000/02/23"
//#pragma message(PBGWIN_GIAN_H)



///// [更新履歴] /////

// 2000/02/23 : GIAN06 同等の性能をもつようになった。
// 2000/02/09 : 大幅な変更



//#define PBG_DEBUG		// デバッグモードを可能にする時に定義する



///// [Include Files] /////
#include "PBGUTY_X.h"			// DirectX & その他の関数統合
#include "ExDef.h"				// 色々な定義
#include "WINDOWSYS.h"			// ウィンドウシステム本体
#include "WINDOWCTRL.h"			// ウィンドウ定義
#include "FONTUTY.h"			// フォント管理(GrpPut系,CrateFont系)

#include "ECL.h"				// ＥＣＬ定義ファイル
#include "SCL.h"				// ＳＣＬ定義ファイル
#include "ENEMY.h"				// 敵の移動処理など
#include "BOSS.h"				// ボスの定義＆ボス用エフェクトなど
//#include "EnemyExCtrl.h"		// 特殊動作処理

#include "PrankCtrl.h"			// プレイランク管理
#include "GAMEMAIN.h"			// メインのルーチン切り替え処理
#include "EFFECT.h"				// 主にテキストベースのエフェクト処理
#include "EFFECT3D.h"			// ３Ｄエフェクト
#include "FRAGMENT.h"			// 破片系エフェクト処理
#include "BOMBEFC.h"			// 爆発エフェクト処理
#include "TAMA.h"				// 敵弾処理
#include "LASER.h"				// 短いレーザー＆リフレクトレーザー処理
#include "LLASER.h"				// なが～いレーザーの処理
#include "HOMINGL.h"			// ホーミングレーザーの処理
#include "MAIDTAMA.h"			// 自機ショットの処理
#include "MAID.h"				// その名の通り
#include "SCROLL.h"				// 背景スクロール＆ＳＣＬ管理
#include "LOADER.h"				// 各種ローダー

#include "Item.h"				// アイテム処理

#include "Score.h"


///// [ 定数 ] /////

// ステージ関連 //
#define STAGE_MAX		6			// ステージ数



// 座標関連 //
#define X_MIN		(128)						// 表示Ｘ座標最小値
#define X_MAX		(511)						// 表示Ｘ座標最大値
#define X_MID		((X_MAX+X_MIN)/2)
#define Y_MIN		(0)							// 表示Ｙ座標最小値
#define Y_MAX		(479)						// 表示Ｙ座標最大値
#define Y_MID		((Y_MAX+Y_MIN)/2)

#define X_RNDV		(-30000)				// Ｘ座標のランダム指定用
#define Y_RNDV		(-30000)				// Ｙ座標のランダム指定用

#define GX_MIN		(X_MIN * 64)			// ゲーム座標上におけるＸ座標最小値
#define GX_MAX		(X_MAX * 64)			// ゲーム座標上におけるＸ座標最大値
#define GX_MID		((GX_MAX+GX_MIN)/2)		// ゲーム座標上におけるＸ座標中心値
#define GY_MIN		(Y_MIN * 64)			// ゲーム座標上におけるＹ座標最小値
#define GY_MAX		(Y_MAX * 64)			// ゲーム座標上におけるＹ座標最大値
#define GY_MID		((GY_MAX+GY_MIN)/2)		// ゲーム座標上におけるＹ座標中心値

#define SX_WID		(64*10)						// サボテンのＸ幅？
#define SY_WID		(64*10)						// サボテンのＹ幅？
#define SX_MIN		(GX_MIN + SX_WID)			// サボテンのＸ座標最小値
#define SX_MAX		(GX_MAX - SX_WID)			// サボテンのＸ座標最大値
#define SY_MIN		(GY_MIN + SY_WID + 30*64)	// サボテンのＹ座標最小値
#define SY_MAX		(GY_MAX - SY_WID)			// サボテンのＹ座標最大値
#define SX_START	(GX_MID)					// サボテンの開始Ｘ座標
#define SY_START	(GY_MAX+180*64 /*- 50*64*/)			// サボテンの開始Ｙ座標

#define RL_WIDX		(32-4)					// 反射レーザー用_Ｘ座標_補正値
#define RL_WIDY		16						// 反射レーザー用_Ｙ座標_補正値
#define RLX_MIN		(GX_MIN/64+RL_WIDX)		// 反射レーザーの反射Ｘ座標最小値
#define RLX_MAX		(GX_MAX/64-RL_WIDX)		// 反射レーザーの反射Ｘ座標最大値
#define RLY_MIN		(GY_MIN/64+RL_WIDY)		// 反射レーザーの反射Ｙ座標最小値
#define RLY_MAX		(GX_MAX/64-RL_WIDY)		// 反射レーザーの反射Ｙ座標最大値

#define NREG_SX		(X_MID-13*9)			// ネームレジスト用ウィンドウの開始Ｘ
#define NREG_SY		(Y_MID+100)				// ネームレジスト用ウィンドウの開始Ｙ
#define NREGI_X		(X_MID-8*7)				// ネームレジスト用ウィンドウ(名前表示部)の開始Ｘ
#define NREGI_Y		(Y_MID+60)				// ネームレジスト用ウィンドウ(名前表示部)の開始Ｙ

#define STG_RNDXY	0				// 配置する座標がランダムの時の値...(なんだかよう分からん)


// 難易度 ＆ オプション設定 //
#define GAME_EASY		0			// 難易度：Ｅａｓｙ
#define GAME_NORMAL		1			// 難易度：Ｎｏｒｍａｌ
#define GAME_HARD		2			// 難易度：Ｈａｒｄ
#define GAME_LUNATIC	3			// 難易度：Ｌｕｎａｔｉｃ
#define GAME_EXTRA		4			// Ｅｘｔｒａ時...


// スコア //
#define SCORE_NAME		9			// スコア用の最大文字列長(NULL含む)


// フラグ //
#define GRPF_MASK				(~0x07)
#define GRPF_ALPHA_ENABLE		0x01		// 半透明が有効(無効時はメッシュ???)
#define GRPF_WINDOW_UPPER		0x02		// メッセージウィンドウを上の方に表示する
#define GRPF_MSG_DISABLE		0x04		// 会話シーンを省略

#define SNDF_MASK				(~0x03)
#define SNDF_MIDI_ENABLE		0x01		// ＭＩＤＩが有効
#define SNDF_WAVE_ENABLE		0x02		// ＷＡＶＥが有効

#define INPF_MASK				(~0x07)
#define INPF_JOYPAD_ENABLE		0x01		// パッドが有効
#define INPF_Z_MSKIP_ENABLE		0x02		// Ｚキーでメッセージを送れる
#define INPF_Z_SPDDOWN_ENABLE	0x04		// 押しっぱなしでシフト移動

#define DBGF_MASK				(~0x03)
#define DBGF_DISPLAYINFO		0x01		// デバッグ情報を出力する
#define DBGF_HIT_ON				0x02		// 当たり判定が有効



///// [マクロ] /////
#define SPEEDM(v)	((short)(((v)&0x3f)<<4))	// 速度セット用まくろ(BYTE->int?)
#define WAVESP(v)	((short)((v)<<4))			// WAVE?用の速度セット(まずいな)

#define GX_RND	((X_MIN+rnd()%(X_MAX-X_MIN))<<6)			// Ｘ座標ランダム
#define GY_RND	((Y_MIN+rnd()%(Y_MAX-Y_MIN))<<6)			// Ｙ座標ランダム

//#define HITCHK(a,b,h)	( ((DWORD)(a)-(b)+(h))<h*2 )		// ヒットなら非ゼロ
#define HITCHK(a,b,h)	(abs((a)-(b))<(h))


///// [構造体] /////

// オプション保持用構造体 //
typedef struct tagCONFIG_DATA{
	BYTE	GameLevel;		// 難易度
	BYTE	PlayerStock;	// 初期メイド数？
	BYTE	BombStock;		// 初期ボム数

	BYTE	DeviceID;		// 何番目のデバイスか
	BYTE	BitDepth;		// ビット深度
	BYTE	DrawMode;		// 描画スキップの設定

	BYTE	GraphFlags;		// グラフィックに関するフラグ
	BYTE	SoundFlags;		// サウンド・ＭＩＤＩ関連のフラグ
	BYTE	InputFlags;		// 入力に関するフラグ

	BYTE	DebugFlags;		// デバッグに関するフラグ

	BYTE	PadTama;
	BYTE	PadBomb;
	BYTE	PadShift;
	BYTE	PadCancel;

	BYTE	ExtraStgFlags;	// エキストラステージ判定用フラグ
	DWORD	CheckSum;

	BYTE	StageSelect;	// ワケ有りでここにいるのです(チェックサムの範囲外)
} CONFIG_DATA;

#ifdef PBG_DEBUG
// デバッグ情報管理用構造体 //
typedef struct tagDEBUG_DATA{
	BOOL	MsgDisplay;		// デバッグ情報を出力するか
	BOOL	Hit;			// 当たり判定の有・無
	BOOL	DemoSave;		// デモプレイをセーブするか

	BYTE	StgSelect;		// ステージセレクト(開始ステージ)
} DEBUG_DATA;
#endif


/*
// スコア管理用構造体 //
typedef struct tagSCORE_DATA{
	__int64	score;
	BYTE	weapon;
	char	name[SCORE_NAME];
} SCORE_DATA;

typedef struct tagHIGH_SCORE{
	SCORE_DATA		easy[8];
	SCORE_DATA		normal[8];
	SCORE_DATA		hard[8];
	SCORE_DATA		lunatic[8];
} HIGH_SCORE;
*/


///// [グローバル変数] /////
extern CONFIG_DATA	ConfigDat;
//extern HIGH_SCORE	*HighScore;
//extern char			ScoreTable[8][80];
extern DWORD		GameCount;
extern BYTE			GameStage;

#ifdef PBG_DEBUG
extern DEBUG_DATA	DebugDat;
#endif


///// [関数] /////
extern void ConfigLoad(void);		// コンフィグの内容をロードする
extern void ConfigSave(void);		// コンフィグの内容を保存する

#ifdef PBG_DEBUG
extern FVOID OutputDebugMsg(void);	// デバッグ情報を出力する
#endif

extern FVOID StdStatusOutput(void);
extern FVOID swap16(WORD *a,WORD *b);



_inline void BltSetRect(RECT *rc,int x1,int y1,int x2,int y2)
{
	rc->right  = (rc->left = x1) + x2;
	rc->bottom = (rc->top  = y1) + y2;
}


//// 諸事情につき... ////
#include "DemoPlay.h"			// デモプレイ処理


#endif

/*                                                                           */
/*   SCROLL.h   スクロール処理                                               */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_SCROLL_H
#define PBGWIN_SCROLL_H		"SCROLL : Version 0.11 : Update 2000/02/28"
//#pragma message(PBGWIN_SCROLL_H)

#include "Gian.h"


///// [更新履歴] /////
 // 2000/04/01 : スクロールコマンドを追加(STAGE2_BOSS)
 // 2000/02/28 : ＢＯＳＳ命令を Level2 に変更
 // 2000/02/24 : みぢ命令を追加
 // 2000/02/18 : 敵配置処理と結合する
 // 2000/02/01 : 開発開始
 //



///// [ 定数 ] /////

// マップエディタと共有する定数 //
#define LAYER_MAX			5				// レイヤーの深さ
#define TIME_PER_FRAME		20				// １フレームでどれだけ時間が進むか
#define MAP_WIDTH			24				// マップの横幅
#define MAPDATA_NONE		0xffff			// マップ上に何も置かれていない状態

// スクロールステータス //
#define SST_NORMAL			0x00			// 通常時
#define SST_STOP			0x01			// 停止中

// スクロールコマンド //
#define SCMD_QUAKE			0x01			// 画面を振動させる
#define SCMD_STG2BOSS		0x02			// ２面ボスの反転スクロール
#define SCMD_RASTER_ON		0x03			// ラスタースクロール開始
#define SCMD_RASTER_OFF		0x04			// ラスタースクロー終了
#define SCMD_STG3BOSS		0x05			// 背景雲をゲイツモード(謎)に変更する
#define SCMD_STG3RESET		0x06			// ３面の背景をノーマルモードに戻す
#define SCMD_STG6CUBE		0x07			// ６面ボスの３Ｄキューブモード
#define SCMD_STG6RNDECL		0x08			// ６面ボスのランダム偽ＥＣＬ列配置
#define SCMD_STG4ROCK		0x09			// ４面岩
#define SCMD_STG4LEAVE		0x0a			// ４面岩を画面外に吐き出す
#define SCMD_STG6RASTER		0x0b			// ６面ラスター
#define SCMD_STG3STAR		0x0c			// ３面高速星



///// [  型  ] /////
typedef WORD	PBGMAP;		// マップパーツ格納用の型



///// [マクロ] /////



///// [構造体] /////

// スクロール管理用構造体 //
typedef struct tagScrollInfo{
//	GRP		lpMapOffs;					// マップパーツデータ(Graphic)の格納先

	BYTE	*DataHead;					// マップデータのヘッダ

	PBGMAP	*LayerHead[LAYER_MAX];		// 各レイヤーのヘッダ
	PBGMAP	*LayerPtr[LAYER_MAX];		// 現在のレイヤーのポインタ
	DWORD	LayerWait[LAYER_MAX];		// レイヤーの重み
	int		LayerCount[LAYER_MAX];		// レイヤーごとのカウンタ
	BYTE	LayerDy[LAYER_MAX];			// レイヤーの1Dot単位のズレ

	int		NumLayer;			// レイヤー数
	int		ScrollSpeed;		// スクロール速度
	DWORD	Count;				// 現在の時刻
	DWORD	InfStart;			// 無限ループの開始時刻(デフォルトは０)
	DWORD	InfEnd;				// 無限ループの終了時刻(デフォルトはマップの長さの最小値)

	BYTE	State;				// スクロールステータス
	BYTE	IsQuake;			// 振動中か？

	char	RasterDx[31];		// ラスタースクロールによるＸ加算値
	BYTE	RasterWidth;		// ラスタースクロールにおける振幅
	BYTE	RasterDeg;			// ラスタースクロール用の角度

	FVOIDFN(*ExCmd)(void);		// 特殊コマンド
	DWORD	ExCount;			// 特殊コマンド用カウンタ
} SCROLL_INFO;

// マップデータ保存用ヘッダ //
typedef struct tagScrollSaveHeader{
	DWORD		Address;				// このデータの開始アドレス
	DWORD		ScrollWait;				// このレイヤーのディレイ
	DWORD		Length;					// このレイヤーの長さ
} ScrollSaveHeader;

// ＳＣＬ管理用構造体 //
typedef struct tagSCL_INFO{
	BOOL	MsgFlag;			// メッセージスキップ用フラグ
	BOOL	ReturnFlag;			// リターンキー用フラグ
} SCL_INFO;


///// [ 関数 ] /////
FVOID ScrollMove(void);				// 背景を動かす(１フレーム分)
FVOID ScrollDraw(void);				// 背景を描画する

FVOID ScrollSpeed(int speed);		// スクロールスピードを変更する(引数:(1)スクロール速度)
FVOID ScrollCommand(BYTE cmd);		// ＳＣＬ用コマンド実行関数(引数:(1)スクロールコマンド)

FBOOL ScrollInit(void);				// マップデータを初期化する



///// [ 変数 ] /////
extern SCROLL_INFO		ScrollInfo;			// スクロールに関する情報
extern SCL_INFO			SclInfo;			// ＳＣＬに関する情報



#endif

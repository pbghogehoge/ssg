/*                                                                           */
/*   DemoPlay.h   デモプレイ処理                                             */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DEMOPLAY_H
#define PBGWIN_DEMOPLAY_H		"DEMOPLAY : Version 0.02 : Update 2000/04/01"
//#pragma message(PBGWIN_DEMOPLAY_H)

#include "Gian.h"


///// [ 定数 ] /////
#define DEMOBUF_MAX		(60*60*30)		// ２０分のデータ格納ＯＫ



///// [構造体] /////
typedef struct tagDEMOPLAY_INFO{
	DWORD			RndSeed;		// 乱数のたね
	DWORD			DataSize;		// データ部のサイズ(デコード時に使用する)
	CONFIG_DATA		CfgDat;			// コンフィグの情報(Load時に一部を参照する)
	BYTE			Exp;			// 初期パワーアップ
	BYTE			Weapon;			// 初期装備
} DEMOPLAY_INFO;


///// [ 関数 ] /////
FVOID DemoplayInit(void);			// デモプレイデータの準備
FVOID DemoplayRecord(WORD key);		// デモプレイデータを保存する
FVOID DemoplaySave(void);			// デモプレイデータをファイルに書き込む

FBOOL DemoplayLoad(int stage);		// デモプレイデータをロードする
FWORD DemoplayMove(void);			// Key_Data を返す
FVOID DemoplayLoadCleanup(void);		// デモプレイロードの事後処理


// 要望に付き、追加するのだ //
FVOID ReplayInit(void);			// リプレイデータの準備
FVOID ReplayRecord(WORD key);	// リプレイデータを保存する
FVOID ReplaySave(void);			// リプレイデータをファイルに書き込む

FBOOL ReplayLoad(int stage);	// リプレイデータをロードする
FWORD ReplayMove(void);			// Key_Data を返す
FVOID ReplayLoadCleanup(void);	// リプレイデータロードの事後処理



///// [ 変数 ] /////
extern BOOL DemoplaySaveEnable;		// デモプレイのセーブが動作しているか
extern BOOL DemoplayLoadEnable;		// デモプレイのロードが動作しているか


#endif

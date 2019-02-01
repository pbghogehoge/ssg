/*                                                                           */
/*   GameMain.h   ウィンドウシステム切り替えなどの処理                       */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_GAMEMAIN_H
#define PBGWIN_GAMEMAIN_H		"GAMEMAIN : Version 0.01 : Update 2000/02/23"
//#pragma message(PBGWIN_GAMEMAIN_H)



///// [更新履歴] /////

// 2000/02/03 : 製作開始



///// [Include Files] /////
#include "Gian.h"
#include "Ending.h"



///// [ 定数 ] /////
///// [マクロ] /////
///// [構造体] /////

///// [グローバル変数] /////
extern BOOL IsDemoplay;



///// [関数] /////
extern FVOIDFN(*GameMain)(void);		// ゲーム進行用関数ポインタ(WinMainからコールする)

extern FBOOL GameInit(void);			// ゲームの初期化をする
extern FVOID GameRestart(void);			// ゲームを再開する(ESC 抜けから)
extern FBOOL GameExit(BOOL bNeedChgMusic = TRUE);	// ゲームから抜ける
extern FVOID GameOverInit(void);		// ゲームオーバーの前処理
extern FVOID GameContinue(void);		// コンティニューを行う場合

extern FBOOL GameReplayInit(int Stage);	// リプレイ用の初期化を行う

extern FBOOL SProjectInit(void);		// 西方Ｐｒｏｊｅｃｔ表示の初期化

extern FBOOL GameExstgInit(void);		// エキストラステージを始める

extern FBOOL NameRegistInit(BOOL bNeedChgMusic);	// お名前入力の初期化
extern FBOOL ScoreNameInit(void);		// お名前表示画面

extern FBOOL GameNextStage(void);		// 次のステージに移行する

extern FBOOL MusicRoomInit(void);		// 音楽室の初期化をする

#endif

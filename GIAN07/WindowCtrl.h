/*                                                                           */
/*   WINDOWCTRL.h   ウィンドウの定義＆管理                                   */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_WINDOWCTRL_H
#define PBGWIN_WINDOWCTRL_H		"WINDOWCTRL : Version 0.01 : Update 2000/02/12"
//#pragma message(PBGWIN_WINDOWCTRL_H)



///// [更新履歴] /////

//



///// [Include Files] /////
#include "Gian.h"



///// [ 定数 ] /////
///// [マクロ] /////
///// [構造体] /////

///// [グローバル変数] /////
extern struct tagWINDOW_SYSTEM MainWindow;
extern struct tagWINDOW_SYSTEM ExitWindow;
extern struct tagWINDOW_SYSTEM ContinueWindow;


///// [関数] /////
void InitMainWindow(void);			// メインメニューの初期化
void InitExitWindow(void);			// 終了Ｙ／Ｎウィンドウの初期化
void InitContinueWindow(void);		// コンティニューＹ／Ｎウィンドウの初期化



#endif

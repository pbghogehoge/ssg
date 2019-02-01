/*                                                                           */
/*   WINDOWSYS.h   コマンドウィンドウ処理                                    */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_WINDOWSYS_H
#define PBGWIN_WINDOWSYS_H		"WINDOWSYS : Version 0.24 : Update 2000/02/28"

#include "GIAN.h"



///// [更新履歴] /////
/*
 * 2000/07/26 : 操作性を改善した
 *
 * 2000/02/28 : メッセージウィンドウの半透明部を高速化
 * 2000/02/10 : PWINDOW->WINDOWSYS
 *            : WINDOWSYS において、コマンドウィンドウは、スタックで選択状態を保持する。
 *            : ウィンドウを動かしたり枠を付けたりするのはあとでね...
 *
 * 2000/01/31 : 開発はじめ
 *
 */



// 最大数に関する定数 //
#define WINITEM_MAX		20			// 項目の最大数
#define WINDOW_DEPTH	10			// ウィンドウの深さ
#define MESSAGE_MAX		100			// 一行に表示できる最大メッセージ長
#define MSG_HEIGHT		5			// メッセージウィンドウの高さ

// コマンドウィンドウの状態 //
#define CWIN_DEAD		0x00		// 使用されていない
#define CWIN_FREE		0x01		// 入力待ち状態
#define CWIN_OPEN		0x02		// 項目移動中(進む)
#define CWIN_CLOSE		0x03		// 項目移動中(戻る)
#define CWIN_NEXT		0x04		// 次のウィンドウに移行中
#define CWIN_BEFORE		0x05		// 前のウィンドウに移行中
#define CWIN_INIT		0xff		// 初期化処理中

// メッセージウィンドウ・コマンド //
#define MWCMD_SMALLFONT		0x00		// スモールフォントを使用する
#define MWCMD_NORMALFONT	0x01		// ノーマルフォントを使用する
#define MWCMD_LARGEFONT		0x02		// ラージフォントを使用する
#define MWCMD_NEWPAGE		0x03		// 改ページする

// メッセージウィンドウの状態 //
#define MWIN_DEAD		0x00			// 使用されていない
#define MWIN_OPEN		0x01			// オープン中
#define MWIN_CLOSE		0x02			// クローズ中
#define MWIN_FREE		0x03			// 待ち状態

// メッセージウィンドウ(顔の状態) //
#define MFACE_NONE		0x00			// 表示されていない
#define MFACE_OPEN		0x01			// オープン中
#define MFACE_CLOSE		0x02			// クローズ中
#define MFACE_NEXT		0x03			// 次の顔へ
#define MFACE_WAIT		0x04			// 待ち状態

// その他の定数 //
#define CWIN_KEYWAIT	8



///// [構造体] /////

// 子ウィンドウの情報 //
typedef struct tagWINDOW_INFO{
	char			*Title;			// タイトル文字列へのポインタ(実体ではない！)
	char			*Help;			// ヘルプ文字列へのポインタ(これも実体ではない)

	BOOL			(*CallBackFn)(WORD);	// 特殊処理用コールバック関数(未使用ならNULL)
	BYTE			NumItems;				// 項目数(<ITEM_MAX)
	tagWINDOW_INFO	*ItemPtr[WINITEM_MAX];	// 次の項目へのポインタ
} WINDOW_INFO;

// ウィンドウ群 //
typedef struct tagWINDOW_SYSTEM{
	WINDOW_INFO		Parent;					// 親ウィンドウ
	int				x,y;					// ウィンドウ左上の座標
	DWORD			Count;					// フレームカウンタ
	BYTE			Select[WINDOW_DEPTH];	// 選択中の項目スタック
	BYTE			SelectDepth;			// 選択中の項目に対するＳＰ
	BYTE			State;					// 状態

	WORD			OldKey;					// 前に押されていたキー
	BYTE			KeyCount;				// キーボードウェイト
	BOOL			FirstWait;				// 最初のキー解放待ち
} WINDOW_SYSTEM;

// メッセージウィンドウ管理用構造体 //
typedef struct tagMSG_WINDOW{
	RECT		MaxSize;		// ウィンドウの最終的な大きさ
	RECT		NowSize;		// ウィンドウの現在のサイズ

	BYTE		FontID;				// 使用するフォント
	BYTE		FontDy;				// フォントのＹ増量値
	BYTE		State;				// 状態
	BYTE		MaxLine;			// 最大表示可能行数
	BYTE		Line;				// 次に挿入する行

	BYTE		FaceID;				// 使用する顔番号
	BYTE		NextFace;			// 次に表示する顔番号
	BYTE		FaceState;			// 顔の状態
	BYTE		FaceTime;			// 顔表示用カウンタ

	char		*Msg[MSG_HEIGHT];	// 表示するメッセージへのポインタ

	//char		MsgBuf[MSG_HEIGHT][MESSAGE_MAX];	// メッセージ格納配列の実体
} MSG_WINDOW;

// ウィンドウ全体の情報 //
typedef struct tagWIN_GRPINFO{
	//LPDIRECTDRAWSURFACE		lpSurf;						// ウィンドウデータ用Surface
	HFONT					SmallFont;					// フォント(小さい文字用)
	HFONT					NormalFont;					// フォント(通常の文字用)
	HFONT					LargeFont;					// フォント(大きい文字用)
} WIN_GRPINFO;



///// [ 関数 ] /////

// コマンドウィンドウ処理 //

// コマンドウィンドウの初期化 //
void CWinInit(WINDOW_SYSTEM *ws,int x,int y,int select);

void CWinMove(WINDOW_SYSTEM *ws);				// コマンドウィンドウを１フレーム動作させる
void CWinDraw(WINDOW_SYSTEM *ws);				// コマンドウィンドウの描画
BOOL CWinExitFn(WORD key);						// コマンド [Exit] のデフォルト処理関数


// メッセージウィンドウ処理 //
void MWinOpen(RECT *rc);		// メッセージウィンドウをオープンする
void MWinClose(void);			// メッセージウィンドウをクローズする
void MWinForceClose(void);		// メッセージウィンドウを強制クローズする
void MWinMove(void);			// メッセージウィンドウを動作させる(後で上と統合する)
void MWinDraw(void);			// メッセージウィンドウを描画する(上に同じ)

void MWinMsg(char *s);			// メッセージ文字列を送る
void MWinFace(BYTE faceID);		// 顔をセットする
void MWinCmd(BYTE cmd);			// コマンドを送る

void MWinHelp(WINDOW_SYSTEM *ws);		// メッセージウィンドウにヘルプ文字列を送る



///// [グローバル変数] /////

extern WIN_GRPINFO		WinGrpInfo;		// ウィンドウに使用するフォント、Surface など
extern MSG_WINDOW		MsgWindow;		// メッセージウィンドウ



#endif

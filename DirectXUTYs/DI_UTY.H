/*                                                                           */
/*   DI_UTY.h   Direct Input 関連の関数                                      */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DI_UTY_H
#define PBGWIN_DI_UTY_H		"DI_UTY : Ver 0.10"


// 更新履歴 //

// 2000/01/11 : .c->.cpp に変更、英語->日本語に変更。


#include "DX_TYPE.h"
#include "DX_ERROR.h"


// キーボード定数 //
#define KEY_UP			0x0001
#define KEY_DOWN		0x0002
#define KEY_LEFT		0x0004
#define KEY_RIGHT		0x0008
#define KEY_TAMA		0x0010
#define KEY_BOMB		0x0020
#define KEY_SHIFT		0x0040
#define KEY_RETURN		0x0080
#define KEY_ESC			0x0100

#define KEY_ULEFT		(KEY_UP|KEY_LEFT)
#define KEY_URIGHT		(KEY_UP|KEY_RIGHT)
#define KEY_DLEFT		(KEY_DOWN|KEY_LEFT)
#define KEY_DRIGHT		(KEY_DOWN|KEY_RIGHT)


// キーボード定義用構造体 //
typedef struct tagInputConfig{
	BYTE	KeyUp;			// 上方向に移動
	BYTE	KeyDown;		// 下方向に移動
	BYTE	KeyLeft;		// 左方向に移動
	BYTE	KeyRight;		// 右方向に移動
	BYTE	KeyTama;		// 弾発射＆決定
	BYTE	KeyBomb;		// ボム発動＆キャンセル
	BYTE	KeyShift;		// 低速移動

	BYTE	KeyULeft;			// 左上
	BYTE	KeyURight;			// 右上
	BYTE	KeyDLeft;			// 左下
	BYTE	KeyDRight;			// 右下

	DWORD	PadMask_Tama;		// パッド用マスク
	DWORD	PadMask_Bomb;		//
	DWORD	PadMask_Shift;		//
	DWORD	PadMask_Cancel;		//

	BOOL	bIsUsed[256];		// 多重定義防止用
} InputConfig;

typedef struct tagINPUT_OBJ{
	LPDIRECTINPUT			pdi;
	LPDIRECTINPUTDEVICE		pdev;
} INPUT_OBJ;


// グローバル変数(Public) //
extern WORD Key_Data;
extern WORD Pad_Data;

extern void (*Key_Read)(void);


// 関数プロトタイプ宣言 //
extern BOOL Key_Start(HINSTANCE hInst,HWND hWnd);	// キー入力開始
extern void Key_Config(WORD Key);					// あるキーボード定数に対するコンフィグ
extern void Key_End(void);							// キー入力終了

extern void Key_Read_STD(void);
extern void Key_Read_DI(void);

extern int  Key_PadConfig(WORD Key);
extern void Key_PadConfig2(BYTE PadID, WORD KeyData);

#endif

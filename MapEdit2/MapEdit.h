/*
 *   MapEdit.h   簡易マップエディタ
 *
 */

#ifndef PBGWIN_MAPEDIT_H

#define MAPEDIT2_VERSION	"Ver 0.72"
#define PBGWIN_MAPEDIT_H	"MapEdit : "MAPEDIT2_VERSION" Update 2000/02/01"
#pragma message(PBGWIN_MAPEDIT_H)

#include "PBGUTY_X.h"
#include "..\\Gian07SrcFiles\\SCROLL.h"


// グラフィック関連定数 //
#define ID_MAP_CHIP		0
#define ID_TAMA			1


// マップ管理用定数 //
#define UNDO_MAX			10						// 最大Ｕｎｄｏ回数

#define KEY_WAITTIME		10						// キーボード用ウェイト


#define RESERVED_WIDTH		2						// 予約済みの幅
#define RESERVED_HEIGHT		1						// 予約済みの高さ

#define SCROLLWAIT_DEFAULT	1						// スクロールの重みの初期値

#define MAP_HEIGHT			(30-RESERVED_HEIGHT)	// マップの表示する縦幅
#define MAP_LEN				1000					// マップの縦幅
#define MAP_SIZE			(MAP_WIDTH*MAP_LEN)		// マップに必要なメモリ

#define CHIP_WIDTH			(40-RESERVED_WIDTH-MAP_WIDTH)

#define BMP_CHIP_WIDTH		640						// マップチップの幅
#define BMP_CHIP_HEIGHT		480						// マップチップの高さ



// マップデータ管理用構造体 //
typedef struct tagMapData{
	BOOL	bMask;			// マスクがかかっているか
	PBGMAP	DataID;			// マップパーツ番号
} MapData;


// マップ全体のMapData //
typedef struct tagMapInfo{
	MapData		Data[LAYER_MAX][MAP_SIZE];	// 全レイヤーを含むマップデータ
	WORD		ScrollWait[LAYER_MAX];		// １Dotスクロールに要するカウント数
} MapInfo;


// Ｕｎｄｏ用の環状スタック //
typedef struct tagUndoStack{
	int			StackPtr;			// スタックポインタ
	int			UndoLeft;			// 残りＵｎｄｏ回数
	int			RedoLeft;			// 残りＲｅｄｏ回数
	MapInfo		Data[UNDO_MAX];		// マップデータ１個分
} UndoStack;


// マップパーツコピー用構造体 //
typedef struct tagCopyData{
	PBGMAP	*Data;			// コピーするデータ列
	int		Width;			// 幅
	int		Height;			// 高さ
} CopyData;


// マップエディタの状態 //
typedef enum tagMeState{
	ME_NONE,			// 何もしない
	ME_MASK_ON,			// マスク(描き込み不可)領域選択中
	ME_MASK_OFF,		// マスク(描き込み 可 )領域選択中
	ME_GETRECTM,		// パーツ選択中(Mapﾊﾟｰﾂから)
	ME_GETRECTE,		// パーツ選択中(作業領域から)
	ME_PUTRECT,			// パーツ貼り付け中
	ME_VIEW,			// ビューモード
	ME_HELP,			// ヘルプ表示
} MeState;


// マップエディタ管理用構造体 //
typedef struct tagMapEditor{
	POINT		MouseInfo;			// マウスの状態(座標)
	POINT		MouseTemp;			// ＰＵＴ用マウス座標保持
	BYTE		KeyInfo[256];		// キーボードの状態

	BOOL		MouseR;				// マウスの右ボタンが押されている
	BOOL		MouseL;				// マウスの左ボタンが押されている

	MeState		State;				// 現在の状態
	RECT		Select;				// 矩形領域選択用

	DWORD		KeyCount;			// リピート制御カウンタ
	DWORD		ViewTime;			// ビューモードの時間

	int			Line;				// 画面下は何行か？
	int			Layer;				// 作業中のレイヤー
	int			ChipX;				// マップチップの表示Ｘ

	MapInfo		Data;				// 現在のマップの状態
} MapEditor;


// ぐろーばる変数 //
extern UndoStack	Undo;
extern MapEditor	Map;
extern CopyData		Copy;
extern GRP			GrMapChip;
extern GRP			GrTama;


BOOL EditorInit(void);			// エディタの初期化
void EditorMain(void);			// エディタのメイン
void EditorDraw(void);			// 画面描画関連

BOOL LoadMapData(char *filename);	// マップチップの配置データをLoadする
BOOL SaveMapData(char *filename);	// マップチップの配置データをSaveする

void ConvertRect(RECT *rc);		// 小→大となる矩形を得る

void StkRedo(void);				// Undo スタックを用いて Redo を行う
void StkPushUndo(void);			// Undo スタックに挿入
void StkPopUndo(void);			// Undo スタックから取り出し

void SetMask(void);				// マスクをセットする
void ResetMask(void);			// マスクをリセットする

void GetMapDataM(void);			// マップデータの取得(マップパーツエリア)
void GetMapDataE(void);			// マップデータの取得(作業領域)
void FreeMapData(void);			// マップデータの解放
void PutMapData(void);			// マップデータの貼り付け


#endif

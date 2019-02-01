/*                                                                           */
/*   LOADER.h   グラフィック、サウンド等のロード                             */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_LOADER_H
#define PBGWIN_LOADER_H		"LOADER : Version 0.10 : Update 2000/02/18"
//#pragma message(PBGWIN_LOADER_H)

#include "GIAN.h"
#include "Lens.h"


///// [ 定数 ] /////

// Surface 関連 //
#define SURFACE_ID_SYSTEM		0			// システム用       Surface のＩＤ
#define SURFACE_ID_MAPCHIP		1			// マップチップ用   Surface のＩＤ
#define SURFACE_ID_SPROJECT		1			// 西方Project表示用Surface のＩＤ
#define SURFACE_ID_ENEMY		2			// 敵(雑魚＆Boss)用 Surface のＩＤ
#define SURFACE_ID_MUSIC		2			// 音楽室用         Surface のＩＤ(Enemyと共有)
#define SURFACE_ID_TITLE		2			// たいとる用       Surface のＩＤ(上に同じ)
#define SURFACE_ID_NAMEREG		2			// お名前登録用     Surface のＩＤ(上に同じ)
#define SURFACE_ID_MTITLE		3			// 曲名加工用       Surface のＩＤ
#define SURFACE_ID_BOMBER		7			// ボム画像用       Surface のＩＤ
#define SURFACE_ID_LENS			8			// レンズ加工用     Surface のＩＤ

#define FACE_MAX				3			// 同時にロード可能な人数...
#define FACE_NUMX				6			// 顔グラの横の個数
#define SURFACE_ID_FACE0		4			// 顔グラ用(0)      Surface のＩＤ
#define SURFACE_ID_FACE1		5			// 顔グラ用(1)      Surface のＩＤ
#define SURFACE_ID_FACE2		6			// 顔グラ用(2)      Surface のＩＤ

// 特殊グラフィックID (For LoadGraph() ) //
#define GRAPH_ID_MUSICROOM		(128)		// 音楽室用ＢＭＰのＩＤ(数値は1-6 で無ければ良い)
#define GRAPH_ID_TITLE			(128+1)		// タイトル画面のＢＭＰのＩＤ
#define GRAPH_ID_NAMEREGIST		(128+2)		// お名前登録画面のＢＭＰのＩＤ
#define GRAPH_ID_EXSTAGE		(128+3)		// エキストラステージ・システム
#define GRAPH_ID_EXBOSS1		(128+4)		// エキストラステージ・ボス１
#define GRAPH_ID_EXBOSS2		(128+5)		// エキストラステージ・ボス２
#define GRAPH_ID_SPROJECT		(128+6)		// 西方Ｐｒｏｊｅｃｔの表示
#define GRAPH_ID_ENDING			(128+7)		// エンディングのロードを行う

// サウンド(効果音番号) //
#define SOUND_ID_KEBARI		0x00
#define SOUND_ID_TAME		0x01
#define SOUND_ID_LASER		0x02
#define SOUND_ID_LASER2		0x03
#define SOUND_ID_BOMB		0x04
#define SOUND_ID_SELECT		0x05
#define SOUND_ID_HIT		0x06
#define SOUND_ID_CANCEL		0x07
#define SOUND_ID_WARNING	0x08
#define SOUND_ID_SBLASER	0x09
#define SOUND_ID_BUZZ		0x0a
#define SOUND_ID_MISSILE	0x0b
#define SOUND_ID_JOINT		0x0c
#define SOUND_ID_DEAD		0x0d
#define SOUND_ID_SBBOMB		0x0e
#define SOUND_ID_BOSSBOMB	0x0f
#define SOUND_ID_ENEMYSHOT	0x10
#define SOUND_ID_HLASER		0x11
#define SOUND_ID_TAMEFAST	0x12
#define SOUND_ID_WARP		0x13

// サウンド(最大数) //
#define SNDMAX_KEBARI		5
#define SNDMAX_TAME			5
#define SNDMAX_LASER		1
#define SNDMAX_LASER2		1
#define SNDMAX_BOMB			1		// 5
#define SNDMAX_SELECT		1
#define SNDMAX_HIT			1		// 5
#define SNDMAX_CANCEL		1
#define SNDMAX_WARNING		1
#define SNDMAX_SBLASER		1
#define SNDMAX_BUZZ			2		// 2
#define SNDMAX_MISSILE		5
#define SNDMAX_JOINT		1
#define SNDMAX_DEAD			1
#define SNDMAX_SBBOMB		1
#define SNDMAX_BOSSBOMB		1
#define SNDMAX_ENEMYSHOT	5
#define SNDMAX_HLASER		1
#define SNDMAX_TAMEFAST		5
#define SNDMAX_WARP			1



typedef struct tagFACE_DATA{
	GRP					GrSurf;			// 顔グラ用 Surface へのポインタ
	PALETTEENTRY		pal[256];		// 顔グラ用パレット
} FACE_DATA;

// エンディングのグラフィック管理用 //
typedef struct tagENDING_GRP{
	GRP					GrSurf;			// Surface
	RECT				rcTarget;		// 矩形の範囲
	PALETTEENTRY		pal[256];		// パレット
} ENDING_GRP;



///// [ 関数 ] /////
extern BOOL InitSurface(void);			// Surface を作成する

extern BOOL LoadStageData(BYTE stage);				// ＥＣＬ&ＳＣＬデータ列をメモリ上にロードする
extern BOOL LoadGraph(int stage);					// あるステージのグラフィックをロードする
extern BOOL LoadFace(BYTE SurfaceID,BYTE FileNo);	// 顔グラフィックをロードする
extern BOOL LoadMusic(int no);						// ｎ番目の曲をロードする
extern BOOL LoadSound(void);						// 全ての Sound データをロードする
extern BOOL LoadMusicRoomComment(int no);			// MusicRoom のコメントをロードする

extern void LoadPaletteFromMap(void);		// 前後にある４０色をマップパーツのパレットにする
extern void LoadPaletteFromEnemy(void);		// 敵のパレットにする
extern void LoadPaletteFromMusic(void);

extern void CleanupLens(void);	//  レンズを破棄する


/*
// 廃止：2000/12/01 //
extern void EnterBombPalette(void);		// ボム用パレット属性に変更する
extern void LeaveBombPalette(void);		// ボム用パレット属性を外す
*/

//// [ 変数 ] ////
extern GRP			GrTama;				// システム用
extern GRP			GrEnemy;			// 敵(雑魚＆ボス)用
extern GRP			GrMap;				// 背景用
extern GRP			GrMTitle;			// 曲のタイトル加工用
extern GRP			GrBomber;			// ボム用グラフィック用
extern GRP			GrLens;				// レンズエフェクト用

extern FACE_DATA	FaceData[FACE_MAX];	// 顔グラ用

extern DWORD		MusicNum;			// 曲数

extern char MCommentBuffer[12][50];
extern char MTitleBuffer[50];

extern LensInfo *Lens;

extern PALETTEENTRY SProjectPalette[256];

extern ENDING_GRP	EndingGrp[6];



#endif

/*                                                                           */
/*   DD_UTY.h   描画関連の関数                                               */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DD_UTY_H
#define PBGWIN_DD_UTY_H		"DD_UTY : Version 0.07 : Update 2000/02/03"


// 更新履歴 //

// 2000/08/21 : クリッピング関数を更新・追加

// 2000/03/01 : GrpSetRestoreHook() を追加
// 2000/02/28 : GrpFatCircleA() を追加。使い方次第では便利なのでは？

// 1999/12/05 : クリッピング描画を追加(一通りの関数が揃う)
// 1999/12/02 : 列挙系の関数がほぼ完成する。ＢＭＰローダも後少しで完成するはず
//            : 関数ポインタの追加
// 1999/11/30 : D3D_IM を使用するように変更。それに伴い、全体的に大幅な変更あり
//            : また、D3DRM_UTY を削除した。


// ヘッダファイル //
#include "DX_TYPE.h"					// 型＆マクロ
#include "DX_ERROR.h"					// エラー処理用関数
#include "LZ_UTY.h"						// 圧縮管理用
#include "D3_TEX.h"						// テクスチャ管理
#include "MtoB.h"						// スクリーンキャプチャー


// 定数＆マクロ //
#define SURFACE_MAX		10				// テクスチャ,Prim,BackをのぞくSurfaceの最大数
#define XDDRAWENUM_MAX	4				// ビデオカード４枚差しをする人はいないと思うので...
#define XD3DENUM_MAX	8				// DDrawデバイスに対する3Dデバイスの列挙最大数

#define ALPHA_ONE		0x00			// 一種の加算α
#define ALPHA_NORM		0x01			// ノーマルなSrc-α

#define RGB256(r,g,b)	((r)+(g)*6+(b)*36+0x14)


// 構造体 //
typedef struct tagDXOBJECT{
	LPDIRECTDRAW2			DD;			// DirectDraw Object
	LPDIRECT3D2				D3;			// Direct3D Object
	LPDIRECT3DDEVICE2		D3Dev;		// Direct3D Device
	LPDIRECT3DVIEWPORT2		View;		// Direct3D Viewport
	LPDIRECT3DMATERIAL2		Mat;		// Direct3D Material

	LPDIRECTDRAWPALETTE		Pal;		// パレット(8Bitのときだけ)
	LPDIRECTDRAWCLIPPER		Clip;		// クリッパー(WindowModeのみ)

	GRP						Prim;		// 表画面
	GRP						Back;		// 裏画面
	GRP						Zbuf;		// Ｚバッファ

	PALETTEENTRY			pe[256];	// パレットの初期値

	DWORD					Bpp;			// 色数(なぜ３２ビット？)
	HWND					hWnd;			// ウィンドウハンドル
	BOOL					bWndMode;		// ウィンドウモードか？
	BOOL					bNeedChgPal;	// パレットを変更する必要があるか
} DXOBJECT;

typedef struct tagDXSURFACE{
	GRP			surf;			// DirectDraw OffscreenSurface (NULLなら未使用)
	BYTE		*img;			// BMP イメージを一時的に保存 (NULL ならロードはまだ)
	int			width;
	int			height;
} DXSURFACE;

typedef struct tagD3DENUM{
	LPGUID		lpGuid;					// GUID
	char		*name;					// お名前
	BOOL		bIsHW;					// ハードウェアか
	BOOL		b8Bit;					// この D3D_DEVICE が  8Bit に対応しているか
	BOOL		b16Bit;					// この D3D_DEVICE が 16Bit に対応しているか
	BOOL		b24Bit;					// この D3D_DEVICE が 24Bit に対応しているか
	BOOL		b32Bit;					// この D3D_DEVICE が 32Bit に対応しているか
	DWORD		Zbuf;					// サポートするＺバッファの深度(０は多分無いのでしょう)
} XD3DENUM;

typedef struct tagDDRAWENUM{
	LPGUID		lpDDGuid;				// あるドライバのGUID
	char		*name;					// お名前
	XD3DENUM	D3D[XD3DENUM_MAX];		// D3D_DEVICE の GUID & ドライバ名
	DWORD		D3DNow;					// D3D_DEVICE の現在の数
	BOOL		b8Bit;					// この DD_DEVICE が  8Bit に対応しているか
	BOOL		b16Bit;					// この DD_DEVICE が 16Bit に対応しているか
	BOOL		b24Bit;					// この DD_DEVICE が 24Bit に対応しているか
	BOOL		b32Bit;					// この DD_DEVICE が 32Bit に対応しているか
} XDDRAWENUM;


// グローバル変数 //
extern DXOBJECT		DxObj;
extern DXSURFACE	DxSurf[SURFACE_MAX];
extern XDDRAWENUM	DxEnum[XDDRAWENUM_MAX];
extern DWORD		DxEnumNow;


// 初期化など //
FBOOL GrpEnum(HWND hWnd);								// 使用できるデバイスを列挙する
FBOOL GrpInit(LPGUID DDG,XD3DENUM *D3De,BYTE Bpp);
FBOOL GrpInitEmu8(void);								// 「えみゅ」で８ビット初期化
FBOOL GrpInitW(LPGUID DDG,XD3DENUM *D3De,BYTE Bpp);
FVOID GrpCleanup(void);									// いつも通りに(ただし失敗したら異常終了)
FVOID GrpSetClip(int sx,int sy,int ex,int ey);			// 3DViewportが生きていたらそちらもセット
FBOOL GrpRestore(void);									// ロード済みのSurfaceを修復

FVOID GrpGetPalette(PALETTEENTRY *pal);					// 現在のパレットを取得する
FVOID GrpSetPalette(PALETTEENTRY *pal);					// 現在のパレットを変更する
FVOID GrpSetRestoreHook(FBOOLFN(*func)(void));			// Restore の事後処理フック

// オフスクリーン Surface 作成 //
FGRP  GrpCreateSurface(int width,int height,BYTE surf_id);	// Surface の作成(BMPリロード有り)
FBOOL GrpReleaseSurface(BYTE surf_id);						// Surface の削除(BMPは変更せず)

// ＢＭＰファイルロード //
FBOOL GrpBMPLoad(char *filename,BYTE surf_id);				// BMP->Surface
FBOOL GrpBMPLoadP(BIT_DEVICE *in,DWORD n,BYTE surf_id);		// (圧)BMP->Surface
FBOOL GrpBMFree(BYTE surf_id);								// SurfaceからBMPだけ切り離す

extern BOOL GrpSetColorKey(LPDIRECTDRAWSURFACE lpds);	// ビット深度によりカラーキーをセットする
extern void GrpOffsClear(LPDIRECTDRAWSURFACE lpds);		// ある Surface を初期化する


// 図形描画関連関数Ptr(GrpInitにより初期化される) //
FVOID GrpCls(void);							// 指定色で塗りつぶしをかける(クリッピング無効)
FVOID GrpClsEx(RECT *Target, DWORD col);	// 直接番号指定で矩形内を塗りつぶす

extern FVOIDFN(*GrpFlip)(void);								// ウィンドウモード時の動作に注意
extern FVOIDFN(*GrpLock)(void);								// 図形描画の準備をする
extern FVOIDFN(*GrpUnlock)(void);							// 図形描画を完了する

extern FVOIDFN(*GrpSetColor)(BYTE r,BYTE g,BYTE b);			// 色セット
extern FVOIDFN(*GrpSetAlpha)(BYTE a,BYTE mode);				// αセット

extern FVOIDFN(*GrpLine)(int x1,int y1,int x2,int y2);		// 直線
extern FVOIDFN(*GrpBox)(int x1,int y1,int x2,int y2);		// 長方形
extern FVOIDFN(*GrpCircle)(int x,int y,int r);				// 円
extern FVOIDFN(*GrpCircleF)(int x,int y,int r);				// 円塗りつぶし
extern FVOIDFN(*GrpPolygon)(POINT *p,int n);				// ポリゴン塗りつぶし

extern FVOIDFN(*GrpBoxA)(int x1,int y1,int x2,int y2);		// α長方形
extern FVOIDFN(*GrpCircleA)(int x,int y,int r);				// α円塗りつぶし
extern FVOIDFN(*GrpPolygonA)(POINT *p,int n);				// αポリゴン塗りつぶし

extern FVOIDFN(*GrpFatCircleA)(int x,int y,int r,int w);	// 太い円を描く

extern PVOIDFN(*GrpBlt)(RECT *src,int x,int y,GRP surf);	// ビットマップ転送(WithClip)

//extern FVOIDFN(*GrpSetGamma)(BYTE r,BYTE g,BYTE b,BYTE a);	// ガンマコントロール

extern FVOID GrpBltEnable(BOOL flag);						// 描画を行うか？
extern PBOOL GrpClip(RECT *src,int *x,int *y);				// クリッピングをかける
extern FVOID GrpGetClip(RECT *dest);						// クリッピング枠を取得する


// カラーキー付き転送(クリッピングはないので注意！！) //
// src の値は変更されません //
IVOID GrpBltX(RECT *src,int x,int y,GRP surf)
{
	HRESULT		ddrval;

	for(;;){
		ddrval = DxObj.Back->BltFast(x,y,surf,src,DDBLTFAST_SRCCOLORKEY);
		if(ddrval == DD_OK){
			break;
		}
		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
			return;
		}
		if(ddrval != DDERR_WASSTILLDRAWING){
			return;
		}
	}
}


// カラーキー無し転送(クリッピングはないので注意！！) //
// src の値は変更されません //
IVOID GrpBltXC(RECT *src,int x,int y,GRP surf)
{
	HRESULT		ddrval;

	for(;;){
		ddrval = DxObj.Back->BltFast(x,y,surf,src,DDBLTFAST_NOCOLORKEY);
		if(ddrval == DD_OK){
			break;
		}
		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
			return;
		}
		if(ddrval != DDERR_WASSTILLDRAWING){
			return;
		}
	}
}

#endif

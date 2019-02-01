/*                                                                           */
/*   D3_TEX.h   テクスチャ管理用関数                                         */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_D3_TEX_H
#define PBGWIN_D3_TEX_H		"D3_TEX : Version 0.02 : Update 1999/12/13"


// 更新履歴 //



// ヘッダファイル //
#include "DX_TYPE.h"


// 定数＆マクロ //
#define TEXTURE_MAX		20				// テクスチャの枚数(最大)


// 構造体 //
typedef struct tagDXTEXTURE{
	D3DTEXTUREHANDLE	hTexture;
	GRP					surf;
	BYTE				*img;
} DXTEXTURE;


// グローバル変数 //
extern DXTEXTURE DxTex[TEXTURE_MAX];


// 関数 //
FBOOL TexInit(void);							// 最適なテクスチャフォーマットを検索する
FBOOL TexCreateTexture(char *name,DWORD txid);	// テクスチャをロードする
FVOID TexReleaseTexture(DWORD txid);			// テクスチャを解放する
FVOID TexSetTexture(DWORD txid);				// テクスチャハンドルをセットする


IINT TexGetWidth(BYTE *img)
{
	BITMAPFILEHEADER		*BMP_file;
	BITMAPINFOHEADER		*BMP_info;
	int						width,temp;

	BMP_file = (BITMAPFILEHEADER *)img;
	BMP_info = (BITMAPINFOHEADER *)((BMP_file) +   1);

	width = (BMP_info->biWidth+3)&(~3);		// DWORD に揃える(いらないか...)
	for(temp=1;temp<width;temp<<=1);		// Power of 2 なサイズを作る

	return min(temp,256);		// 大きすぎると扱えないので...
}

IINT TexGetHeight(BYTE *img)
{
	BITMAPFILEHEADER		*BMP_file;
	BITMAPINFOHEADER		*BMP_info;
	int						height,temp;

	BMP_file = (BITMAPFILEHEADER *)img;
	BMP_info = (BITMAPINFOHEADER *)((BMP_file) +   1);

	height = (BMP_info->biHeight);
	for(temp=1;temp<height;temp<<=1);		// Power of 2 なサイズを作る

	return min(temp,256);		// 大きすぎると扱えないので...
}

#endif

/*
 *   PaletteEx.h   : 特殊パレット処理
 *
 */

#ifndef PBG_PALETTEEX_VERSION
#define PBG_PALETTEEX_VERSION	"特殊パレット処理 : Version 0.01 : Update 2000/11/23"

/*  [更新履歴]
 *    Version 0.01 : 2000/11/23 :
 */



#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif



/***** [関数プロトタイプ] *****/

// パレットをｕｖ固定で変換 //
void ConvertPalette(PALETTEENTRY *pal, BYTE u, BYTE v);

// パレットをαで合成する(p1 = p1*a/255 + p2*(255-a)/255 //
void BlendPalette(PALETTEENTRY *p1, PALETTEENTRY *p2, BYTE a);



#endif

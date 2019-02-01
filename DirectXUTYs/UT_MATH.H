/*                                                                           */
/*   UT_MATH.h   整数限定の数学関数群                                        */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_UT_MATH_H
#define PBGWIN_UT_MATH_H		"UT_MATH : Version 0.01 : Update 1999/12/04"


// 更新履歴 //

// 1999/12/05 : PBG_MATH.c より移植開始


// 使用禁止かな //
#ifndef DISABLE_UT_MATH


// C++ 対策 //
#ifdef __cplusplus
extern "C" {
#endif


// ヘッダファイル //
#include <windows.h>


// マクロ //
#ifndef max
	#define max(a,b)	((a>b) ? (a) : (b))
	#define min(a,b)	((a<b) ? (a) : (b))
#endif

#define sinm(deg) (SIN256[(unsigned char)deg]) // SINﾃｰﾌﾞﾙ参照用ﾏｸﾛ
#define cosm(deg) (COS256[(unsigned char)deg]) // COSﾃｰﾌﾞﾙ参照用ﾏｸﾛ


// 定数 //
extern const signed int SIN256[256+64];
extern const signed int *COS256;


// 三角関数2 //
long __fastcall sinl(BYTE deg,int length);	// SIN(deg)*length/256
long __fastcall cosl(BYTE deg,int length);	// COS(deg)*length/256
long __fastcall sinDiv(BYTE deg, int length);	// length*256/(SIN(deg)>0 ? SIN(deg) : 256)
long __fastcall cosDiv(BYTE deg, int length);	// length*256/(COS(deg)>0 ? COS(deg) : 256)

BYTE pascal atan8(int x,int y);				// ３２ビット版です


// 平方根(整数版) //
int isqrt(int s);


// 乱数 //
void __fastcall rnd_seed_set(DWORD val);
WORD __fastcall rnd(void);


// デバッグ用(後で消すこと) //
//extern DWORD random_ref;


// C++ 対策 //
#ifdef __cplusplus
}
#endif


// 使用禁止かな //
#endif


#endif

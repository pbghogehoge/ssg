/*                                                                           */
/*   FontUty.h   フォントの処理いろいろ                                      */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_FONTUTY_H
#define PBGWIN_FONTUTY_H		"FONTUTY : Version 0.02 : Update 2000/07/22"
//#pragma message(PBGWIN_FONTUTY_H)



///// [更新履歴] /////

// 2000/07/22 : フォント追加に伴い、プログラムの一部を変更
// 2000/02/19 : フォントに関する処理の開発を始める



#include "GIAN.h"



typedef union tagEXPIXEL {
	BYTE		Byte;
	WORD		Word;
} EXPIXEL;

typedef struct tagEXPOINT {
	int		x, y;		// 元座標(中心からの相対座標)
	int		l;			// 極座標における長さ
	BYTE	d;			// 極座標における角度

	EXPIXEL	c;			// 色情報
} EXPOINT;

typedef struct tagExtraFontInfo {
	int		Width;		// 元画像の幅
	int		Height;		// 元画像の高さ

	int		DataSize;	// 点の数

	EXPOINT	*Data;		// データ格納先
} ExtraFontInfo;




///// [ 関数 ] /////
extern FVOID GrpPut16(int x,int y,const char *s);		// 16x16 透過フォントで文字列出力(高速)
extern FVOID GrpPut16c(int x,int y,char *s);	// 16x16 透過フォントで文字列出力(ｸﾘｯﾋﾟﾝｸﾞ有)
extern FVOID GrpPut16c2(int x,int y,char *s);	// 上と同じだが、ｘ移動幅が１６
extern FVOID GrpPutc(int x,int y,char c);		// 16x16 透過フォントで文字出力(ｸﾘｯﾋﾟﾝｸﾞ有)
extern FVOID GrpPut57(int x,int y,char *s);		// 05x07 べた貼りフォント
extern FVOID GrpPut7B(int x,int y,char *s);		// 07x11 音楽室用フォント
extern FVOID GrpPutScore(int x,int y,char *s);	// 得点アイテムのスコアを描画

extern FVOID GrpPutMidNum(int x,int y,int n);	// MIDI 用フォントを描画する

extern FVOID InitFonts(void);					// フォントをセットする
extern FVOID CleanupFont(void);					// フォントを解放する

extern FVOID DrawGrdFont(GRP surf,char *s);		// グラデーション付きフォントを描画する

ExtraFontInfo *CreateExtraFont(GRP Surface, RECT *pSrc);			// フォント生成
void DeleteExtraFont(ExtraFontInfo *pFont);							// フォント削除
void DrawExtraFont(ExtraFontInfo *pFont, int ox, int oy, int t);	// フォント描画
void DrawExtraFont2(ExtraFontInfo *pFont, int ox, int oy, int t);	// フォント描画


#endif

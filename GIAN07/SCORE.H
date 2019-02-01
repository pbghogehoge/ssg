/*                                                                           */
/*   Score.h   スコア入出力関数                                              */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_SCORE_H
#define PBGWIN_SCORE_H		"SCORE : Version 0.01 : Update 2000/07/29"
//#pragma message(PBGWIN_SCORE_H)

#include "Gian.h"



///// [ 定数 ] /////
#define NR_NAME_LEN		9		// ネームレジストの名前の長さ('\0' 含む)
#define NR_RANK_MAX		5		// 順位付け(Save)される最大数



///// [構造体] /////

typedef struct tagNR_NAME_DATA{
	char		Name[NR_NAME_LEN];		// 名前
	__int64		Score;					// スコア
	DWORD		Evade;					// かすり回数
	BYTE		Stage;					// ステージ
	BYTE		Weapon;					// 装備品
} NR_NAME_DATA;

typedef struct tagNR_SCORE_DATA{
	NR_NAME_DATA	Easy[NR_RANK_MAX];		// 難易度：Ｅａｓｙ
	NR_NAME_DATA	Normal[NR_RANK_MAX];	// 難易度：Ｎｏｒｍａｌ
	NR_NAME_DATA	Hard[NR_RANK_MAX];		// 難易度：Ｈａｒｄ
	NR_NAME_DATA	Lunatic[NR_RANK_MAX];	// 難易度：Ｌｕｎａｔｉｃ
	NR_NAME_DATA	Extra[NR_RANK_MAX];		// 難易度：Ｅｘｔｒａ
} NR_SCORE_DATA;

typedef struct tagNR_SCORE_STRING{
	BYTE	Rank;					// 実際の順位(ある順位が複数ある場合の対策)
	int		x,y;					// 描画用座標
	BOOL	bMoveEnable;			// 移動可能か？

	char	Name[NR_NAME_LEN];		// 名前
	char	Score[11+1];			// 得点
	char	Evade[6+1];				// かすり
	char	Stage[1+1];				// ステージ
	BYTE	Weapon;					// 装備
} NR_SCORE_STRING;



///// [ 関数 ] /////
int  SetScoreString(NR_NAME_DATA *NData, BYTE Dif);	// 現在のスコア列を取得する(Ret:下と同じ)
int  IsHighScore(NR_NAME_DATA *NData, BYTE Dif);	// ０：ハイスコアでない  それ以外：順位
BOOL SaveScoreData(NR_NAME_DATA *NData, BYTE Dif);	// スコアデータを書き出す



///// [グローバル変数] /////
extern NR_SCORE_STRING ScoreString[NR_RANK_MAX];	// スコアデータ文字列格納先


#endif

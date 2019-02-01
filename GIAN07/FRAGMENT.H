/*************************************************************************************************/
/*   FRAGMENT.H   破片処理関数群                                                                 */
/*                                                                                               */
/*************************************************************************************************/

#ifndef PBGWIN_FRAGMETN_H
#define PBGWIN_FRAGMENT_H		"FRAGMENT : Ver 0.10 : Update 99/10/31"
//#pragma message(PBGWIN_FRAGMENT_H)


//// 破片定数 ////
#define FRAGMENT_MAX	1000		// 破片の最大数
#define FRG_EVADE		0x00		// かすり(開発中)
#define FRG_SMOKE		0x01		// 煙その１
#define FRG_FATCIRCLE	0x02		// 赤丸...
#define FRG_STAR1		0x03		// お星様１
#define FRG_STAR2		0x04		// お星様２
#define FRG_HIT			0x05		// ショットがヒットした
#define FRG_STAR3		0x06
#define FRG_HEART		0x07		// ハート型

#define FRG_ESCAPE		0x10		// 指定座標から逃げる
#define FRG_APPROACH	0x20		// 指定座標に近づく


//// 破片構造体 ////
typedef struct{
	int		x,y;		// 現在の座標
	int		vx,vy;		// 速度成分 (x64)
	BYTE	count;		// フレームカウンタ(０の時は使用していないとする)
	BYTE	cmd;		// どんな破片？
} FRAGMENT_DATA;


//// 破片用変数 ////
extern FRAGMENT_DATA	Fragment[FRAGMENT_MAX];


//// 破片関数 ////
void fragment_set(int x,int y,BYTE cmd);
void fragment_move(void);
void fragment_draw(void);
void fragment_setup(void);


#endif

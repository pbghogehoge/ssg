/*                                                                           */
/*   GRPUTY.h   グラフィック補助関数                                         */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_GRPUTY_H
#define PBGWIN_GRPUTY_H		"GRPUTY : Version 0.01 : Update 1999/12/05"
#pragma message(PBGWIN_GRPUTY_H)


// 更新履歴 //


// ヘッダファイル //
#include "PBGUTY_X.h"


// 関数 //
void GrpPuts(char *s,int x,int y);
void GrpPut8(char *s,int x,int y);

/*
_inline void BltSetRect(RECT *rc,int x1,int y1,int x2,int y2)
{
	rc->right  = (rc->left = x1) + x2;
	rc->bottom = (rc->top  = y1) + y2;
}
*/

#endif

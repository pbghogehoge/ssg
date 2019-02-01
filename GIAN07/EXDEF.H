/*                                                                           */
/*   ExDef.h   いろんなものの定義とか                                        */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_EXDEF_H
#define PBGWIN_EXDEF_H		"EXDEF : Version 0.01 : Update 2000/09/04"
//#pragma message(PBGWIN_EXDEF_H)




// 角度付き座標の管理用構造体 //
typedef struct tagDegPoint{
	int		x,y;	// 座標
	BYTE	d;		// 角度
} DegPoint;



#endif

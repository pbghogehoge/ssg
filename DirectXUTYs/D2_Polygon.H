/*                                                                           */
/*   D2_Polygon.h   ８ビット時のポリゴン描画                                 */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_D2_POLYGON_H
#define PBGWIN_DX_POLYGON_H		"D2_POLYGON : Version 0.01"


//// [更新履歴] ////
// 2000/05/15       : アセンブラ化で、かろうじてバグ排除



//// [ヘッダファイル] ////
#include "PBGUTY_X.h"


//// [構造体] ////

typedef struct tagLINE_WORK{
	int		x;		// 開始ｘ座標
	int		dlx;	// 誤差変数加算値
	int		s;		// 誤差変数
	int		d;		// 最小ｘ移動量
} LINE_WORK;



//// [インライン関数] ////

// DDA 構造体のセット //
IVOID SetLineWork(LINE_WORK *w,int x1,int x2,int dy)
{
	int		x,dlx,s,d;

	_asm{
		PUSH	ESI

		MOV		EDX,x1
		MOV		EAX,x2
		MOV		ECX,dy

		MOV		x,EDX

		SUB		EAX,EDX
		CMP		ECX,1
		ADC		ECX,0
		CDQ
		IDIV	ECX
		CMP		EDX,80000000H
		ADC		EAX,-1
		MOV		d,EAX
		CMP		EDX,80000000H
		CMC
		SBB		ESI,ESI
		ADD		EDX,ESI
		XOR		EDX,ESI
		XOR		EAX,EAX
		DIV		ECX
		ADD		EAX,ESI
		XOR		EAX,ESI
		MOV		dlx,EAX
		MOV		s,80000000H
		POP		ESI
	}

	w->x = x;
	w->dlx = dlx;
	w->s = s;
	w->d = d;
}



//// [ 関数 ] ////
FVOID DrawPolygon(POINT *p,int n);		// ポリゴンの描画



#endif

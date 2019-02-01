/*                                                                           */
/*   DD_GRP3D.h   ３Ｄ時の図形描画関数                                       */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DD_GRP3D_H
#define PBGWIN_DD_GRP3D_H		"DD_GRP3D : Version 0.03 : Update 2000/02/22"


// 更新履歴 //

// 2000/02/22 : GrpGrdPolygon() を追加
// 1999/12/04 : レーザー用特殊α関数を追加
// 1999/12/02 : やっと一部の関数が動作し始めた。16Bit-αはやっぱり綺麗だな...


// ヘッダファイル //
#include "DX_TYPE.h"


// 関数 //
FVOID _3DSetup(void);							// 描画系関数ポインタをセットする

FVOID _3DLock(void);							// 画面描画の準備をする
FVOID _3DUnlock(void);							// 画面描画を完了する

FVOID _3DSetColor(BYTE r,BYTE g,BYTE b);		// 色セット
FVOID _3DSetAlpha(BYTE a,BYTE mode);			// αセット

FVOID _3DLine(int x1,int y1,int x2,int y2);		// 直線
FVOID _3DBox(int x1,int y1,int x2,int y2);		// 長方形
FVOID _3DCircle(int x,int y,int r);				// 円
FVOID _3DCircleF(int x,int y,int r);			// 円塗りつぶし
FVOID _3DPolygon(POINT *p,int n);				// ポリゴン塗りつぶし

FVOID _3DBoxA(int x1,int y1,int x2,int y2);		// α長方形
FVOID _3DCircleA(int x,int y,int r);			// α円塗りつぶし
FVOID _3DPolygonA(POINT *p,int n);				// αポリゴン塗りつぶし
FVOID _3DFatCircleA(int x,int y,int r,int w);	// 太い半透明円描画


// ３Ｄ専用関数 //
FVOID GrpGrdRect(POINT *p);						// グラデーション付き長方形(ナナメ可)
FVOID GrpGrdRectA(POINT *p);					// グラデーション付き長方形(ナナメ可+α)
FVOID GrpGrdPolygon(POINT *p,int n);			// グラデーション付きポリゴン(レーザー用？)
FVOID GrpGrdLineEx(int x,int y1,int y2);		// スペアな用グラデーションライン


// インライン関数 //
IVOID Set2DPoint(D3DTLVERTEX *tlv,int x,int y)
{
	extern BYTE	 Col3D_R,Col3D_G,Col3D_B,Col3D_A;

	memset(tlv,0,sizeof(D3DTLVERTEX));

	tlv->sx       = D3DVAL(x);
	tlv->sy       = D3DVAL(y);
	tlv->sz       = D3DVAL(0);
	tlv->rhw      = D3DVAL(1);
	tlv->color    = RGBA_MAKE(Col3D_R,Col3D_G,Col3D_B,Col3D_A);
	tlv->specular = RGB_MAKE(0,0,0);
}

IVOID Set2DPointC(D3DTLVERTEX *tlv,int x,int y,DWORD c)
{
	memset(tlv,0,sizeof(D3DTLVERTEX));

	tlv->sx       = D3DVAL(x);
	tlv->sy       = D3DVAL(y);
	tlv->sz       = D3DVAL(0);
	tlv->rhw      = D3DVAL(1);
	tlv->color    = c;
	tlv->specular = RGB_MAKE(0,0,0);
}

IBOOL AlphaEnable(BOOL flag)
{
	HRESULT dxret;

	dxret = DxObj.D3Dev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,flag);
	if(dxret!=D3D_OK){
		if(!GrpRestore()) return FALSE;
	}

	return TRUE;
}


#endif

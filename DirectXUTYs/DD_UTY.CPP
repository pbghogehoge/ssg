/*                                                                           */
/*   DD_UTY.cpp   描画関連の関数                                             */
/*                                                                           */
/*                                                                           */

#include "DD_UTY.h"
#include "DD_GRP2D.h"					// ２Ｄ用描画関数(公開しないこと)
#include "DD_GRP3D.h"					// ３Ｄ用描画関数(公開しないこと)
#include "D3_TEX.h"
#include <stdio.h>
#include <io.h>
#pragma message(PBGWIN_DD_UTY_H)


// 図形描画関数に対するダミー関数 //
static FVOID DM_V(void);
static FVOID DM_BBBB(BYTE a,BYTE b,BYTE c,BYTE d);
static FVOID DM_BBB(BYTE a,BYTE b,BYTE c);
static FVOID DM_BB(BYTE a,BYTE b);
static FVOID DM_III(int a,int b,int c);
static FVOID DM_IIII(int a,int b,int c,int d);
static FVOID DM_PI(POINT *p,int n);
static FBOOL DM_RESTORE(void);
static PVOID DM_GRPBLT(RECT *src,int x,int y,GRP surf);

// クリッピング枠 //
int CLIP_X_MIN = 0;
int CLIP_Y_MIN = 0;
int CLIP_X_MAX = 639;
int CLIP_Y_MAX = 479;

// キャプチャーを行うか(->MtoB) //
BOOL CaptureFlag = FALSE;
PVOID STD_GrpBlt(RECT *src,int x,int y,GRP surf);


// オフスクリーン Surface を Restore し、ＢＭＰがあれば読み直す //
static BOOL OffsRestore(DXSURFACE *surf);
static BOOL GrpBMLoadMemory(DXSURFACE *surf);

// カラーキーをセットする DXSURFACE 用 //
extern BOOL SetColorKey(DXSURFACE *surf);

// Ｚバッファを必要なら生成し、接続する //
static BOOL CreateZbuffer(XD3DENUM *D3De);

// 列挙用コールバック関数群 //
static BOOL FAR PASCAL DriverEnum(GUID FAR *lpGUID,LPSTR lpDDesc,LPSTR lpDName,LPVOID lpContext);
static HRESULT FAR PASCAL DeviceEnum(LPGUID lpGUID,LPSTR lpDeviceDesc,LPSTR lpdeviceName,LPD3DDEVICEDESC lpHWDesc,LPD3DDEVICEDESC lpHELDesc,LPVOID lpContext);
static HRESULT FAR PASCAL ModeEnum(DDSURFACEDESC *pddsd,VOID *pInfo);

// デバイスが求める性能を持っているか //
static BOOL ConfirmDevice3D(LPD3DDEVICEDESC lpDesc);

// ダミー関数をセットする //
static void SetDummyFunc(void);


// フリップ用関数 //
static FVOID GrpFlipFS(void);
static FVOID GrpFlipWnd(void);


// グローバル変数 //
DXOBJECT		DxObj;
DXSURFACE		DxSurf[SURFACE_MAX];
XDDRAWENUM		DxEnum[XDDRAWENUM_MAX];
DWORD			DxEnumNow;

// 図形描画用関数ポインタの実体 //
FVOIDFN(*GrpFlip)(void)                            = DM_V;
FVOIDFN(*GrpLock)(void)                            = DM_V;
FVOIDFN(*GrpUnlock)(void)                          = DM_V;
FVOIDFN(*GrpSetColor)(BYTE r,BYTE g,BYTE b)        = DM_BBB;
FVOIDFN(*GrpSetAlpha)(BYTE a,BYTE mode)            = DM_BB;
FVOIDFN(*GrpLine)(int x1,int y1,int x2,int y2)     = DM_IIII;
FVOIDFN(*GrpBox)(int x1,int y1,int x2,int y2)      = DM_IIII;
FVOIDFN(*GrpCircle)(int x,int y,int r)             = DM_III;
FVOIDFN(*GrpCircleF)(int x,int y,int r)            = DM_III;
FVOIDFN(*GrpPolygon)(POINT *p,int n)               = DM_PI;
FVOIDFN(*GrpBoxA)(int x1,int y1,int x2,int y2)     = DM_IIII;
FVOIDFN(*GrpCircleA)(int x,int y,int r)            = DM_III;
FVOIDFN(*GrpPolygonA)(POINT *p,int n)              = DM_PI;
FVOIDFN(*GrpFatCircleA)(int x,int y,int r,int w)   = DM_IIII;
//FVOIDFN(*GrpSetGamma)(BYTE r,BYTE g,BYTE b,BYTE a) = DM_BBBB;
PVOIDFN(*GrpBlt)(RECT *src,int x,int y,GRP surf)   = STD_GrpBlt;

static FBOOLFN(*RestoreHookFn)(void) = DM_RESTORE;		// Restore の事後処理用関数


FBOOL GrpEnum(HWND hWnd)
{
	HRESULT		dxret;

	ZEROMEM(DxEnum);
	DxEnumNow = 0;

	try{
		// 列挙開始 //
		dxret = DirectDrawEnumerate(DriverEnum,NULL);
		if(dxret!=DD_OK) throw("DirectDrawEnumerate に失敗");

		// 一応ダミー関数をセットしておく //
		SetDummyFunc();
	}
	catch(char *error){
		ErrInsert(error);
		return FALSE;
	}

	DxObj.hWnd = hWnd;

#ifdef _DEBUG
	ErrInsert("正常に列挙できたの");
#endif

	return TRUE;
}

FBOOL GrpInit(LPGUID DDG,XD3DENUM *D3De,BYTE Bpp)
{
	HRESULT				dxret;
	LPDIRECTDRAW		lpDD  = NULL;
	DDSURFACEDESC		ddsd;
	DDSCAPS				ddscaps;
//	D3DMATERIAL			mat;
//	D3DMATERIALHANDLE	hmat;

	if(DxObj.hWnd==NULL) return FALSE;

	GrpCleanup();
	ZEROMEM(ddsd);
	ZEROMEM(ddscaps);

	try{
		dxret = DirectDrawCreate(DDG,&lpDD,NULL);
		if(dxret!=DD_OK) throw("DirectDrawCreate に失敗");

		dxret = lpDD->QueryInterface(IID_IDirectDraw2,(void **)&DxObj.DD);
		if(dxret!=DD_OK) throw("QueryInterface に失敗");
		RELEASE(lpDD);

		// 強調レベルのセット //
		dxret = DxObj.DD->SetCooperativeLevel(DxObj.hWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
		if(dxret!=DD_OK) throw("SetCooperativeLevel に失敗");

		// ディスプレイモードのセット //
		dxret = DxObj.DD->SetDisplayMode(640,480,Bpp,0,0);
		if(dxret!=DD_OK) throw("SetDisplayMode に失敗");

		// プライマリ＆バックサーフェスの作成 //
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_3DDEVICE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		dxret = DxObj.DD->CreateSurface(&ddsd,&DxObj.Prim,NULL);
		if(dxret!=DD_OK) throw("CreateSurface(Prim) に失敗");

		// BackBuffer へのポインタを Get //
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		dxret = DxObj.Prim->GetAttachedSurface(&ddscaps,&DxObj.Back);
		if(dxret!=DD_OK) throw("GetAttachedSurface に失敗");

		// フルスクリーン用フリップ //
		GrpFlip = GrpFlipFS;

		// 2Dの初期化はここでおしまい //
		DxObj.Bpp = Bpp;
		if(Bpp==8) throw(TRUE);

		// Direct3D Object の取得 //
		dxret = DxObj.DD->QueryInterface(IID_IDirect3D2,(void **)&DxObj.D3);
		if(dxret!=DD_OK) throw("QueryInterface に失敗");

		// Direct3D Device の作成 //
		dxret = DxObj.D3->CreateDevice(*D3De->lpGuid,DxObj.Back,&DxObj.D3Dev);
		//dxret = DxObj.D3->CreateDevice(IID_IDirect3DHALDevice,DxObj.Back,&DxObj.D3Dev);
		if(dxret!=D3D_OK) throw("CreateDevice(D3D_Device) に失敗");

		// Ｚバッファ作成＆接続 //
		if(!CreateZbuffer(D3De))
			throw("CreateSurface(Z-Buffer) に失敗");

		// Direct3D Viewport の作成＆接続 //
		dxret = DxObj.D3->CreateViewport(&DxObj.View,NULL);
		if(dxret!=D3D_OK) throw("CreateViewport に失敗");
		dxret = DxObj.D3Dev->AddViewport(DxObj.View);
		if(dxret!=D3D_OK) throw("AddViewport に失敗");
		dxret = DxObj.D3Dev->SetCurrentViewport(DxObj.View);
		if(dxret!=D3D_OK) throw("SetCurrentViewport に失敗");

		throw(TRUE);
	}
	catch(char *error){
		ErrInsert(error);
		GrpCleanup();
		return FALSE;
	}
	catch(BOOL flag){
		if(Bpp==8) _2DSetup();
		else       _3DSetup();

		GrpSetClip(0,0,640,480);
		return flag;
	}

	return FALSE;
}

FBOOL GrpInitEmu8(void)
{
	HRESULT				dxret;
	LPDIRECTDRAW		lpDD  = NULL;
	DDSURFACEDESC		ddsd;
	DDSCAPS				ddscaps;

	if(DxObj.hWnd==NULL) return FALSE;

	GrpCleanup();
	ZEROMEM(ddsd);
	ZEROMEM(ddscaps);

	try{
		dxret = DirectDrawCreate((GUID *)DDCREATE_EMULATIONONLY,&lpDD,NULL);
		if(dxret!=DD_OK) throw("DirectDrawCreate に失敗");

		dxret = lpDD->QueryInterface(IID_IDirectDraw2,(void **)&DxObj.DD);
		if(dxret!=DD_OK) throw("QueryInterface に失敗");
		RELEASE(lpDD);

		// 強調レベルのセット //
		dxret = DxObj.DD->SetCooperativeLevel(DxObj.hWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
		if(dxret!=DD_OK) throw("SetCooperativeLevel に失敗");

		// ディスプレイモードのセット //
		dxret = DxObj.DD->SetDisplayMode(640,480,8,0,0);
		if(dxret!=DD_OK) throw("SetDisplayMode に失敗");

		// プライマリ＆バックサーフェスの作成 //
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_3DDEVICE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		dxret = DxObj.DD->CreateSurface(&ddsd,&DxObj.Prim,NULL);
		if(dxret!=DD_OK) throw("CreateSurface(Prim) に失敗");

		// BackBuffer へのポインタを Get //
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		dxret = DxObj.Prim->GetAttachedSurface(&ddscaps,&DxObj.Back);
		if(dxret!=DD_OK) throw("GetAttachedSurface に失敗");

		// フルスクリーン用フリップ //
		GrpFlip = GrpFlipFS;

		// 2Dの初期化はここでおしまい //
		DxObj.Bpp = 8;
	}
	catch(char *error){
		ErrInsert(error);
		GrpCleanup();
		return FALSE;
	}

	_2DSetup();
	GrpSetClip(0,0,640,480);

	return TRUE;
}

FBOOL GrpInitW(LPGUID DDG,XD3DENUM *D3De,BYTE Bpp)
{
	return FALSE;
}

FVOID GrpCleanup(void)
{
	int		i;

	RELEASE(DxObj.Mat);
	RELEASE(DxObj.View);
	RELEASE(DxObj.D3Dev);
	RELEASE(DxObj.D3);
	RELEASE(DxObj.Zbuf);

	RELEASE(DxObj.Clip);
	RELEASE(DxObj.Pal);

	// ここではSurfaceのBMPを解放しないことに注意 //
	for(i=0;i<SURFACE_MAX;i++){
		GrpReleaseSurface(i);
	}

	RELEASE(DxObj.Back);
	RELEASE(DxObj.Prim);
	RELEASE(DxObj.DD);

	// グラフィック系関数を実行できないようにする //
	SetDummyFunc();
}

// クリッピング枠を取得する //
FVOID GrpGetClip(RECT *dest)
{
	dest->left   = CLIP_X_MIN;
	dest->top    = CLIP_Y_MIN;

	dest->right  = CLIP_X_MAX;
	dest->bottom = CLIP_Y_MAX;
}

FVOID GrpSetClip(int sx,int sy,int ex,int ey)
{
	D3DVIEWPORT2		view;

	CLIP_X_MIN = (sx<0) ? 0 : sx;
	CLIP_Y_MIN = (sy<0) ? 0 : sy;
	CLIP_X_MAX = (ex>639) ? 639 : ex;
	CLIP_Y_MAX = (ey>479) ? 479 : ey;

	if(CLIP_X_MIN==CLIP_X_MAX || CLIP_Y_MIN==CLIP_Y_MAX)	GrpBltEnable(FALSE);
	else													GrpBltEnable(TRUE);

	if(DxObj.View){
		view.dwSize = sizeof(view);
		view.dwX          = sx;
		view.dwY          = sy;
		view.dwWidth      = ex-sx;
		view.dwHeight     = ey-sy;
		view.dvClipX      = D3DVAL(1);
		view.dvClipY      = D3DVAL(1);
		view.dvClipWidth  = D3DVAL(2);
		view.dvClipHeight = D3DVAL(2);
		view.dvMinZ       = D3DVAL(0);
		view.dvMaxZ       = D3DVAL(1);
		DxObj.View->SetViewport2(&view);
	}
}

FBOOL GrpRestore(void)
{
	HRESULT		dxval;
	int			i;

	if(DxObj.Prim){
		dxval = DxObj.Prim->Restore();
		if(dxval!=DD_OK) return FALSE;
	}
	if(DxObj.Back){
		DxObj.Back->Restore();
		if(dxval!=DD_OK) return FALSE;
	}
	if(DxObj.Zbuf){
		DxObj.Zbuf->Restore();
		if(dxval!=DD_OK) return FALSE;
	}

	for(i=0;i<SURFACE_MAX;i++){
		if(!OffsRestore(&DxSurf[i])) return FALSE;
	}

	return RestoreHookFn();
}

// Restore の事後処理フック //
FVOID GrpSetRestoreHook(FBOOLFN(*func)(void))
{
	// 安全対策(上を見れば分かるように、NULLチェックがないので...)
	if(func==NULL) RestoreHookFn = DM_RESTORE;
	else           RestoreHookFn = func;
}

// 現在のパレットを取得する(ただの代入なのでこの関数を使うまでも無いかもしれないね) //
FVOID GrpGetPalette(PALETTEENTRY *pal)
{
	int		i;

	// ８Ｂｉｔモード以外では、何もしないでリターンする //
	if(DxObj.Bpp!=8) return;

	for(i=0;i<256;i++){
		pal[i] = DxObj.pe[i];
	}
}

// 現在のパレットを変更する //
FVOID GrpSetPalette(PALETTEENTRY *pal)
{
	int			i;

	// ８Ｂｉｔモード以外では、何もしないでリターンする //
	if(DxObj.Bpp!=8) return;

	// 強制的に色をセットしてしまう //
	pal[0].peRed   = 0;
	pal[0].peGreen = 0;
	pal[0].peBlue  = 0;

	for(i=0;i<256;i++){
		DxObj.pe[i] = pal[i];
	}

	DxObj.Pal->SetEntries(0,0,256,pal);		// パレットを変更
	DxObj.bNeedChgPal = TRUE;

	DisableAlphaTable();
}

FGRP GrpCreateSurface(int width,int height,BYTE surf_id)
{
	HRESULT				dxval;
	DDSURFACEDESC		ddsd;
	GRP					temp = NULL;


	if(surf_id>=SURFACE_MAX) return NULL;
	if(DxSurf[surf_id].surf) return NULL;

	ZEROMEM(ddsd);

	ddsd.dwSize         = sizeof(ddsd);
	ddsd.dwWidth        = width;
	ddsd.dwHeight       = height;
	ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	dxval = DxObj.DD->CreateSurface(&ddsd,&temp,NULL);

	GrpOffsClear(temp);

	if(dxval==DD_OK){
		DxSurf[surf_id].surf   = temp;
		DxSurf[surf_id].width  = width;
		DxSurf[surf_id].height = height;
		OffsRestore(&DxSurf[surf_id]);	// メモリから再読み込み or ゼロクリア
	}
	else
		DxSurf[surf_id].surf = NULL;

	return DxSurf[surf_id].surf;
}

FBOOL GrpReleaseSurface(BYTE surf_id)
{
	if(surf_id>=SURFACE_MAX) return FALSE;
	RELEASE(DxSurf[surf_id].surf);

	return TRUE;
}

FBOOL GrpBMPLoad(char *filename,BYTE surf_id)
{
	FILE	*fp;
	size_t	size;

	if(surf_id>=SURFACE_MAX) return FALSE;

	// 以前に何かがロードされていたら解放する //
	GrpBMFree(surf_id);

	fp = fopen(filename,"rb");
	if(fp==NULL) return FALSE;

	size = filelength(fileno(fp));
	DxSurf[surf_id].img = (BYTE *)LocalAlloc(LPTR,size);
	if(DxSurf[surf_id].img==NULL){
		fclose(fp);
		return FALSE;
	}

	fread(DxSurf[surf_id].img,1,size,fp);
	fclose(fp);

	return GrpBMLoadMemory(&DxSurf[surf_id]);
}

FBOOL GrpBMPLoadP(BIT_DEVICE *in,DWORD n,BYTE surf_id)
{
	if(surf_id>=SURFACE_MAX) return FALSE;

	// 以前に何かがロードされていたら解放する //
	GrpBMFree(surf_id);

	if((DxSurf[surf_id].img=MemExpand(in,n))==NULL) return FALSE;

	return GrpBMLoadMemory(&DxSurf[surf_id]);
}

FBOOL GrpBMFree(BYTE surf_id)
{
	if(surf_id>=SURFACE_MAX) return FALSE;

	if(DxSurf[surf_id].img){
		LocalFree(DxSurf[surf_id].img);
		DxSurf[surf_id].img = NULL;
	}

	return TRUE;
}

FVOID GrpCls(void)
{
	DDBLTFX		ddbltfx;
	HRESULT		ddrval;

	ZeroMemory(&ddbltfx,sizeof(ddbltfx));
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = (DxObj.Bpp==8) ? RGB256(0,0,0) : 0;

	for(;;){
		ddrval = DxObj.Back->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

		if(ddrval == DD_OK)
			break;

		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
				return;
		}

		if(ddrval != DDERR_WASSTILLDRAWING)
			break;
	}
}


// 直接番号指定で矩形内を塗りつぶす //
FVOID GrpClsEx(RECT *Target, DWORD col)
{
	DDBLTFX		ddbltfx;
	HRESULT		ddrval;

	ZeroMemory(&ddbltfx,sizeof(ddbltfx));
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = col;

	for(;;){
		ddrval = DxObj.Back->Blt(Target,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

		if(ddrval == DD_OK)
			break;

		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
				return;
		}

		if(ddrval != DDERR_WASSTILLDRAWING)
			break;
	}
}


extern void GrpOffsClear(LPDIRECTDRAWSURFACE lpds)
{
	DDBLTFX		ddbltfx;
	HRESULT		ddrval;

	ZeroMemory(&ddbltfx,sizeof(ddbltfx));
	ddbltfx.dwSize      = sizeof(ddbltfx);
	ddbltfx.dwFillColor = (DxObj.Bpp==8) ? RGB256(0,0,0) : 0;

	for(;;){
		ddrval = lpds->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

		if(ddrval == DD_OK)
			break;

		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
				return;
		}

		if(ddrval != DDERR_WASSTILLDRAWING)
			break;
	}
}

static FVOID GrpFlipFS(void)
{
	HRESULT		ddrval;

	// キャプチャーフラグが有効かつ、キャプチャー要求が出ていれば画像を取り込む //
	if (CaptureFlag && (GetAsyncKeyState(VK_SNAPSHOT) & 0x8000))
		// TODO
		//PutSurfaceToBmp(DxObj.Back);
		;

	// パレットを変更する必要があれば、変更だ //
	if(DxObj.bNeedChgPal && DxObj.Bpp==8){
		DxObj.Back->SetPalette(DxObj.Pal);		// 裏画面にパレットをセットする
		DxObj.bNeedChgPal = FALSE;
	}

	for(;;){
		ddrval = DxObj.Prim->Flip(NULL,0);
		if(ddrval == DD_OK)
			break;

		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
				return;
		}

		if(ddrval != DDERR_WASSTILLDRAWING)
			break;
	}
}

static FVOID GrpFlipWnd(void)
{
	HRESULT		ddrval;

	// キャプチャーフラグが有効かつ、キャプチャー要求が出ていれば画像を取り込む //
	if (CaptureFlag && (GetAsyncKeyState(VK_SNAPSHOT) & 0x8000))
		// TODO
		//PutSurfaceToBmp(DxObj.Back);
		;

	for(;;){
		ddrval = DxObj.Prim->Blt(NULL,DxObj.Back,NULL,0,NULL);
		if(ddrval == DD_OK)
			break;

		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
				return;
		}

		if(ddrval != DDERR_WASSTILLDRAWING)
			break;
	}
}

// クリッピングを行うか？ //
FVOID GrpBltEnable(BOOL flag)
{
	if(flag == TRUE)	GrpBlt = STD_GrpBlt;
	else				GrpBlt = DM_GRPBLT;
}

PBOOL GrpClip(RECT *src,int *x,int *y)
{
	int w = src->right - src->left - 1;
	int h = src->bottom - src->top - 1;

	// 完全に外かな？ //
	if((*x)>CLIP_X_MAX||(*y)>CLIP_Y_MAX||(*x)+w<CLIP_X_MIN||(*y)+h<CLIP_Y_MIN) return FALSE;

	// Ｘクリッピング //
	if((*x)<CLIP_X_MIN){
		if((*x)+w>CLIP_X_MAX)
			src->right -= ((*x)+w - CLIP_X_MAX);
		src->left += (CLIP_X_MIN - (*x));
		(*x) = CLIP_X_MIN;
	}
	else if((*x)+w>CLIP_X_MAX)
		src->right -= ((*x)+w - CLIP_X_MAX);

	// Ｙクリッピング //
	if((*y)<CLIP_Y_MIN){
		if((*y)+h>CLIP_Y_MAX)
			src->bottom -= ((*y)+h - CLIP_Y_MAX);
		src->top += (CLIP_Y_MIN - (*y));
		(*y) = CLIP_Y_MIN;
	}
	else if((*y)+h>CLIP_Y_MAX)
		src->bottom -= ((*y)+h - CLIP_Y_MAX);

	return TRUE;
}

// カラーキー＆クリッピング付きＢＭＰ転送 //
// 注意 : src の内容が変更される可能性があります //
PVOID STD_GrpBlt(RECT *src,int x,int y,GRP surf)
{
	HRESULT		ddrval;

	// 完全に範囲外なら何もしない //
	if(!GrpClip(src,&x,&y)) return;

	for(;;){
		ddrval = DxObj.Back->BltFast(x,y,surf,src,DDBLTFAST_SRCCOLORKEY);
		if(ddrval == DD_OK){
			break;
		}
		if(ddrval == DDERR_SURFACELOST){
			if(!GrpRestore())
			return;
		}
		if(ddrval != DDERR_WASSTILLDRAWING){
			return;
		}
	}
}
extern BOOL GrpSetColorKey(LPDIRECTDRAWSURFACE lpds)
{
	DDSURFACEDESC			ddsd;
	HRESULT					dxval;
	DDCOLORKEY				key;
	DWORD					temp;
	void					*ptr;

	// Lock する //
	ZEROMEM(ddsd);ddsd.dwSize = sizeof(ddsd);
	if(lpds->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL)!=DD_OK) return FALSE;
	ptr = ddsd.lpSurface;

	// はじっこの色獲得＆カラーキー準備 //
	switch(DxObj.Bpp){
		case(8):	temp = *(BYTE  *)ptr;	break;
		case(16):	temp = *(WORD  *)ptr;	break;
		case(32):	temp = *(DWORD *)ptr;	break;

		case(24):
			// これでいいのか？ //
			temp = (*(BYTE *)ptr) + ((*(BYTE *)ptr+1)<<8) + ((*(BYTE *)ptr+2)<<16);
		break;
	}

	// Unlock する //
	dxval = lpds->Unlock(NULL);

	// カラーキーをセットする//
	key.dwColorSpaceHighValue = temp;
	key.dwColorSpaceLowValue  = temp;
	lpds->SetColorKey(DDCKEY_SRCBLT,&key);
	if(dxval!=DD_OK) return FALSE;
	else             return TRUE;
}

static BOOL SetColorKey(DXSURFACE *surf)
{
	return GrpSetColorKey(surf->surf);
}

static BOOL OffsRestore(DXSURFACE *surf)
{
	HRESULT		dxret;

	if(surf->surf){
		dxret = surf->surf->Restore();
		if(dxret!=DD_OK) return FALSE;

		if(surf->img) return GrpBMLoadMemory(surf);		// ＢＭＰがロードされている
	}

	return TRUE;
}

static BOOL GrpBMLoadMemory(DXSURFACE *surf)
{
	HRESULT					ddrval;
	BITMAPFILEHEADER		*BMP_file;
	BITMAPINFOHEADER		*BMP_info;
	RGBQUAD					*pal;
	int						i,width,height;
	BYTE					*image;
	HDC						hdc,hdcmem;
	HBITMAP					hbmp;


	BMP_file = (BITMAPFILEHEADER *)surf->img;
	BMP_info = (BITMAPINFOHEADER *)((BMP_file) +   1);
	pal      = (RGBQUAD          *)((BMP_info) +   1);
	image    = (BYTE             *)((pal)      + 256);

	// ヘッダチェッカ //
	if(BMP_file->bfType != MAKEWORD('B','M')) return FALSE;
	if(BMP_info->biBitCount != 8) return FALSE;

	width  = (BMP_info->biWidth+3)&(~3);
	height = BMP_info->biHeight;

	if(DxObj.Bpp==8){
		for(i=0;i<256;i++){
			DxObj.pe[i].peRed   = pal[i].rgbRed;
			DxObj.pe[i].peGreen = pal[i].rgbGreen;
			DxObj.pe[i].peBlue  = pal[i].rgbBlue;
			DxObj.pe[i].peFlags = 0;
		}

		// 例外的な処理 //
		DxObj.pe[0].peRed   = pal[0].rgbRed   = 0;
		DxObj.pe[0].peGreen = pal[0].rgbGreen = 0;
		DxObj.pe[0].peBlue  = pal[0].rgbBlue  = 0;

		if(DxObj.Pal) DxObj.Pal->Release();

		ddrval = DxObj.DD->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256,DxObj.pe,&DxObj.Pal,NULL);
		if(ddrval!=DD_OK) return FALSE;
		surf->surf->SetPalette(DxObj.Pal);
		DxObj.Prim->SetPalette(DxObj.Pal);
		DxObj.Back->SetPalette(DxObj.Pal);
	}

	// DIB から DC を作成 //
	ddrval = surf->surf->GetDC(&hdc);
	if(ddrval!=DD_OK) return FALSE;
	hbmp   = CreateDIBitmap(hdc,BMP_info,CBM_INIT,image,(BITMAPINFO *)BMP_info,DIB_RGB_COLORS);
	hdcmem = CreateCompatibleDC(hdc);
	SelectObject(hdcmem,hbmp);

	BitBlt(hdc,0,0,width,height,hdcmem,0,0,SRCCOPY);

	// 後始末 //
	surf->surf->ReleaseDC(hdc);
	DeleteDC(hdcmem);
	DeleteObject(hbmp);

	// カラーキーをセットする //
	SetColorKey(surf);

	DisableAlphaTable();

	return TRUE;
}

static BOOL CreateZbuffer(XD3DENUM *D3De)
{
	HRESULT				dxret;
	DDSURFACEDESC		ddsd;
	DWORD				mem_flag;

	// Ｚバッファが必要ない場合 //
	if(D3De->Zbuf==0) return TRUE;

	ZEROMEM(ddsd);

	mem_flag = D3De->bIsHW ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY;
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS|DDSD_ZBUFFERBITDEPTH;
	ddsd.dwWidth = 640;
	ddsd.dwHeight = 480;
	ddsd.dwZBufferBitDepth = D3De->Zbuf;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER|mem_flag;
	dxret = DxObj.DD->CreateSurface(&ddsd,&DxObj.Zbuf,NULL);
	if(dxret!=D3D_OK) return FALSE;

	// Ｚバッファをバックバッファに接続 //
	dxret = DxObj.Back->AddAttachedSurface(DxObj.Zbuf);
	if(dxret!=DD_OK) return FALSE;

	return TRUE;
}

static BOOL FAR PASCAL DriverEnum(GUID FAR *lpGUID,LPSTR lpDDesc,LPSTR lpDName,LPVOID lpContext)
{
	HRESULT			dxret;
	LPDIRECTDRAW	lpDD  = NULL;
	LPDIRECT3D2		lpD3D = NULL;
	DDCAPS			DriverCaps,HelCaps;
	XDDRAWENUM		*xenum = &DxEnum[DxEnumNow];

	// これは作成できないね //
	if(DirectDrawCreate(lpGUID,&lpDD,NULL)!=DD_OK) return DDENUMRET_OK;

	// ドライバの能力取得 //
	ZEROMEM(DriverCaps);	DriverCaps.dwSize = sizeof(DDCAPS);
	ZEROMEM(HelCaps);		HelCaps.dwSize    = sizeof(DDCAPS);

	if(lpDD->GetCaps(&DriverCaps,&HelCaps)==DD_OK){
		// ２Ｄでサポートしている解像度をチェック(色数だが...) //
		dxret = lpDD->EnumDisplayModes(0,NULL,(LPVOID)xenum,ModeEnum);

		if(dxret==DD_OK){
			// このドライバで使用できる３Ｄデバイスを列挙         //
			// これが実行できないのはＤＸ３以下であると考えられる //
			// ただし、失敗しても列挙は中断しない                 //
			dxret = lpDD->QueryInterface(IID_IDirect3D2,(LPVOID *)&lpD3D);
			if(dxret==DD_OK)
				lpD3D->EnumDevices(DeviceEnum,(LPVOID)xenum);

			// この時点で xenum->D3DNow==0 なら３Ｄは使用できないことになる //
			xenum->name    = lpDName;
			xenum->lpDDGuid = lpGUID;
			DxEnumNow++;
		}
	}

	RELEASE(lpD3D);
	RELEASE(lpDD);

	return DDENUMRET_OK;
}

static HRESULT FAR PASCAL DeviceEnum(LPGUID lpGUID,LPSTR lpDeviceDesc,LPSTR lpdeviceName,
						LPD3DDEVICEDESC lpHWDesc,LPD3DDEVICEDESC lpHELDesc,LPVOID lpContext)
{
	BOOL		hw_flag;
	DWORD		zbuf;
	DWORD		depth;
	XD3DENUM	*xd3d;
	XDDRAWENUM	*xddraw;

	// ハードウェアかどうか //
	if(lpHWDesc->dcmColorModel) hw_flag = TRUE;
	else                        hw_flag = FALSE;
	//if(!hw_flag) return D3DENUMRET_OK;

	// ３Ｄカードがサポートするビット深度等のチェックする //
	if(hw_flag){
		depth = lpHWDesc->dwDeviceRenderBitDepth;
		zbuf  = lpHWDesc->dwDeviceZBufferBitDepth;

		// 必要な性能をサポートしているか //
		if(!ConfirmDevice3D(lpHWDesc)) return D3DENUMRET_OK;
	}
	else{
		depth = lpHELDesc->dwDeviceRenderBitDepth;
		zbuf  = lpHELDesc->dwDeviceZBufferBitDepth;

		// 必要な性能をサポートしているか //
		if(!ConfirmDevice3D(lpHELDesc)) return D3DENUMRET_OK;
	}

	// ポインタをセット //
	xddraw = (XDDRAWENUM *)lpContext;
	xd3d = (xddraw->D3D) + (xddraw->D3DNow);		// ポインタ＋整数であることに注意

	// どの色深度をサポートしているかな(８ビットは無視する) //
	if((depth & DDBD_16) && xddraw->b16Bit)		xd3d->b16Bit = TRUE;
	if((depth & DDBD_24) && xddraw->b24Bit)		xd3d->b24Bit = TRUE;
	if((depth & DDBD_32) && xddraw->b32Bit)		xd3d->b32Bit = TRUE;

	// Ｚバッファの深さのうちもっとも浅いものを選択する //
	xd3d->Zbuf = 0;							// どのｉｆにもかからないときは０
	if(zbuf & DDBD_32)	xd3d->Zbuf = 32;
	if(zbuf & DDBD_24)	xd3d->Zbuf = 24;
	if(zbuf & DDBD_16)	xd3d->Zbuf = 16;
	//if(zbuf & DDBD_8)	xd3d->Zbuf = 8;

	// ＧＵＩＤ＆デバイス名を保存するぞ //
	xd3d->lpGuid = lpGUID;			// ＧＵＩＤ
	xd3d->name   = lpdeviceName;	// デバイス名
	xd3d->bIsHW  = hw_flag;			// ハードウェアか

	xddraw->D3DNow++;

	return D3DENUMRET_OK;
}

static BOOL ConfirmDevice3D(LPD3DDEVICEDESC lpDesc)
{
	// 半透明をサポートしているかな？ //
	// DX5 の HEL はサポートしていないので結局ハード側でサポートしていなければ //
	// 実行することはできないのでありました。 //
	// でも、DX7-RGB で動いたとしても速度が1/60ぐらいになるので結局使えないけどね... //

	if((lpDesc->dpcTriCaps.dwDestBlendCaps & D3DBLEND_ONE)==0) return FALSE;

	return TRUE;
}

static HRESULT FAR PASCAL ModeEnum(DDSURFACEDESC *pddsd,VOID *pInfo)
{
	XDDRAWENUM		*xenum = (XDDRAWENUM *)pInfo;
	int				color;

	// ６４０Ｘ４８０についてだけ調べるようにする //
	if(pddsd->dwWidth!=640)		return DDENUMRET_OK;
	if(pddsd->dwHeight!=480)	return DDENUMRET_OK;

	// どの色か //
	color  = pddsd->ddpfPixelFormat.dwRGBBitCount;
	switch(color){
		case(8):	xenum->b8Bit  = TRUE;	break;
		case(16):	xenum->b16Bit = TRUE;	break;
		case(24):	xenum->b24Bit = TRUE;	break;
		case(32):	xenum->b32Bit = TRUE;	break;
	}

	return DDENUMRET_OK;
}

static void SetDummyFunc(void)
{
	GrpFlip     = DM_V;
	GrpLock     = DM_V;
	GrpUnlock   = DM_V;
	GrpSetColor = DM_BBB;
	GrpSetAlpha = DM_BB;
	GrpLine     = DM_IIII;
	GrpBox      = DM_IIII;
	GrpCircle   = DM_III;
	GrpCircleF  = DM_III;
	GrpPolygon  = DM_PI;
	GrpBoxA     = DM_IIII;
	GrpCircleA  = DM_III;
	GrpPolygonA = DM_PI;

	GrpFatCircleA = DM_IIII;
	//GrpSetGamma = DM_BBBB;
}

// ダミー関数の実体(かなり手抜き...) //
static FVOID DM_V(void){}
static FVOID DM_BBBB(BYTE a,BYTE b,BYTE c,BYTE d){};
static FVOID DM_BBB(BYTE a,BYTE b,BYTE c){}
static FVOID DM_BB(BYTE a,BYTE b){}
static FVOID DM_III(int a,int b,int c){}
static FVOID DM_IIII(int a,int b,int c,int d){}
static FVOID DM_PI(POINT *p,int n){}

static FBOOL DM_RESTORE(void)
{
	// ダミー関数は確実に成功する //
	return TRUE;
}

static PVOID DM_GRPBLT(RECT *src,int x,int y,GRP surf){}

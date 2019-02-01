/*                                                                           */
/*   Lens.cpp   レンズエフェクト                                             */
/*                                                                           */
/*                                                                           */

#include "Lens.h"



// 秘密の関数 //
static FVOID __DrawLensBall_8(LensInfo *LInfo, int x, int y);
static FVOID __DrawLensBall_16(LensInfo *LInfo, int x, int y);

static BYTE *__LSurfLock(LensInfo *LInfo);
static FVOID __LSurfUnlock(LensInfo *LInfo);

static FBOOL __BackSurfLock(void);
static FVOID __BackSurfUnlock(void);

static BYTE		*__GrpBack = NULL;
static DWORD	__GrpPitch = 0;



// LInfo->gTemp に対して->Lock() を行う  (return : 書き込み先) //
static BYTE *__LSurfLock(LensInfo *LInfo)
{
	DDSURFACEDESC	ddsd;
	HRESULT			ddret;

	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	ddret = LInfo->gTemp->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(ddret != DD_OK) return NULL;

	LInfo->gPitch = ddsd.lPitch;

	return (BYTE *)ddsd.lpSurface;
}


// LInfo->gTemp に対して、->Unlock() を行う //
static FVOID __LSurfUnlock(LensInfo *LInfo)
{
	LInfo->gTemp->Unlock(NULL);
}


static FBOOL __BackSurfLock(void)
{
	DDSURFACEDESC	ddsd;
	HRESULT			ddret;

	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	ddret = DxObj.Back->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(ddret != DD_OK) return FALSE;

	__GrpPitch = ddsd.lPitch;
	__GrpBack  = (BYTE *)ddsd.lpSurface;

	return TRUE;
}


static FVOID __BackSurfUnlock(void)
{
	DxObj.Back->Unlock(NULL);
}


// 半径:r  出っ張り:m  のレンズを作成 //
extern LensInfo *GrpCreateLensBall(int r, int m, GRP Surface)
{
	LensInfo	*NewLens;
	DWORD		*Table;
	DWORD		DataSize;
	DWORD		Diameter;
	DWORD		Pitch;
	BYTE		BitWeight;
	int			s,dx,z,r2,w;
	int			i;

	if(DxObj.Bpp == 16) BitWeight = 2;
	else                BitWeight = 1;

	if(r <= 0) return NULL;		// 半径がおかしい
	if(r <= m) return NULL;		// 出っ張りは半径より小さくせよ！

	NewLens = (LensInfo *)LocalAlloc(LPTR, sizeof(LensInfo));
	if(NewLens == NULL){
		return NULL;
	}

	Diameter = r * 2;
	DataSize = sizeof(DWORD) * Diameter * Diameter;
	NewLens->Data = (DWORD *)LocalAlloc(LPTR, DataSize);
	if(NewLens->Data == NULL){
		LocalFree(NewLens);
		return NULL;
	}

	NewLens->gTemp = Surface;

	NewLens->r      = r;
	NewLens->Height = Diameter;

	__LSurfLock(NewLens);
	__LSurfUnlock(NewLens);
	Pitch = NewLens->gPitch;

	Table = NewLens->Data;
	r2    = (r * r);
	s     = isqrt(r2 - (m * m));

	for(i=-r; i<r; i++){
		// ｘ座標の測定だ //
		dx = (s * s) - (i * i);

		if(dx > 0){		// 円の内部
			dx = isqrt(dx);
			*Table = w = dx*2;				Table++;	// Width
			*Table = (r-dx)*BitWeight;		Table++;	// Dx
		}
		else{			// 円の外部
			w      = 0;
			*Table = 0;		Table++;		// Width
			*Table = 0;		Table++;		// Dx
		}

		while(w--){
			z = (dx - w) * (dx - w);
			z = isqrt(r2 - z - (i*i));

			*Table = (i * m)/z + r;								// ｙ座標
			*Table = (*Table) * Pitch;							// 幅を掛ける
			*Table = (*Table) + (((dx-w) * m)/z + r)*BitWeight;	// ｘ座標

			Table++;
		}
	}

	return NewLens;
}


// GrpLock() 系関数 : レンズボールを描画する //
static FVOID __DrawLensBall_8(LensInfo *LInfo, int x, int y)
{
	DWORD	Height,Width;
	DWORD	*Table;			// テーブル参照用

	BYTE	*Src;		// ８ビット用＆１６ビット用兼用
	BYTE	*Dest;
	BYTE	*p;			// 上に同じ...

	HRESULT		ddrval;
	RECT		src;

	SetRect(&src, x-LInfo->r, y-LInfo->r, x+LInfo->r, y+LInfo->r);
	for(;;){
		ddrval = LInfo->gTemp->BltFast(0, 0,DxObj.Back,&src,DDBLTFAST_NOCOLORKEY);
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

	// (x,y) が中心になるように補正する //
	x -= LInfo->r;
	y -= LInfo->r;

	if(x<0 || y<0 || x+LInfo->Height>639 || y+LInfo->Height>479) return;

	if(!__BackSurfLock()) return;
//	GrpLock();
//	if(Grp_Vram == NULL) return;

	Src = __LSurfLock(LInfo);
	if(Src == NULL){
		__BackSurfUnlock();
		return;
	}

	Table = LInfo->Data;
	Dest  = (__GrpBack + x*sizeof(BYTE) + (y * __GrpPitch));
	//Dest  = Grp_Vram + x + Grp_AddrY[y];

	for(Height=0; Height<LInfo->Height; Height++){
		Width = *Table;						Table++;	// 幅
		p     = (BYTE *)(Dest + (*Table));	Table++;	// 初期シフト値

		while(Width--){
			*p = *(BYTE *)(Src + (*Table));
			p++;
			Table++;
		}

		Dest += __GrpPitch;
	}

	__LSurfUnlock(LInfo);
	__BackSurfUnlock();
	//GrpUnlock();
}


// GrpLock() 系関数 : レンズボールを描画する //
static FVOID __DrawLensBall_16(LensInfo *LInfo, int x, int y)
{
	DWORD	Height,Width;
	DWORD	*Table;			// テーブル参照用

	BYTE	*Src;		// ８ビット用＆１６ビット用兼用
	BYTE	*Dest;
	WORD	*p;			// 上に同じ...

	HRESULT		ddrval;
	RECT		src;

	SetRect(&src, x-LInfo->r, y-LInfo->r, x+LInfo->r, y+LInfo->r);
	for(;;){
		ddrval = LInfo->gTemp->BltFast(0, 0,DxObj.Back,&src,DDBLTFAST_NOCOLORKEY);
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

	// (x,y) が中心になるように補正する //
	x -= LInfo->r;
	y -= LInfo->r;

	if(x<0 || y<0 || x+LInfo->Height>639 || y+LInfo->Height>479) return;

	// 転送先はロックされているかな //
	if(!__BackSurfLock()) return;
//	GrpLock();
//	if(Grp_Vram == NULL) return;

	Src = __LSurfLock(LInfo);
	if(Src == NULL){
		__BackSurfUnlock();
		return;
	}

	Table = LInfo->Data;
	Dest  = (__GrpBack + x*sizeof(WORD) + (y * __GrpPitch));
	//Dest  = Grp_Vram + x + Grp_AddrY[y];

	for(Height=0; Height<LInfo->Height; Height++){
		Width = *Table;						Table++;	// 幅
		p     = (WORD *)(Dest + (*Table));	Table++;	// 初期シフト値

		while(Width--){
			*p = *(WORD *)(Src + (*Table));
			p++;
			Table++;
		}

		Dest += __GrpPitch;
	}

	__LSurfUnlock(LInfo);
	__BackSurfUnlock();
	//GrpUnlock();
}


// GrpLock() 系関数 : レンズボールを描画する //
extern FVOID GrpDrawLensBall(LensInfo *LInfo, int x, int y)
{
	if(DxObj.Bpp == 16) __DrawLensBall_16(LInfo, x, y);
	else                __DrawLensBall_8(LInfo, x, y);
}


// レンズボール用データを破棄する //
extern FVOID GrpReleaseLensBall(LensInfo *LInfo)
{
	if(LInfo == NULL) return;

	if(LInfo->Data) LocalFree(LInfo->Data);

	LocalFree(LInfo);
}

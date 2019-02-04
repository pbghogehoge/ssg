/*                                                                           */
/*   D3_TEX.cpp   テクスチャ管理用関数                                       */
/*                                                                           */
/*                                                                           */

#include "DD_UTY.h"
#include "D3_TEX.h"
#pragma message(PBGWIN_D3_TEX_H)


// 列挙用コールバック関数 //
static HRESULT CALLBACK TextureEnum(DDSURFACEDESC *ddsd,VOID *param);

static DWORD TexDepth(DWORD col);						// ビット深度を調べる

// テクスチャ用 Surface を作成する //
static GRP CreateTexSurface(int width,int height,DWORD MemCaps);

// システムメモリなテクスチャに画像イメージを転送 //
static BOOL BMP2SysMem(GRP surf,BYTE *img);


// ローカル変数 //
static DDPIXELFORMAT TextureFormat;

// グローバル変数 //
DXTEXTURE DxTex[TEXTURE_MAX];


FBOOL TexInit(void)
{
	HRESULT dxret;
	BOOL	flag = FALSE;

	//DebugSetup("TexInfo.txt");
	dxret = DxObj.D3Dev->EnumTextureFormats(TextureEnum,(LPVOID)&flag);
	if(dxret!=D3D_OK) return FALSE;
	//DebugCleanup();

	memset(DxTex,0,TEXTURE_MAX*sizeof(DXTEXTURE));

	return flag;
}

/*
FBOOL TexCreateTexture(char *filename,DWORD txid)
{
	HRESULT				dxret;
	DXTEXTURE			*p;
	GRP					temp = NULL;
	FILE				*fp  = NULL;
	long				size;
	int					width,height;
	LPDIRECT3DTEXTURE2	DevTexture = NULL;
	LPDIRECT3DTEXTURE2	MemTexture = NULL;


	if(txid>=TEXTURE_MAX) return FALSE;

	try{
		// すでに作成されている場合 //
		p = &DxTex[txid];
		if(p->surf) throw("作成済み");

		// ＢＭＰイメージをメモリに送る //
		fp = fopen(filename,"rb");
		if(fp==NULL) throw("ファイルにゃい");

		size = filelength(fileno(fp));
		p->img = (BYTE *)LocalAlloc(LPTR,size);
		if(p->img==NULL) throw("メモリ足りん");

		fread(p->img,1,size,fp);
		fclose(fp);

		width  = TexGetWidth(p->img);
		height = TexGetHeight(p->img);

		// 作業用 Surface //
		temp = CreateTexSurface(width,height,DDSCAPS_SYSTEMMEMORY);
		if(temp==0) throw("CTexSurf");
		if(!BMP2SysMem(temp,p->img)) throw("BMP->MEM");

		// テクスチャ用 Surface //
		p->surf = CreateTexSurface(width,height,DDSCAPS_VIDEOMEMORY);
		if(p->surf==0) throw("CTexSurf(VRAM)");

		dxret = temp->QueryInterface(IID_IDirect3DTexture2,(void **)&MemTexture);
		if(dxret!=DD_OK) throw("QI(Mem)");

		dxret = p->surf->QueryInterface(IID_IDirect3DTexture2,(void **)&DevTexture);
		if(dxret!=DD_OK) throw("QI");

		// ＶＲＡＭのほうにＬＯＡＤする //
		dxret = DevTexture->Load(MemTexture);
		if(dxret!=D3D_OK) throw("TexなLoad");

		dxret = DevTexture->GetHandle(DxObj.D3Dev,&(p->hTexture));
		if(dxret!=D3D_OK) throw("GetHandle");

		RELEASE(MemTexture);
		RELEASE(DevTexture);
		RELEASE(temp);
	}
	catch(char *error){
		RELEASE(MemTexture);
		RELEASE(DevTexture);
		RELEASE(temp);
		RELEASE(p->surf);
		if(p->img) LocalFree(p->img);
		if(fp) fclose(fp);

		ErrInsert(error);
		return FALSE;
	}

	return TRUE;
}
*/

FVOID TexReleaseTexture(DWORD txid)
{
	DXTEXTURE *p;

	p = &DxTex[txid];

	LocalFree(p->img);		// イメージ解放
	RELEASE(p->surf);		// テクスチャ用Surface 解放
	p->hTexture = NULL;		// ハンドルを使用不能にする(というか上の奴ですでに使用できない)
}

FVOID TexSetTexture(DWORD txid)
{
	if(DxTex[txid].hTexture)
		DxObj.D3Dev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,DxTex[txid].hTexture);
}

static DWORD TexDepth(DWORD col)
{
	DWORD temp = 0;

	if(!col) return 0;

	while((col & 0x01)==0) col>>=1;
	while((col & 0x01)==1) col>>=1,temp++;

	return temp;
}

static HRESULT CALLBACK TextureEnum(DDSURFACEDESC *ddsd,VOID *param)
{
	DDPIXELFORMAT	ddpf = ddsd->ddpfPixelFormat;
	DWORD			r,g,b,a;

	if(!(ddpf.dwFlags&DDPF_RGB)) return DDENUMRET_OK;

	r = TexDepth(ddpf.dwRBitMask);
	g = TexDepth(ddpf.dwGBitMask);
	b = TexDepth(ddpf.dwBBitMask);
	a = TexDepth(ddpf.dwRGBAlphaBitMask);
	//char buf[1000];
	//sprintf(buf,"R:%2d  G:%2d  B:%2d  A:%2d",r,g,b,a);
	//DebugOut(buf);

	// 手抜き判定処理 //
	if(a==0 && ddpf.dwRGBBitCount>8){
		TextureFormat = ddpf;
		*(BOOL *)param = TRUE;
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

static GRP CreateTexSurface(int width,int height,DWORD MemCaps)
{
	DDSURFACEDESC	ddsd;
	HRESULT			dxret;
	GRP				temp;

	ZEROMEM(ddsd);
	ddsd.dwSize  = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;

	// ハードウェアなら DDSCAPS_VIDEOMEMORY そうでなければ DDSCAPS_SYSTEMMEMORY を加える //
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE|DDSCAPS_ALLOCONLOAD|MemCaps;

	ddsd.ddpfPixelFormat = TextureFormat;
	ddsd.dwWidth  = width;
	ddsd.dwHeight = height;

	dxret = DxObj.DD->CreateSurface(&ddsd,&temp,NULL);

	return (dxret==DD_OK) ? temp : NULL;
}

static BOOL BMP2SysMem(GRP surf,BYTE *img)
{
	HRESULT					ddrval;
	BITMAPFILEHEADER		*BMP_file;
	BITMAPINFOHEADER		*BMP_info;
	RGBQUAD					*pal;
	//PALETTEENTRY			pe[256];
	int						width,height;
	BYTE					*image;
	HDC						hdc,hdcmem;
	HBITMAP					hbmp;


	BMP_file = (BITMAPFILEHEADER *)img;
	BMP_info = (BITMAPINFOHEADER *)((BMP_file) +   1);
	pal      = (RGBQUAD          *)((BMP_info) +   1);
	image    = (BYTE             *)((pal)      + 256);

	// ヘッダチェッカ //
	if(BMP_file->bfType != MAKEWORD('B','M')) return FALSE;
	if(BMP_info->biBitCount != 8) return FALSE;

	width  = (BMP_info->biWidth+3)&(~3);
	height = BMP_info->biHeight;

	// DIB から DC を作成 //
	ddrval = surf->GetDC(&hdc);
	if(ddrval!=DD_OK) return FALSE;
	hbmp   = CreateDIBitmap(hdc,BMP_info,CBM_INIT,image,(BITMAPINFO *)BMP_info,DIB_RGB_COLORS);
	hdcmem = CreateCompatibleDC(hdc);
	SelectObject(hdcmem,hbmp);

	BitBlt(hdc,0,0,width,height,hdcmem,0,0,SRCCOPY);

	// 後始末 //
	surf->ReleaseDC(hdc);
	DeleteDC(hdcmem);
	DeleteObject(hbmp);
/*
	// カラーキーをセットする //
	SetColorKey(surf);
*/
	return TRUE;
}

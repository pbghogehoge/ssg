/*                                                                           */
/*   FontUty.cpp   フォントの処理いろいろ                                    */
/*                                                                           */
/*                                                                           */

#include "FONTUTY.h"

// ヒミツの関数 //
void __FillExpoint(EXPOINT *p, int x, int y, int w, int h);



// 16x16 透過フォントで文字列出力(高速) //
extern FVOID GrpPut16(int x,int y,const char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	// GrTama における文字の並びは次のようになっている。 //
	// (変更する可能性もあるので十分に注意すること)      //
	// ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789              //
	// abcdefghijklmnopqrstuvwxyz!?#\<>=,+-              //

	for(;(*s)!='\0';s++,x+=14){
		if((*s)>='A' && (*s)<='Z'){
			BltSetRect(&src,((*s)-'A')<<4,480-32,16,16);
		}
		else if((*s)>='a' && (*s)<='z'){
			BltSetRect(&src,((*s)-'a')<<4,480-16,16,16);
		}
		else if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<4)+416,480-32,16,16);
		}
		else{
			switch(*s){
			case('!'):	BltSetRect(&src,((0)<<4)+416,480-16,16,16);	break;
			case('?'):	BltSetRect(&src,((1)<<4)+416,480-16,16,16);	break;
			case('#'):	BltSetRect(&src,((2)<<4)+416,480-16,16,16);	break;
			case('\\'):	BltSetRect(&src,((3)<<4)+416,480-16,16,16);	break;
			case('<'):	BltSetRect(&src,((4)<<4)+416,480-16,16,16);	break;
			case('>'):	BltSetRect(&src,((5)<<4)+416,480-16,16,16);	break;
			case('='):	BltSetRect(&src,((6)<<4)+416,480-16,16,16);	break;
			case(','):	BltSetRect(&src,((7)<<4)+416,480-16,16,16);	break;
			case('+'):	BltSetRect(&src,((8)<<4)+416,480-16,16,16);	break;
			case('-'):	BltSetRect(&src,((9)<<4)+416,480-16,16,16);	break;
			default:	continue;
			}
		}

		tx = x;ty = y;
		if(tx>=0 && tx<630)	// 安全対策???
			GrpBltX(&src,tx,ty,GrTama);
	}
}

// 16x16 透過フォントで文字列出力(高速) //
extern FVOID GrpPut16c(int x,int y,char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	// GrTama における文字の並びは次のようになっている。 //
	// (変更する可能性もあるので十分に注意すること)      //
	// ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789              //
	// abcdefghijklmnopqrstuvwxyz!?#\<>=,+-              //

	for(;(*s)!='\0';s++,x+=14){
		if((*s)>='A' && (*s)<='Z'){
			BltSetRect(&src,((*s)-'A')<<4,480-32,16,16);
		}
		else if((*s)>='a' && (*s)<='z'){
			BltSetRect(&src,((*s)-'a')<<4,480-16,16,16);
		}
		else if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<4)+416,480-32,16,16);
		}
		else{
			switch(*s){
			case('!'):	BltSetRect(&src,((0)<<4)+416,480-16,16,16);	break;
			case('?'):	BltSetRect(&src,((1)<<4)+416,480-16,16,16);	break;
			case('#'):	BltSetRect(&src,((2)<<4)+416,480-16,16,16);	break;
			case('\\'):	BltSetRect(&src,((3)<<4)+416,480-16,16,16);	break;
			case('<'):	BltSetRect(&src,((4)<<4)+416,480-16,16,16);	break;
			case('>'):	BltSetRect(&src,((5)<<4)+416,480-16,16,16);	break;
			case('='):	BltSetRect(&src,((6)<<4)+416,480-16,16,16);	break;
			case(','):	BltSetRect(&src,((7)<<4)+416,480-16,16,16);	break;
			case('+'):	BltSetRect(&src,((8)<<4)+416,480-16,16,16);	break;
			case('-'):	BltSetRect(&src,((9)<<4)+416,480-16,16,16);	break;
			default:	continue;
			}
		}

		tx = x;ty = y;
		if(tx>=0 && tx<630)	// 安全対策???
			GrpBlt(&src,tx,ty,GrTama);
	}
}

// 上と同じだが、ｘ移動幅が１６ //
extern FVOID GrpPut16c2(int x,int y,char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	// GrTama における文字の並びは次のようになっている。 //
	// (変更する可能性もあるので十分に注意すること)      //
	// ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789              //
	// abcdefghijklmnopqrstuvwxyz!?#\<>=,+-              //

	for(;(*s)!='\0';s++,x+=16){
		if((*s)>='A' && (*s)<='Z'){
			BltSetRect(&src,((*s)-'A')<<4,480-32,16,16);
		}
		else if((*s)>='a' && (*s)<='z'){
			BltSetRect(&src,((*s)-'a')<<4,480-16,16,16);
		}
		else if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<4)+416,480-32,16,16);
		}
		else{
			switch(*s){
			case('!'):	BltSetRect(&src,((0)<<4)+416,480-16,16,16);	break;
			case('?'):	BltSetRect(&src,((1)<<4)+416,480-16,16,16);	break;
			case('#'):	BltSetRect(&src,((2)<<4)+416,480-16,16,16);	break;
			case('\\'):	BltSetRect(&src,((3)<<4)+416,480-16,16,16);	break;
			case('<'):	BltSetRect(&src,((4)<<4)+416,480-16,16,16);	break;
			case('>'):	BltSetRect(&src,((5)<<4)+416,480-16,16,16);	break;
			case('='):	BltSetRect(&src,((6)<<4)+416,480-16,16,16);	break;
			case(','):	BltSetRect(&src,((7)<<4)+416,480-16,16,16);	break;
			case('+'):	BltSetRect(&src,((8)<<4)+416,480-16,16,16);	break;
			case('-'):	BltSetRect(&src,((9)<<4)+416,480-16,16,16);	break;
			default:	continue;
			}
		}

		tx = x;ty = y;
		//if(tx>=0 && tx<630)	// 安全対策???
		GrpBlt(&src,tx,ty,GrTama);
	}
}

// 16x16 透過フォントで文字出力(クリッピング有) //
extern FVOID GrpPutc(int x,int y,char c)
{
	RECT	src;

	if(c>='A' && c<='Z'){
		BltSetRect(&src,(c-'A')<<4,480-32,16,16);
	}
	else if(c>='a' && c<='z'){
		BltSetRect(&src,(c-'a')<<4,480-16,16,16);
	}
	else if(c>='0' && c<='9'){
		BltSetRect(&src,((c-'0')<<4)+416,480-32,16,16);
	}
	else{
		switch(c){
		case('!'):	BltSetRect(&src,((0)<<4)+416,480-16,16,16);	break;
		case('?'):	BltSetRect(&src,((1)<<4)+416,480-16,16,16);	break;
		case('#'):	BltSetRect(&src,((2)<<4)+416,480-16,16,16);	break;
		case('\\'):	BltSetRect(&src,((3)<<4)+416,480-16,16,16);	break;
		case('<'):	BltSetRect(&src,((4)<<4)+416,480-16,16,16);	break;
		case('>'):	BltSetRect(&src,((5)<<4)+416,480-16,16,16);	break;
		case('='):	BltSetRect(&src,((6)<<4)+416,480-16,16,16);	break;
		case(','):	BltSetRect(&src,((7)<<4)+416,480-16,16,16);	break;
		case('+'):	BltSetRect(&src,((8)<<4)+416,480-16,16,16);	break;
		case('-'):	BltSetRect(&src,((9)<<4)+416,480-16,16,16);	break;
		default:	return;
		}
	}

	GrpBlt(&src,x,y,GrTama);
}

// 05x07 べた貼りフォント //
extern FVOID GrpPut57(int x,int y,char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	for(;(*s)!='\0';s++,x+=6){
		if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<3)+128,80,5,7);
		}
		else{
			continue;
		}

		tx = x;ty = y;
		if(tx>=0 && tx<630)	// 安全対策???
			GrpBlt(&src,tx,ty,GrTama);
	}
}

// 07x11 音楽室用フォント
extern FVOID GrpPut7B(int x,int y,char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	for(;(*s)!='\0';s++,x+=8){
		if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<3)+184,456,7,11);
		}
		else if((*s)=='-'){
			BltSetRect(&src,264,456,7,11);
		}
		else if((*s)==':'){
			BltSetRect(&src,264+8,456,7,11);
		}
		else{
			continue;
		}

		tx = x;ty = y;
		if(tx>=0 && tx<630)	// 安全対策???
			GrpBltX(&src,tx,ty,GrEnemy);
	}
}

// 得点アイテムのスコアを描画 //
extern FVOID GrpPutScore(int x,int y,char *s)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	for(;(*s)!='\0';s++,x+=6){
		if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,(((*s)-'0')<<3)+128,88,5,7);
		}
		else{
			continue;
		}

		tx = x;ty = y;
		if(tx>=0 && tx<630)	// 安全対策???
			GrpBlt(&src,tx,ty,GrTama);
	}
}

// MIDI 用フォントを描画する //
extern FVOID GrpPutMidNum(int x,int y,int n)
{
	char	buf[10];
	int		i;
	RECT	src;

	sprintf(buf,"%3d",n);

	// n = 1Byte ならば３桁以内に収まるハズ //
	for(i=0;i<3;i++,x+=5){
		if(buf[i]>='0' && buf[i]<='9'){
			BltSetRect(&src,80+(buf[i]-'0')*4,432,4,5);
			GrpBlt(&src,x,y,GrEnemy);
		}
		if(buf[i]=='-'){
			BltSetRect(&src,80+10*4,432,4,5);
			GrpBlt(&src,x,y,GrEnemy);
		}
	}
}

// フォントをセットする //
extern FVOID InitFonts(void)
{
	// でかいフォント //
	WinGrpInfo.LargeFont = CreateFont(24,12,0,0,500,FALSE,0,0,SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH,"ＭＳ ゴシック");

	// 並なフォント //
	WinGrpInfo.NormalFont = CreateFont(16,8,0,0,400,FALSE,0,0,SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH,"ＭＳ ゴシック");

	// ちっこいフォント //
	WinGrpInfo.SmallFont = CreateFont(14,7,0,0,400,FALSE,0,0,SHIFTJIS_CHARSET,
		OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH,"ＭＳ ゴシック");
}

// フォントを解放する //
extern FVOID CleanupFont(void)
{
	if(WinGrpInfo.LargeFont)	DeleteObject(WinGrpInfo.LargeFont);
	if(WinGrpInfo.NormalFont)	DeleteObject(WinGrpInfo.NormalFont);
	if(WinGrpInfo.SmallFont)	DeleteObject(WinGrpInfo.SmallFont);
}

// グラデーション付きフォントを描画する //
extern FVOID DrawGrdFont(GRP surf,char *s)
{
	HDC			hdc;
	HFONT		oldfont;
	int			i,j;
	int			len;
	DWORD		temp,old;

	len = strlen(s)*16;

	surf->GetDC(&hdc);

	// ここら辺は、一種の常套手段か？ //
	old = GetPixel(hdc,0,0);
	SetPixel(hdc,0,0,RGB(255,255,255));
	temp = GetPixel(hdc,0,0);
	SetPixel(hdc,0,0,old);

	oldfont = (HFONT)SelectObject(hdc,WinGrpInfo.NormalFont);
	SetBkMode(hdc,TRANSPARENT);

	SetTextColor(hdc,RGB(0,0,128));			TextOut(hdc,2,2,s,len);
	SetTextColor(hdc,RGB(0,0,128));			TextOut(hdc,1,2,s,len);
	SetTextColor(hdc,RGB(255,255,255));		TextOut(hdc,1,1,s,len);
	SetTextColor(hdc,RGB(255,255,255));		TextOut(hdc,0,1,s,len);

#define FSIZE 8
	//COLORREF temp = GetPixel(hdc,0,0);
	for(i=1;i<len;i++){
		for(j=1;j<16;j++){
			if(GetPixel(hdc,i,j)==temp)//RGB(255,255,255))
				SetPixel(hdc,i,j,RGB(255+FSIZE-j*FSIZE,255+FSIZE-j*FSIZE,255));
		}
	}

	SelectObject(hdc,oldfont);
	surf->ReleaseDC(hdc);
}


// フォント生成 //
ExtraFontInfo *CreateExtraFont(GRP Surface, RECT *pSrc)
{
	ExtraFontInfo		*pInfo;
	int					MaxSize;
	DWORD				x, y;
	DWORD				Pitch, Width, Height;
	DWORD				TransID;
	void				*Target;

	BYTE		*pByte;

	DDSURFACEDESC		ddsd;
	HRESULT				ddret;

	// 俺様メモリの確保ぉ //
	pInfo = (ExtraFontInfo *)LocalAlloc(LPTR, sizeof(ExtraFontInfo));
	if(pInfo == NULL) return NULL;

	// 幅と高さを代入しましょう //
	pInfo->Width  = Width  = (pSrc->right)  - (pSrc->left);
	pInfo->Height = Height = (pSrc->bottom) - (pSrc->top);

	// 画像格納に必要なサイズを求める //
	MaxSize = pInfo->Width * pInfo->Height * sizeof(EXPOINT);

	pInfo->Data = (EXPOINT *)LocalAlloc(LPTR, MaxSize);
	if(pInfo->Data == NULL){
		LocalFree(pInfo);
		return NULL;
	}

	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	ddret = Surface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(ddret != DD_OK){
		DeleteExtraFont(pInfo);
		return NULL;
	}

	Pitch  = ddsd.lPitch;
	Target = ddsd.lpSurface;

	// 透過色をセットしましょう //
	if(DxObj.Bpp == 16)	TransID = *(WORD *)Target;
	else                TransID = *(BYTE *)Target;

	pByte = (BYTE *)Target;

	pInfo->DataSize = 0;

	for(y=0; y<Height; y++){
		for(x=0; x<Width; x++){
			if(DxObj.Bpp == 16){
				if(*(WORD *)(&pByte[(x+pSrc->left)*2 + (y+pSrc->top) * Pitch]) != TransID){
					pInfo->Data[pInfo->DataSize].c.Word = *(WORD *)(&pByte[(x+pSrc->left)*2 + (y+pSrc->top) * Pitch]);
					__FillExpoint(&(pInfo->Data[pInfo->DataSize]), x, y, Width, Height);
					pInfo->DataSize++;
				}
			}
			else{
				if(pByte[x+pSrc->left + (y+pSrc->top) * Pitch] != TransID){
					pInfo->Data[pInfo->DataSize].c.Byte = pByte[x+pSrc->left + (y+pSrc->top) * Pitch];
					__FillExpoint(&(pInfo->Data[pInfo->DataSize]), x, y, Width, Height);
					pInfo->DataSize++;
				}
			}
		}
	}

	Surface->Unlock(NULL);

	return pInfo;
}


void __FillExpoint(EXPOINT *p, int x, int y, int w, int h)
{
	x -= w / 2;
	y -= h / 2;

	p->x = x;
	p->y = y;

	p->d = atan8(x * 64, y * 64);
	p->l = isqrt(x * x + y * y);
}


// フォント削除 //
void DeleteExtraFont(ExtraFontInfo *pFont)
{
	if(pFont == NULL) return;

	if(pFont->Data) LocalFree(pFont->Data);

	LocalFree(pFont);
}


// フォント描画 //
void DrawExtraFont(ExtraFontInfo *pFont, int ox, int oy, int t)
{
	int		i;
	int		x, y;

	BYTE	d;
	int		l1, l2;

	DDSURFACEDESC		ddsd;
	HRESULT				ddret;
	DWORD				Pitch;

	BYTE		*pByte;
	EXPOINT		*pData;

	if(pFont == NULL) return;


	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	ddret = DxObj.Back->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(ddret != DD_OK) return;

	Pitch  = ddsd.lPitch;
	pData  = pFont->Data;
	pByte  = (BYTE *)ddsd.lpSurface;

	if(DxObj.Bpp == 16){
		for(i=0; i<pFont->DataSize; i++, pData++){
			if(t){
				//x = ox + pData->x;
				//y = oy + pData->y;

				//d  = pData->d + (pData->l * (sinl(t-64,128)+128)) / 64;
				d  = pData->d + (sinl(t-64, 64)+64) * (pData->l) / 32;
				l1 = (pData->l+60) * cosl(t, 200) / (200) - 60;
				l2 = (pData->l-10) * cosl(t, 200) / (200) + 10;

				x = ox + cosl(d, min(100, abs(l1)) );
				y = oy + sinl(d, min(100, abs(l2)) );
			}
			else{
				x = ox + pData->x;
				y = oy + pData->y;
			}
			if(x < 0 || x > 639 || y < 0 || y > 399) continue;

			*(WORD *)(&pByte[x*2 + y * Pitch]) = pData->c.Word;
		}
	}
	else{
		for(i=0; i<pFont->DataSize; i++, pData++){
			if(t){
				//x = ox + pData->x;
				//y = oy + pData->y;

				//d  = pData->d + (pData->l * (sinl(t-64,128)+128)) / 64;
				d  = pData->d + (sinl(t-64, 64)+64) * (pData->l) / 32;
				l1 = (pData->l+60) * cosl(t, 200) / (200) - 60;
				l2 = (pData->l-10) * cosl(t, 200) / (200) + 10;

				x = ox + cosl(d, min(100, abs(l1)) );
				y = oy + sinl(d, min(100, abs(l2)) );
			}
			else{
				x = ox + pData->x;
				y = oy + pData->y;
			}
			if(x < 0 || x > 639 || y < 0 || y > 399) continue;

			pByte[x + y * Pitch] = pData->c.Byte;
		}
	}

	DxObj.Back->Unlock(NULL);
}


// フォント描画 //
void DrawExtraFont2(ExtraFontInfo *pFont, int ox, int oy, int t)
{
	int		i;
	int		x, y;

	BYTE	d;
	int		l1, l2;

	DDSURFACEDESC		ddsd;
	HRESULT				ddret;
	DWORD				Pitch;

	BYTE		*pByte;
	EXPOINT		*pData;

	if(pFont == NULL) return;


	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	ddret = DxObj.Back->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	if(ddret != DD_OK) return;

	Pitch  = ddsd.lPitch;
	pData  = pFont->Data;
	pByte  = (BYTE *)ddsd.lpSurface;

	if(DxObj.Bpp == 16){
		for(i=0; i<pFont->DataSize; i++, pData++){
			if(t){
				//x = ox + pData->x;
				//y = oy + pData->y;

				//d  = pData->d + (pData->l * (sinl(t-64,128)+128)) / 64;
				d  = pData->d + (sinl(t-64, 64)+64) * (pData->l) / 32;
				l1 = (pData->l+60) * cosl(-t, 256) / (256) - 60;
				l2 = (pData->l-10) * cosl(-t, 256) / (256) + 10;

				x = ox + cosl(d, l1);
				y = oy + sinl(d, l2);
			}
			else{
				x = ox + pData->x;
				y = oy + pData->y;
			}
			if(x < 0 || x > 639 || y < 0 || y > 399) continue;

			*(WORD *)(&pByte[x*2 + y * Pitch]) = pData->c.Word;
		}
	}
	else{
		for(i=0; i<pFont->DataSize; i++, pData++){
			if(t){
				//x = ox + pData->x;
				//y = oy + pData->y;

				//d  = pData->d + (pData->l * (sinl(t-64,128)+128)) / 64;
				d  = pData->d + (sinl(t-64, 64)+64) * (pData->l) / 32;
				l1 = (pData->l+60) * cosl(t, 256) / (256) - 60;
				l2 = (pData->l-10) * cosl(t, 256) / (256) + 10;

				x = ox + cosl(d, l1);
				y = oy + sinl(d, l2);
			}
			else{
				x = ox + pData->x;
				y = oy + pData->y;
			}
			if(x < 0 || x > 639 || y < 0 || y > 399) continue;

			pByte[x + y * Pitch] = pData->c.Byte;
		}
	}

	DxObj.Back->Unlock(NULL);
}

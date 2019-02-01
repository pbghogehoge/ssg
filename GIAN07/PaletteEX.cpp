/*
 *   PaletteEx.cpp   : 特殊パレット処理
 *
 */

#include "PaletteEx.h"


// パレットをｕｖ固定で変換 //
void ConvertPalette(PALETTEENTRY *pal, BYTE u, BYTE v)
{
	double	ty, tu, tv;
	double	r, g, b;
	int		i;

	tu = u;
	tv = v;

	// これもダメだね //
	for(i=0; i<256; i++, pal++){
		ty = (double)(pal->peRed)*0.299
		   + (double)(pal->peGreen)*0.587
		   + (double)(pal->peBlue)*0.144;

		r = ty + tv * 1.4026;
		g = ty - tu * 0.3444 + tv * 0.7114;
		b = ty + tu * 1.733;

		if(r < 0)        r = 0;
		else if(r > 255) r = 255;

		if(g < 0)        g = 0;
		else if(g > 255) g = 255;

		if(b < 0)        b = 0;
		else if(b > 255) b = 255;

		pal->peRed   = r;
		pal->peGreen = g;
		pal->peBlue  = b;
	}
}


// パレットをαで合成する(p1 = p1*a/255 + p2*(255-a)/255 //
void BlendPalette(PALETTEENTRY *p1, PALETTEENTRY *p2, BYTE a)
{
	int		i;
	DWORD	temp;

	// 時間がないから、今回はこれで我慢 //
	for(i=0; i<256; i++, p1++, p2++){
		temp = (((DWORD)p1->peRed) * a + ((DWORD)p2->peRed) * (255-a)) / 255;
		p1->peRed = (temp > 255) ? 255 : temp;

		temp = (((DWORD)p1->peGreen) * a + ((DWORD)p2->peGreen) * (255-a)) / 255;
		p1->peGreen = (temp > 255) ? 255 : temp;

		temp = (((DWORD)p1->peBlue) * a + ((DWORD)p2->peBlue) * (255-a)) / 255;
		p1->peBlue = (temp > 255) ? 255 : temp;
	}
}

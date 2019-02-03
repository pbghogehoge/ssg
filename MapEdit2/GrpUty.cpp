/*                                                                           */
/*   GrpUty.cpp   グラフィック補助関数                                       */
/*                                                                           */
/*                                                                           */

#include "MapEdit.h"
#include "GrpUty.h"
#include "DD_GRP3D.h"


void GrpPuts(char *s,int x,int y)
{
	RECT	src;
	int		sx;

	sx = x;

	for(;(*s)!='\0';s++,x+=9){
		if((*s)>='a' && (*s)<='z'){
			src.top    = 40;
			src.bottom = src.top + 16;
			src.left   = 256 + ((*s)-'a')*9;
			src.right  = src.left + 9;
		}
		else if((*s)>='A' && (*s)<='Z'){
			src.top    = 40 + 16;
			src.bottom = src.top + 16;
			src.left   = 256 + ((*s)-'A')*9;
			src.right  = src.left + 9;
		}
		else if((*s)>='0' && (*s)<='9'){
			src.top    = 40 + 32;
			src.bottom = src.top + 16;
			src.left   = 256 + ((*s)-'0')*9;
			src.right  = src.left + 9;
		}
		else{
			switch(*s){
				case('+'):	src.left = 256;			break;
				case('-'):	src.left = 256  + 9;	break;
				case('*'):	src.left = 256  + 18;	break;
				case('/'):	src.left = 256  + 27;	break;
				case('('):	src.left = 256  + 36;	break;
				case(')'):	src.left = 256  + 45;	break;
				case(','):	src.left = 256  + 54;	break;
				case('.'):	src.left = 256  + 63;	break;
				case('!'):	src.left = 256  + 72;	break;
				case('?'):	src.left = 256  + 81;	break;
				case('\"'):	src.left = 256  + 90;	break;
				case('#'):	src.left = 256  + 99;	break;
				case('$'):	src.left = 256  + 108;	break;
				case('%'):	src.left = 256  + 117;	break;
				case('&'):	src.left = 256  + 126;	break;
				case('\''):	src.left = 256  + 135;	break;
				case('\\'):	src.left = 256  + 144;	break;

				case('='):	src.left = 256  + 153;	break;
				case(':'):	src.left = 256  + 162;	break;
				case(';'):	src.left = 256  + 171;	break;
				case('<'):	src.left = 256  + 180;	break;
				case('>'):	src.left = 256  + 189;	break;
				case('_'):	src.left = 256  + 198;	break;

				case('\n'):		// 改行(x=sx-9,y=y+16とか)
					x = sx-9;
					y = y+16;
				default:
				continue;
			}
			src.top    = 40 + 48;
			src.bottom = src.top + 16;
			src.right  = src.left + 9;
		}

		GrpBltX(&src,x,y,GrTama);
	}
}

void GrpPut8(char *s,int x,int y)
{
	RECT	src;
	int		sx,tx,ty;

	sx = x;

	for(;(*s)!='\0';s++,x+=8){
		if((*s)>='A' && (*s)<='Z'){
			BltSetRect(&src,256+(((*s)-'A')<<3),32,8,8);
		}
		else if((*s)>='a' && (*s)<='z'){
			BltSetRect(&src,256+(((*s)-'a')<<3),32,8,8);
		}
		else if((*s)>='0' && (*s)<='9'){
			BltSetRect(&src,640-17*8+20+(((*s)-'0')<<3),72+16+16,8,8);
		}
		else{
			continue;
		}

		tx = x;ty = y;
		GrpBlt(&src,tx,ty,GrTama);
	}
}

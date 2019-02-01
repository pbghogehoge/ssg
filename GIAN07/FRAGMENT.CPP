/*************************************************************************************************/
/*   FRAGMENT.C   破片処理関数群                                                                 */
/*                                                                                               */
/*************************************************************************************************/

#include "GIAN.H"


FRAGMENT_DATA	Fragment[FRAGMENT_MAX];		// 破片データ管理用構造体
int				FragmentPtr = 0;			// 次に破片データを挿入する位置


static void pascal _FDraw(FRAGMENT_DATA *f);


void fragment_set(int x,int y,BYTE cmd)
{
	int				i;
	int				l;
	BYTE			d;
	FRAGMENT_DATA	*f = Fragment+FragmentPtr;

	if(cmd==FRG_ESCAPE){
		for(i=0;i<FRAGMENT_MAX;i++){
			f = Fragment+i;
			if(f->count){
				f->vx = (f->x-x)/16;f->count;
				f->vy = (f->y-y)/16;f->count;
			}
		}
	}
	else if(cmd==FRG_APPROACH){
		for(i=0;i<FRAGMENT_MAX;i++){
			f = Fragment+i;
			if(f->count){
				f->vx = (x-f->x)/f->count;
				f->vy = (x-f->y)/f->count;
			}
		}
	}

	f->cmd   = cmd;
	f->x     = x;
	f->y     = y;

	switch(cmd){
		case(FRG_HIT):
			d = rnd()&0xff;
			l = 64 + rnd()%(64*3);
			f->count = 24;
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_EVADE):
			d = rnd()&0xff;
			l = 64*4 + rnd()%(64*3);
			f->count = 24;
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_SMOKE):
			f->count = 24;
			f->vx    = 0;//cosl((BYTE)rnd(),rnd()&0xff);//cosl((BYTE)rnd()%256,64*6);
			f->vy    = 0;//sinl((BYTE)rnd(),rnd()&0xff);//sinl((BYTE)rnd()%256,64*6);
		break;

		case(FRG_STAR1):
			f->count = 64;
			d = rnd()&0xff;
			l = 64*5 + rnd()%(64*3);
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_STAR2):
			f->count = 64;
			d = rnd()&0xff;
			l = 64*4 + rnd()%(64*3);
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_STAR3):
			f->count = 64;
			d = -64 - 48 + rnd()%96;
			l = 64*6 + rnd()%(64*4);
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_HEART):
			f->count = 105;
			d = rnd()&0xff;
			l = 64*2 + rnd()%(64*5);
			f->vx    = cosl(d,l);
			f->vy    = sinl(d,l);
		break;

		case(FRG_FATCIRCLE):
			f->count = 60;
			f->vx    = 0;
			f->vy    = 0;
		break;

		default:
			// バグだけど... //
		break;
	}

	FragmentPtr = (FragmentPtr+1)%FRAGMENT_MAX;
}

void fragment_move(void)
{
	int i;
	FRAGMENT_DATA *f = Fragment;

	for(i=0;i<FRAGMENT_MAX;i++){
		if(f->count){
			f->x += f->vx;
			f->y += f->vy;
			f->count--;
		}
		f++;
	}
}

void fragment_draw(void)
{
	int i;
	FRAGMENT_DATA *f = Fragment;

	for(i=0;i<FRAGMENT_MAX;i++){
		if(f->count)	_FDraw(f);
		f++;
	}
}

void fragment_setup(void)
{
	int i;

	for(i=0;i<FRAGMENT_MAX;i++){
		//memset(Fragment+i,0,sizeof(FRAGMENT_DATA));
		Fragment[i].count = 0;
	}

	FragmentPtr = 0;
}

static void pascal _FDraw(FRAGMENT_DATA *f)
{
	int		x,y;
	RECT	src;

	switch(f->cmd){
		case(FRG_EVADE):
			x = (f->x>>6) - 4;
			y = (f->y>>6) - 4;
			BltSetRect(&src,592+(((24-f->count)>>2)<<3),8,8,8);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_HIT):
			x = (f->x>>6) - 4;
			y = (f->y>>6) - 4;
			BltSetRect(&src,592+(((24-f->count)>>2)<<3),8+8,8,8);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_SMOKE):
			x = (f->x>>6) - 4;
			y = (f->y>>6) - 4;
			BltSetRect(&src,592+(((24-f->count)>>2)<<3),0,8,8);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_STAR1):
			x = (f->x>>6) - 8;
			y = (f->y>>6) - 8;
			BltSetRect(&src,624,432,16,16);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_STAR2):
			x = (f->x>>6) - 16;
			y = (f->y>>6) - 16;
			BltSetRect(&src,608,448,32,32);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_STAR3):
			x = (f->x>>6) - 16;
			y = (f->y>>6) - 16;
			BltSetRect(&src,608,448,32,32);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_HEART):
			x = (f->x>>6) - 16;
			y = (f->y>>6) - 16;
			BltSetRect(&src,576,448,32,32);
			GrpBlt(&src,x,y,GrTama);
		break;

		case(FRG_FATCIRCLE):
			if(DxObj.Bpp==16){
				GrpLock();
				GrpSetColor(4,0,0);
				GrpSetAlpha(128,ALPHA_ONE);
				GrpFatCircleA(f->x>>6,f->y>>6,(60-f->count)*6,5);
				GrpUnlock();
			}
		break;

		default:
			// ここには来ないはずだが ... //
		break;
	}
}

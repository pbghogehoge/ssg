/*                                                                           */
/*   Item.cpp   アイテムの処理                                               */
/*                                                                           */
/*                                                                           */

#include "Item.h"



ITEM_DATA	Item[ITEM_MAX];
WORD		ItemInd[ITEM_MAX];
WORD		ItemNow;


static void Indsort(WORD *ind,WORD *now);



// アイテムを発生させる //
FVOID ItemSet(int x,int y,BYTE type)
{
	ITEM_DATA	*ip;
	BYTE		deg;

	if(ItemNow+1>=ITEM_MAX) return;

	ip = Item + ItemInd[ItemNow++];

	deg = -64;//rnd()%(128-110)+128+55;

	ip->x     = x;
	ip->y     = y;
	ip->type  = type;	//ITEM_SCORE;
	ip->count = 0;

	switch(type){
		case(ITEM_SCORE):
			ip->vx    = cosl(deg,64*3);
			ip->vy    = sinl(deg,64*3);
		break;

		case(ITEM_EXTEND):
			ip->vx    = cosl(deg,64*2);
			ip->vy    = sinl(deg,64*2);
		break;

		case(ITEM_BOMB):
			ip->vx    = cosl(deg,64*2);
			ip->vy    = sinl(deg,64*2);
		break;
	}
}

// アイテムを動かす //
FVOID ItemMove(void)
{
	ITEM_DATA		*ip;
	int				i,tx,ty,l;
	DWORD			point;

	//point = 100+(Viv.evade)*100;
	point = (((SY_MAX-Viv.y)>>6)+Viv.evade*4)*160;

	for(i=0;i<ItemNow;i++){
		ip = Item + ItemInd[i];
		if(!Viv.bomb_time){
			ip->x += ip->vx;
			ip->y += ip->vy;
		}
		else{
			tx = (Viv.x - ip->x);
			ty = (Viv.y - ip->y);
			l  = 1 + (isqrt(tx*tx + ty*ty) / 700);		// 512(3+6)
			ip->x += tx / l;
			ip->y += ty / l;
		}

		if(ip->vy<64*6) ip->vy += ITEM_GRAVITY;
		ip->count++;
		if(HITCHK(ip->x,Viv.x,ITEM_HITX) && HITCHK(ip->y,Viv.y,ITEM_HITY)){
			switch(ip->type){
				case(ITEM_SCORE):
					SndPlayEX(SOUND_ID_SELECT,ip->x,0);
					PlayRankAdd((SY_MAX-Viv.y)>>10);
					score_add(point);
					StringEffect2(ip->x,ip->y,point);
					if(Viv.evade){
						fragment_set(ip->x, ip->y, FRG_STAR3);
						fragment_set(ip->x, ip->y, FRG_STAR3);
					}
				break;

				case(ITEM_EXTEND):
					SndPlayEX(SOUND_ID_SELECT,ip->x,0);
					StringEffect(180+64,80,"E x t e n d  !");
					Viv.left++;
				break;

				case(ITEM_BOMB):
					SndPlayEX(SOUND_ID_SELECT,ip->x,0);
					StringEffect(120+64,80,"B o m b   E x t e n d  !");
					Viv.bomb++;
				break;
			}
			ip->type = ITEM_DELETE;
		}

		// 上方向では消去しない //
		if((ip->x)<GX_MIN-8*64||(ip->x)>GX_MAX+8*64||(ip->y)>GY_MAX+8*64)
			ip->type = ITEM_DELETE;
	}

	Indsort(ItemInd,&ItemNow);
}

// アイテムを描画する //
FVOID ItemDraw(void)
{
	ITEM_DATA		*ip;
	int				i,j,x,y;
	BYTE			ptn;
	RECT			src;
	static BYTE		time;

	for(i=0;i<ItemNow;i++){
		ip = Item + ItemInd[i];
		switch(ip->type){
			case(ITEM_SCORE):
				ptn = (BYTE)((ip->count>>2)&3);
				BltSetRect(&src,384+(ptn<<4),256+16,16,16);
				x = (ip->x>>6) - 8;
				y = (ip->y>>6) - 8;
				GrpBlt(&src,x,y,GrTama);
			break;

			case(ITEM_EXTEND):
				for(j=0; j<8; j++){
					ptn = (BYTE)((ip->count>>2)&3);
					BltSetRect(&src,384+16*4+(ptn<<4),256+16,16,16);
					x = (ip->x>>6) - 8 + cosl(ip->count+j*256/8, 12);
					y = (ip->y>>6) - 8 + sinl(ip->count+j*256/8, 12);
					GrpBlt(&src,x,y,GrTama);
				}

			//	ptn = (BYTE)((ip->count>>2)&3);
			//	BltSetRect(&src,384+16*4+(ptn<<4),256+16,16,16);
			//	x = (ip->x>>6) - 8;
			//	y = (ip->y>>6) - 8;
			//	GrpBlt(&src,x,y,GrTama);
			break;

			case(ITEM_BOMB):
				for(j=0; j<8; j++){
					ptn = (BYTE)((ip->count>>2)&3);
					BltSetRect(&src,384+16*8+(ptn<<4),256+16,16,16);
					x = (ip->x>>6) - 8 + cosl(-2*ip->count+j*256/8, 12);
					y = (ip->y>>6) - 8 + sinl(-2*ip->count+j*256/8, 12);
					GrpBlt(&src,x,y,GrTama);
				}

			//	ptn = (BYTE)((ip->count>>2)&3);
			//	BltSetRect(&src,384+16*8+(ptn<<4),256+16,16,16);
			//	x = (ip->x>>6) - 8;
			//	y = (ip->y>>6) - 8;
			//	GrpBlt(&src,x,y,GrTama);
			break;

			default:
			break;
		}
	}
}

// アイテム配列の初期化 //
FVOID ItemIndSet(void)
{
	int i;

	for(i=0;i<ITEM_MAX;i++){
		ItemInd[i] = i;
		//memset(Item+i,0,sizeof(ITEM_DATA));
	}

	ItemNow = 0;
}

static void Indsort(WORD *ind,WORD *now)
{
	WORD i,temp,next;

	for(i=temp=next=0;i<(*now);i++){

		if((Item+(*(ind+i)))->type == ITEM_DELETE){

			// フラグの立っていないアイテムを検索する //
			for(temp=i+1;temp<(*now);temp++){
				if((Item+(*(ind+temp)))->type != ITEM_DELETE){
					next++;
					goto SEARCH_END;
				}
			}

			SEARCH_END:
			swap16(ind+i,ind+temp);
		}
		else
			next++;
	}

	*now=next;
}

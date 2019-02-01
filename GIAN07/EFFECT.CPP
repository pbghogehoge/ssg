/*                                                                           */
/*   EFFECT.cpp   エフェクト管理                                             */
/*                                                                           */
/*                                                                           */

#include "EFFECT.h"


SEFFECT_DATA		SEffect[SEFFECT_MAX];
CIRCLE_EFC_DATA		CEffect[CIRCLE_EFC_MAX];
LOCKON_INFO			LockInfo[LOCKON_MAX];
SCREENEFC_INFO		ScreenInfo;

static char			MTitleBuf[300] = "";


extern FVOID CircleFadeOut(int x,int y,int r);



// 円エフェクトの初期化 //
FVOID CEffectInit(void)
{
	int		i;

	for(i=0;i<CIRCLE_EFC_MAX;i++)
		CEffect[i].type = CEFC_NONE;
}

// 円エフェクトを動かす //
FVOID CEffectMove(void)
{
	int					i;
	CIRCLE_EFC_DATA		*ce;

	for(i=0,ce=CEffect;i<CIRCLE_EFC_MAX;i++,ce++){
		ce->count++;

		switch(ce->type){
			case(CEFC_STAR):
				ce->r-=3;
				ce->d+=2;
				if(ce->r <= 0){
					ce->type = CEFC_NONE;
				}
			break;

			case(CEFC_CIRCLE1):		// 集
				ce->r-=(10+5);
				if(ce->r <= 0){
					ce->type = CEFC_NONE;
				}
			break;

			case(CEFC_CIRCLE2):		// 離
				ce->r+=(8+5);
				if(ce->r >= ce->rmax){
					ce->type = CEFC_NONE;
				}
			break;
		}
	}
}

// 円エフェクトを描画する //
FVOID CEffectDraw(void)
{
	int					i, j, k, r;
	int					x1, x2, y1, y2;
	CIRCLE_EFC_DATA		*ce;
	static BYTE dtable[4] = {0, 1, 3, 7};

	GrpLock();

	for(i=0,ce=CEffect;i<CIRCLE_EFC_MAX;i++,ce++){
		switch(ce->type){
			case(CEFC_STAR):
				for(k=0; k<4; k++){
					r = ce->r - k*7;
					if(r < 0) continue;
					GrpSetColor(5,k+2,k+2);
					for(j=0; j<5; j++){
						x1 = ce->x + cosl(ce->d + dtable[k]*ce->count/10 + j*256/5, r);
						y1 = ce->y + sinl(ce->d + dtable[k]*ce->count/10 + j*256/5, r);
						x2 = ce->x + cosl(ce->d + dtable[k]*ce->count/10 + (j+2)*256/5, r);
						y2 = ce->y + sinl(ce->d + dtable[k]*ce->count/10 + (j+2)*256/5, r);
						GrpLine(x1, y1, x2, y2);
					}
				}
			break;

			case(CEFC_CIRCLE1):		// 集
				for(k=0; k<4; k++){
					r = ce->r - max(2, (k * ce->r)/8);
					if(r < 0) continue;
					GrpSetColor(5,k+2,k+2);
					GrpCircle(ce->x, ce->y, r);
				}
			break;

			case(CEFC_CIRCLE2):		// 離
				for(k=0; k<4; k++){
					r = ce->r - max(2, (k * ce->r)/12);
					//r = ce->r - max(2, (k * (600-ce->r))/16);
					if(r < 0) continue;
					GrpSetColor(5,k+2,k+2);
					GrpCircle(ce->x, ce->y, r);
				}
			break;
		}
	}

	GrpUnlock();
}

// 円エフェクトをセットする //
FVOID CEffectSet(int x,int y,BYTE type)
{
	int					i;
	CIRCLE_EFC_DATA		*ce = NULL;

	for(i=0;i<CIRCLE_EFC_MAX;i++){
		if(CEffect[i].type == CEFC_NONE){
			ce = CEffect + i;
			break;
		}
	}

	if(ce == NULL) return;

	ce->x     = x >> 6;
	ce->y     = y >> 6;
	ce->type  = type;
	ce->count = ce->d = 0;

	switch(type){
		case(CEFC_STAR):
			SndPlay(SOUND_ID_TAMEFAST);
			ce->rmax = ce->r = 400;
		break;

		case(CEFC_CIRCLE1):		// 集
			ce->rmax = ce->r = 600+50;
		break;

		case(CEFC_CIRCLE2):		// 離
			ce->rmax = 900-100;
			ce->r    = 0;
		break;

		default:
		break;
	}
}

// エフェクトの初期化を行う //
FVOID SEffectInit(void)
{
	int		i;

	for(i=0;i<SEFFECT_MAX;i++){
		//memset(SEffect+i,0,sizeof(SEFFECT_DATA));
		SEffect[i].cmd = SEFC_NONE;
	}
}

// 文字列系エフェクト //
FVOID StringEffect(int x,int y,char *s)
{
	int		i,j,len;

	len = strlen(s);
	for(i=j=0;i<len;i++){
		while(SEffect[j].cmd!=SEFC_NONE){
			j++;
			if(j>=SEFFECT_MAX) return;
		}
		SEffect[j].c    = s[i];
		SEffect[j].x    = (x + (i<<4) + 512)<<6;
		SEffect[j].y    = (y)<<6;
		SEffect[j].vx   = (-20)<<6;
		SEffect[j].vy   = (0)<<6;
		SEffect[j].cmd  = SEFC_STR1;
		SEffect[j].time = 26;
	}
}

// 得点表示エフェクト //
FVOID StringEffect2(int x,int y,DWORD point)
{
	int		i;

	for(i=0;i<SEFFECT_MAX;i++){
		if(SEffect[i].cmd!=SEFC_NONE) continue;

		SEffect[i].point = point;
		SEffect[i].x     = x;
		SEffect[i].y     = y;
		SEffect[i].vx    = 0;
		SEffect[i].vy    = -64*3+32;
		SEffect[i].cmd   = SEFC_STR2;
		SEffect[i].time  = 90;
		return;
	}
}

// ゲームオーバーの表示 //
FVOID StringEffect3(void)
{
	int		i;

	for(i=0;i<SEFFECT_MAX;i++){
		if(SEffect[i].cmd!=SEFC_NONE) continue;

		SEffect[i].x     = GX_MID;
		SEffect[i].y     = GY_MID-64*(60+40);
		SEffect[i].vx    = 0;
		SEffect[i].vy    = 0;
		SEffect[i].cmd   = SEFC_GAMEOVER;
		SEffect[i].time  = 120-35;//100;
		return;
	}
}

// 曲名の表示 //
FVOID SetMusicTitle(int y,char *s)
{
	int			n,len;
	int			x;

	if(s==NULL) return;

	// 空きバッファ検索 //
	for(n=0;n<SEFFECT_MAX;n++){
		if(SEffect[n].cmd==SEFC_NONE) break;
	}
	if(n==SEFFECT_MAX) return;

	// 曲のタイトルが長すぎる場合、どうしましょう？ //
	strcpy(MTitleBuf,s);
	len = strlen(s);
	x   = (640-128-16-9*min(strlen(s),23+1));

	SEffect[n].cmd  = SEFC_MTITLE1;
	SEffect[n].x    = (x<<6);// + (64*2)*16;
	SEffect[n].y    = y<<6;
	SEffect[n].time = 64*2;
	SEffect[n].vx   = len*16;
	SEffect[n].vy   = 16;

	GrpOffsClear(GrMTitle);GrpSetColorKey(GrMTitle);
	DrawGrdFont(GrMTitle,MTitleBuf);
}

// エフェクトを動かす(仕様変更の可能性があります) //
FVOID SEffectMove(void)
{
	int				i;
	SEFFECT_DATA	*e;
	BYTE			deg;

	for(i=0;i<SEFFECT_MAX;i++){
		e = SEffect+i;
		switch(e->cmd){
			case(SEFC_STR1):
				e->x += e->vx;
				e->y += e->vy;
				if(e->time==0){
					e->cmd  = SEFC_STR1_2;
					e->time = 256;//128;
				}
			break;

			case(SEFC_STR1_3):
				e->x += e->vx;
				e->y += (e->vy+=16);
				if(e->time==0) e->cmd = SEFC_NONE;
			break;

			case(SEFC_STR1_2):
				if(e->time==0){
					deg = 128 + rnd()%128;
					e->cmd  = SEFC_STR1_3;
					e->time = 64;
					e->vx   = cosl(deg,10*64);
					e->vy   = sinl(deg,10*64);
				}
			break;

			case(SEFC_STR2):
				if(e->time==0) e->cmd = SEFC_NONE;
				e->x += e->vx;
				e->y += (e->vy+=3);
			break;

			case(SEFC_GAMEOVER):
				if(e->time==0){
					e->cmd  = SEFC_GAMEOVER2;
					e->time = 35;
				}
			break;

			case(SEFC_GAMEOVER2):
				if(e->time==0){
					e->cmd  = SEFC_GAMEOVER3;
					e->time = 180;
				}
			break;

			case(SEFC_GAMEOVER3):
				if(e->time==0) e->cmd = SEFC_NONE;
			break;

			case(SEFC_MTITLE1):		// 曲名出現
				//e->x -= 16;
				if(e->time==0){
					e->cmd  = SEFC_MTITLE2;
					e->time = 64*4;
				}
			break;

			case(SEFC_MTITLE2):		// 曲名停止
				if(e->time==0){
					e->cmd  = SEFC_MTITLE3;
					e->time = 64*2;
				}
			break;

			case(SEFC_MTITLE3):		// 曲名消去
				e->x += 64;
				if(e->time==0){
					e->cmd = SEFC_NONE;
				}
			break;

			case(SEFC_NONE):default:
			break;
		}
		e->time--;
	}
}

// エフェクトを描画する(仕様変更の可能性があります) //
FVOID SEffectDraw(void)
{
	int				i,j,k;
	int				x,y;
	int				temp;
	BYTE			degx = 0;
	SEFFECT_DATA	*e;
	RECT			src;
	char			buf[20];

	for(i=0;i<SEFFECT_MAX;i++){
		e = SEffect+i;
		switch(e->cmd){
			case(SEFC_STR1):case(SEFC_STR1_2):case(SEFC_STR1_3):
				GrpPutc(e->x>>6,e->y>>6,e->c);
			break;

			case(SEFC_STR2):
				sprintf(buf,"%d",e->point);
				GrpPutScore(e->x>>6,e->y>>6,buf);
			break;

			case(SEFC_GAMEOVER):
				strcpy(buf,"GAME OVER");
				for(j=0;j<9;j++){
					// *37 //
					x = (e->x>>6) + cosl((BYTE)e->time*3+j*26,e->time*4);
					y = (e->y>>6) + sinl((BYTE)e->time*3+j*26,e->time*4);
					GrpPutc(x,y,buf[j]);
				}
			break;

			case(SEFC_GAMEOVER2):
				x = (e->x >> 6) + 8;
				y = (e->y >> 6) + 8;
				j = (35 - e->time) / 2;
				GrpLock();
				GrpSetColor(0,0,0);
				GrpSetAlpha((BYTE)(35-e->time)*3,ALPHA_NORM);
				GrpBoxA(x-170,y-j,x+170,y+j);
				GrpUnlock();

				strcpy(buf,"GAME OVER");
				for(j=0;j<9;j++){
					// *37 //
					x = (e->x>>6) + (j-4)*(35-e->time);
					y = (e->y>>6);
					GrpPutc(x,y,buf[j]);
				}
			break;

			case(SEFC_GAMEOVER3):
				strcpy(buf,"GAME OVER");
				//if(e->time%60<30) break;
				for(j=0;j<9;j++){
					// *37 //
					x = (e->x>>6) + (j-4)*(35);
					y = (e->y>>6);
					GrpPutc(x,y,buf[j]);
				}
			break;

			case(SEFC_MTITLE3):
				degx = (BYTE)(64*2 - e->time);
				for(j=0;j<e->vx;j++){
					SetRect(&src,j,0,j+1,e->vy);
					temp = sinl(degx,100);
					for(k=0;k<2;k++){
						GrpBlt(&src,(e->x>>6)+sinl(degx+j/2,temp)+j+k,(e->y>>6)-sinl(degx+j,degx/*40*/),GrMTitle);
					}
				}
			break;

			case(SEFC_MTITLE1):
				degx = (BYTE)e->time;
				for(j=0;j<e->vx;j++){
					SetRect(&src,j,0,j+1,e->vy);
					temp = sinl(degx,160);
					for(k=0;k<2;k++){
						GrpBlt(&src,(e->x>>6)+sinl(degx+j/2,temp)+j+k,(e->y>>6)-sinl(degx+j,degx/*40*/),GrMTitle);
					}
				}
			break;

			case(SEFC_MTITLE2):
				GrpLock();
				GrpSetColor(0,0,0);
				GrpSetAlpha(sinl((BYTE)e->time-32,80)+80,ALPHA_NORM);
				for(j=0;j<16;j++){
					temp = sinl(128+j*16,16);
					GrpBoxA((e->x>>6)+temp-16,(e->y>>6)+j,(X_MAX-16)-temp,(e->y>>6)+j+1);
				}
				GrpUnlock();
				SetRect(&src,0,0,e->vx,e->vy);
				GrpBlt(&src,e->x>>6,e->y>>6,GrMTitle);
			break;

			case(SEFC_NONE):default:
			break;
		}
	}
}

// エフェクト用 Restore 処理 //
FBOOL SEffectRestore(void)
{
	if(GrMTitle==NULL) return FALSE;

	GrpOffsClear(GrMTitle);GrpSetColorKey(GrMTitle);
	DrawGrdFont(GrMTitle,MTitleBuf);

	return TRUE;
}

// 画面全体に対するエフェクトの初期化 //
FVOID ScreenEffectInit(void)
{
	ScreenInfo.cmd   = SCNEFC_NONE;
	ScreenInfo.count = 0;

	GrpSetClip(X_MIN,Y_MIN,X_MAX,Y_MAX);
}

// 画面全体に対するエフェクトをセットする //
FVOID ScreenEffectSet(BYTE cmd)
{
	//if(ScreenInfo.cmd != SCNEFC_NONE) return;

	ScreenInfo.cmd   = cmd;
	ScreenInfo.count = 0;

	switch(cmd){
		case(SCNEFC_CFADEIN):	// 円形フェードイン
		case(SCNEFC_CFADEOUT):	// 円形フェードアウト
		break;

		case(SCNEFC_WHITEIN):	// ホワイトイン
		case(SCNEFC_WHITEOUT):	// ホワイトアウト
		break;

		default:	// ばぐばぐ
			ScreenInfo.cmd = SCNEFC_NONE;	// 一応エフェクトを切っておく
		break;
	}
}

// 画面全体に対するエフェクトを動かす //
FVOID ScreenEffectMove(void)
{
	switch(ScreenInfo.cmd){
		case(SCNEFC_CFADEIN):	// 円形フェードイン
			ScreenInfo.count+=10;
			if(ScreenInfo.count>600) ScreenInfo.cmd = SCNEFC_NONE;
		break;

		case(SCNEFC_CFADEOUT):	// 円形フェードアウト
			ScreenInfo.count+=10;
			if(ScreenInfo.count>600){
				ScreenInfo.count = 600;
				//ScreenInfo.cmd = SCNEFC_NONE;
			}
		break;

		case(SCNEFC_WHITEIN):	// ホワイトイン
			ScreenInfo.count+=10;
			if(ScreenInfo.count>=160){
				//ScreenInfo.cmd   = SCNEFC_WHITEOUT;
				//ScreenInfo.count = 0;
				ScreenInfo.count = 150;
			}
		break;

		case(SCNEFC_WHITEOUT):	// ホワイトアウト
			ScreenInfo.count+=10;
			if(ScreenInfo.count>=160) ScreenInfo.cmd = SCNEFC_NONE;
		break;

		default:	// ばぐばぐ
		break;
	}
}

// 画面全体に対するエフェクトを描画する //
FVOID ScreenEffectDraw(void)
{
	int		i, j;
	RECT	src;

	switch(ScreenInfo.cmd){
		case(SCNEFC_CFADEIN):	// 円形フェードイン
			CircleFadeOut(X_MID,Y_MID,ScreenInfo.count);
		break;

		case(SCNEFC_CFADEOUT):	// 円形フェードアウト
			CircleFadeOut(X_MID,Y_MID,400-ScreenInfo.count);
		break;

		case(SCNEFC_WHITEIN):	// ホワイトイン
			BltSetRect(&src,(15-(ScreenInfo.count/10))*16,128+16,16,16);
			for(i=128; i<640-128; i+=16){
				for(j=0; j<480; j+=16){
					GrpBlt(&src, i, j, GrTama);
				}
			}
		break;

		case(SCNEFC_WHITEOUT):	// ホワイトアウト
			BltSetRect(&src,(ScreenInfo.count/10)*16,128+16,16,16);
			for(i=128; i<640-128; i+=16){
				for(j=0; j<480; j+=16){
					GrpBlt(&src, i, j, GrTama);
				}
			}
		break;

		default:	// ばぐばぐ
		break;
	}
}

// 円形フェードサポート関数 //
extern FVOID CircleFadeOut(int x,int y,int r)
{
	RECT	src;
	int		i,j,temp;

	if(r<0) r=0;


	for(i=0;i<640;i+=16){
		for(j=0;j<480;j+=16){
			temp = isqrt((i-x)*(i-x)+(j-y)*(j-y));
			if(temp<r && r-temp<8*16 && temp>=0){
				temp = (r-temp)>>3;
				//temp = (r-temp)>>4;
				//BltSetRect(&src,((temp)<<4)+256,104,16,16);
				BltSetRect(&src,((temp)<<4),128,16,16);
				GrpBlt(&src,i,j,GrTama);
			}
			else if(temp>=r){
				//BltSetRect(&src,256,104,16,16);
				BltSetRect(&src,0,128,16,16);
				GrpBlt(&src,i,j,GrTama);
			}
		}
	}

	if(r != 0){
		SetRect(&src,x-r,y-r,x+r,y+r);
		if(src.left  < X_MIN) src.left  = X_MIN;
		if(src.right < X_MIN) src.right = X_MIN;
		if(src.left  > X_MAX) src.left  = X_MAX;
		if(src.right > X_MAX) src.right = X_MAX;
		if(src.top    < Y_MIN) src.top    = Y_MIN;
		if(src.bottom > Y_MAX) src.bottom = Y_MAX;
		GrpSetClip(src.left,src.top,src.right,src.bottom);
	}
	else{
		GrpSetClip(X_MID,Y_MID,X_MID,Y_MID);
	}
}

// ロックオン配列を初期化 //
FVOID ObjectLockOnInit(void)
{
	int		i;

	for(i=0;i<LOCKON_MAX;i++){
		//memset(LockInfo+i,0,sizeof(LOCKON_INFO));
		LockInfo[i].state = LOCKON_NONE;
	}
}

// 何かをロックオンする //
FVOID ObjectLockOn(int *x,int *y,int wx64,int hx64)
{
	LOCKON_INFO		*l;
	int				i;

	for(i=0;i<LOCKON_MAX;i++){
		if(LockInfo[i].state==LOCKON_NONE) break;
	}
	if(i==LOCKON_MAX) return;

	l = LockInfo + i;

	l->x = x;
	l->y = y;

	l->width  = (wx64<<2);
	l->height = (hx64<<2);

	l->vx = -(wx64*4/30);
	l->vy = -(hx64*4/30);
	l->count = 30;

	l->state = LOCKON_01;
	//SndPlay(SOUND_ID_SELECT);
}

// ロックオンアニメーション動作 //
FVOID ObjectLockMove(void)
{
	LOCKON_INFO		*l;
	int				i;

	for(i=0;i<LOCKON_MAX;i++){
		l = LockInfo + i;
		switch(l->state){
			case(LOCKON_01):
				l->count--;
				l->width  += l->vx;
				l->height += l->vy;
				if(l->count==0){
					l->state = LOCKON_NONE;//LOCKON_02;
					l->count = 30;//120;
				}
			break;
			case(LOCKON_02):
				l->count--;
				if(l->count==0){
					l->state = LOCKON_NONE;//LOCKON_03;
					l->count = 30;
				}
			break;
			case(LOCKON_03):
				l->count--;
				l->width  -= l->vx;
				l->height -= l->vy;
				if(l->count==0){
					l->state = LOCKON_NONE;
				}
			break;
		}
	}
}

// ロックオン枠描画 //
FVOID ObjectLockDraw(void)
{
	LOCKON_INFO		*l;
	int				i;
	RECT			temp;

	for(i=0;i<LOCKON_MAX;i++){
		l = LockInfo + i;
		if(l->state!=LOCKON_NONE){
			SetRect(&temp,208,80,272,96);
			GrpBlt(&temp,((*(l->x))>>6)-8,((*(l->y))>>6)-8,GrTama);

			SetRect(&temp,280,88,288,96);
			GrpBlt(&temp,((*(l->x)-l->width)>>6)-4,((*(l->y)-l->height)>>6)-4,GrTama);
			SetRect(&temp,272,80,280,88);
			GrpBlt(&temp,((*(l->x)+l->width)>>6)-4,((*(l->y)+l->height)>>6)-4,GrTama);

			SetRect(&temp,280,80,288,88);
			GrpBlt(&temp,((*(l->x)-l->width)>>6)-4,((*(l->y)+l->height)>>6)-4,GrTama);
			SetRect(&temp,272,88,280,96);
			GrpBlt(&temp,((*(l->x)+l->width)>>6)-4,((*(l->y)-l->height)>>6)-4,GrTama);
		}
	}
}



#include "DD_GRP3D.h"
// スペアナ描画 //
FVOID GrpDrawSpect(int x,int y)
{
	int		i,j,temp,temp2;
	WORD	ftable[128+8+8],ftable2[128];

	static WORD ftable3[128+8+8];
	static BYTE ftable3flag;

	RECT	src = {16*16,0,16*16+8*21,8};		// ,,,8*4

	BYTE ColTable[10]={
		RGB256(2,0,0),RGB256(3,0,0),RGB256(4,0,0),RGB256(5,0,0),RGB256(5,1,0),
		RGB256(5,2,1),RGB256(5,3,1),RGB256(5,4,2),RGB256(5,5,3),RGB256(5,5,4)
	};

	ftable3flag = (ftable3flag+1)%5;

	for(i=0;i<128;i++){
		temp = temp2 = 0;
		for(j=0;j<16;j++){
			temp += Mid_PlayTable[j][i];
			temp2 += (Mid_PlayTable[j][i] ? 1 : 0);
			if(Mid_PlayTable[j][i]!=0) Mid_PlayTable[j][i]-=((Mid_PlayTable[j][i]>>3)+1);//4
			//if(Mid_PlayTable[j][i]) Mid_PlayTable[j][i]-=(Mid_PlayTable[j][i]>>3)+1;
		}
		if(temp2==0) temp2 = 1;
		ftable2[i] = temp/temp2;
	}

	for(i=0;i<128+8+8;i++){
		ftable[i]  = ((i>=8 && i<=128+7) ? ftable2[i-8] : 0)   *(256-sinm(0))/256;

		ftable[i] += ((i>=9  && i<=128+8 ) ? ftable2[i-9 ] : 0)*(256-sinm( 8))/256;
		ftable[i] += ((i>=10 && i<=128+9 ) ? ftable2[i-10] : 0)*(256-sinm(16))/256;
		ftable[i] += ((i>=11 && i<=128+10) ? ftable2[i-11] : 0)*(256-sinm(24))/256;
		ftable[i] += ((i>=12 && i<=128+11) ? ftable2[i-12] : 0)*(256-sinm(32))/256;
		ftable[i] += ((i>=13 && i<=128+12) ? ftable2[i-13] : 0)*(256-sinm(40))/256;
		ftable[i] += ((i>=14 && i<=128+13) ? ftable2[i-14] : 0)*(256-sinm(48))/256;
		ftable[i] += ((i>=15 && i<=128+14) ? ftable2[i-15] : 0)*(256-sinm(56))/256;
		ftable[i] += ((i>=16 && i<=128+15) ? ftable2[i-16] : 0)*(256-sinm(63))/256;

		ftable[i] += ((i<=128+6 && i>=7) ? ftable2[i-7] : 0) *(256-sinm( 8))/256;
		ftable[i] += ((i<=128+5 && i>=6) ? ftable2[i-6] : 0) *(256-sinm(16))/256;
		ftable[i] += ((i<=128+4 && i>=5) ? ftable2[i-5] : 0) *(256-sinm(24))/256;
		ftable[i] += ((i<=128+3 && i>=4) ? ftable2[i-4] : 0) *(256-sinm(32))/256;
		ftable[i] += ((i<=128+2 && i>=3) ? ftable2[i-3] : 0) *(256-sinm(40))/256;
		ftable[i] += ((i<=128+1 && i>=2) ? ftable2[i-2] : 0) *(256-sinm(48))/256;
		ftable[i] += ((i<=128+0 && i>=1) ? ftable2[i-1] : 0) *(256-sinm(56))/256;
		ftable[i] += ((i<=128-1 && i>=0) ? ftable2[i-0] : 0) *(256-sinm(63))/256;

		ftable[i]>>=3;

		if(ftable3[i]<ftable[i])
			ftable3[i] = ftable[i];
		else if((!ftable3flag) && ftable3[i])
			ftable3[i]--;
	}

	//GrpBlt(&src,SPECT_X-7,SPECT_Y,GrTama);

	GrpLock();

	if(DxObj.Bpp==16){
		for(i=0;i<128+8+8;i++){
			//WORD c2 = 0;	//5
			GrpGrdLineEx(i+x,y-ftable[i]*2,y);
		}
	}
	else{
		GrpSetColor(4,2,1);
		for(i=0;i<128+8+8;i++){
			//WORD c2 = 0;	//5
			if(ftable[i])	GrpLine(i+x,y-ftable[i]*2,i+x,y);
		}
	}

	GrpUnlock();
}

// 押されているところを表示 //
FVOID GrpDrawNote(void)
{
	// 0123456789ab (Mod c)
	// o#o#oo#o#o#o
	// o o oo o o o

	RECT src[12] = {
		{ 0,464, 3,474},		// しろ
		{ 0,456, 3,461},		// 黒

		{ 4,464, 7,474},		// しろ
		{ 0,456, 3,461},		// 黒

		{ 8,464,11,474},		// しろ
		{12,464,15,474},		// しろ

		{ 0,456, 3,461},		// 黒
		{16,464,19,474},		// しろ

		{ 0,456, 3,461},		// 黒
		{20,464,23,474},		// しろ

		{ 0,456, 3,461},		// 黒
		{24,464,27,474},		// しろ
	};

	RECT src2[12] = {
		{ 0,464-24, 3,474-24},		// しろ
		{ 0,456-24, 3,461-24},		// 黒

		{ 4,464-24, 7,474-24},		// しろ
		{ 0,456-24, 3,461-24},		// 黒

		{ 8,464-24,11,474-24},		// しろ
		{12,464-24,15,474-24},		// しろ

		{ 0,456-24, 3,461-24},		// 黒
		{16,464-24,19,474-24},		// しろ

		{ 0,456-24, 3,461-24},		// 黒
		{20,464-24,23,474-24},		// しろ

		{ 0,456-24, 3,461-24},		// 黒
		{24,464-24,27,474-24},		// しろ
	};

	int destX[12] = {
		0,		// しろ
		2,		// 黒

		4,		// しろ
		6,		// 黒

		8,		// しろ
		12,		// しろ

		14,		// 黒
		16,		// しろ

		18,		// 黒
		20,		// しろ

		22,		// 黒
		24,		// しろ
	};

	int		Track,NoteNo,x,y,LevelSum,num;
	RECT	rc;

	for(Track=0;Track<16;Track++){
		GrpPutMidNum( 50,22+Track*24,Mid_VolumeTable[Track]);
		GrpPutMidNum(125,22+Track*24,Mid_ExpressionTable[Track]);
		GrpPutMidNum(181,22+Track*24,(int)Mid_PanpodTable[Track]-64);

		LevelSum = num = 0;
		for(NoteNo=0;NoteNo<128;NoteNo++){
			if(Mid_NoteWTable[Track][NoteNo]){
				x = 40 + destX[NoteNo%12] + (NoteNo/12)*28;
				y =  9 + Track*24;
				GrpBlt(src2+NoteNo%12,x,y,GrEnemy);
				Mid_NoteWTable[Track][NoteNo]--;
			}

			if(Mid_NoteTable[Track][NoteNo]){
				x = 40 + destX[NoteNo%12] + (NoteNo/12)*28;
				y =  9 + Track*24;
				GrpBlt(src+NoteNo%12,x,y,GrEnemy);
			}
			if(Mid_PlayTable2[Track][NoteNo]){
				LevelSum += (Mid_PlayTable2[Track][NoteNo]);
				//if(Mid_PlayTable2[Track][NoteNo]>128) Mid_PlayTable2[Track][NoteNo]=128;
				Mid_PlayTable2[Track][NoteNo] -= max(Mid_PlayTable2[Track][NoteNo]/50,1);
				num++;
			}
		}

		if(num){
			BltSetRect(&rc,80,456,min(LevelSum/num,96),5);
			GrpBlt(&rc,240,22+Track*24,GrEnemy);
		}
	}
}


BOOL bEnableWarnEfc = FALSE;
WORD WarnEfcTime = 0;

// ワーニングの初期化 //
FVOID WarningEffectInit(void)
{
	bEnableWarnEfc = FALSE;
}

// ワーニングの発動！！ //
FVOID WarningEffectSet(void)
{
	bEnableWarnEfc = TRUE;
	WarnEfcTime    = 0;
	InitWarning();
}

// ワーニングの動作 //
FVOID WarningEffectMove(void)
{
	if(!bEnableWarnEfc) return;

	if(WarnEfcTime<64+128){//64+256){
		MoveWarning((BYTE)WarnEfcTime);
	}
	else{
		MoveWarningR(-1);
	}

	if((WarnEfcTime++) == 256+10) bEnableWarnEfc = FALSE;
}

// ワーニングの描画 //
FVOID WarningEffectDraw(void)
{
	int		r;

	if(!bEnableWarnEfc) return;

	if(WarnEfcTime<256-20)
		DrawWarning();

	if(WarnEfcTime>256-40){
		GrpLock();

		r = (WarnEfcTime-(256-40))*3;
		GrpSetColor(1,1,5);
		GrpCircle(320,100,r-=4);
		GrpSetColor(2,2,5);
		GrpCircle(320,100,r-=4);
		GrpSetColor(3,3,5);
		GrpCircle(320,100,r-=6);
		GrpSetColor(4,4,5);
		GrpCircle(320,100,r-=6);
		GrpSetColor(5,5,5);
		GrpCircle(320,100,r-=8);

		GrpUnlock();
	}
}

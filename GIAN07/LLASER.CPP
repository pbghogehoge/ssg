/*                                                                           */
/*   LLaser.cpp   長いレーザーの処理                                         */
/*                                                                           */
/*                                                                           */

#include "GIAN.H"
#include "DD_GRP3D.h"


//// レーザー変数２ ////
LLASER_DATA	LLaser[LLASER_MAX];
LLASER_CMD	LLaserCmd;


//// ローカル関数 ////
static void _LLaserPointSet(LLASER_DATA *lp);
static void _LLaserHitCheck(LLASER_DATA *lp);
static void _LLaserXYSet(int id);			// レーザーの座標をセットする


extern BOOL LLaserSet(BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	// この部分で空いているレーザーのサーチを行う             //
	// もし、レーザーが見つからなければ、FALSE をリターンする //
	// つまり、その場合は、参照カウントを増加させない         //
	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if(lp->flag == LLF_DISABLE) break;
	}
	if(i>=LLASER_MAX) return FALSE;

	lp->dx = LLaserCmd.dx;
	lp->dy = LLaserCmd.dy;
	lp->e  = LLaserCmd.e;

	lp->EnemyID = id;

	lp->x = lp->e->x + lp->dx;
	lp->y = lp->e->y + lp->dy;

	lp->v = LLaserCmd.v;

	lp->c = LLaserCmd.c;

	lp->lx = 0;
	lp->ly = 0;
	lp->wx = 0;
	lp->wy = 0;

	lp->w    = 0;
	lp->wmax = LLaserCmd.w;

	lp->d = LLaserCmd.d;

	if(LLaserCmd.type == LLS_LONGZ){
		lp->d += atan8(Viv.x-lp->x, Viv.y-lp->y);
		lp->type = LLS_LONG;
	}
	else
		lp->type = LLaserCmd.type;

	lp->infx = cosl(lp->d,800);
	lp->infy = sinl(lp->d,800);

	lp->count = 0;

	_LLaserPointSet(lp);				// p[4] をセット

	lp->flag = LLF_LINE;

	//SndPlayEX(1,LaserCmd.x,0);

	return TRUE;
}

extern void LLaserOpen(ENEMY_DATA *e,BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if((lp->e==e) && (lp->EnemyID==id || id==ECLCST_LLASERALL) && lp->flag!=LLF_DISABLE){
			lp->flag = LLF_OPEN;
			SndPlayEX(2,lp->x,DSBPLAY_LOOPING);
		}
	}
}

extern void LLaserClose(ENEMY_DATA *e,BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	if(id==ECLCST_LLASERALL){
		LLaserForceClose(e);
		return;
	}

	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if((lp->e==e) && (lp->EnemyID==id)){
			lp->flag = LLF_CLOSE;
			SndStop(2);
		}
	}
}

extern void LLaserLine(ENEMY_DATA *e,BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if((lp->e==e) && (lp->EnemyID==id ||id==ECLCST_LLASERALL)){
			lp->flag = LLF_CLOSEL;
			SndStop(2);
		}
	}
}

static void _LLaserXYSet(int id)
{
	// 注意！！この関数のid は旧式のid の意味を持つことに注意 //

	LLaser[id].x = LLaser[id].e->x + LLaser[id].dx;
	LLaser[id].y = LLaser[id].e->y + LLaser[id].dy;

	_LLaserPointSet(LLaser+id);			// p[4] をセット
}

extern void LLaserDegA(ENEMY_DATA *e,BYTE d,BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if((lp->e==e) && (lp->EnemyID==id ||id==ECLCST_LLASERALL)){
			lp->d = d;

			lp->lx = cosl(lp->d,lp->w>>6);
			lp->ly = sinl(lp->d,lp->w>>6);

			lp->wx = -(lp->ly);			// lx,ly を６４だけ回転
			lp->wy =   lp->lx ;			//

			lp->infx = cosl(lp->d,800);
			lp->infy = sinl(lp->d,800);

			_LLaserPointSet(lp);				// p[4] をセット
		}
	}
}

extern void LLaserDegR(ENEMY_DATA *e,char d,BYTE id)
{
	LLASER_DATA		*lp;
	int				i;

	for(i=0;i<LLASER_MAX;i++){
		lp = LLaser+i;
		if((lp->e==e) && (lp->EnemyID==id ||id==ECLCST_LLASERALL)){
			lp->d += d;

			lp->lx = cosl(lp->d,lp->w>>6);
			lp->ly = sinl(lp->d,lp->w>>6);

			lp->wx = -(lp->ly);			// lx,ly を６４だけ回転
			lp->wy =   lp->lx ;			//

			lp->infx = cosl(lp->d,800);
			lp->infy = sinl(lp->d,800);

			_LLaserPointSet(lp);				// p[4] をセット
		}
	}
}

// 敵に関連づけられたレーザーを強制クローズ(Level2...) //
extern void LLaserForceClose(ENEMY_DATA *e)
{
	int				i;
	LLASER_DATA		*lp;

	for(i=0,lp=LLaser;i<LLASER_MAX;i++,lp++){
		// (参考) すでにクローズ状態であっても LLaserClose() は問題を発生させない //
		if(lp->e==e){
			lp->flag = LLF_CLOSE;
			SndStop(2);
			//LLaserClose(i);
		}
	}
}

extern void LLaserMove(void)
{
	int				i;
	LLASER_DATA		*lp;

	for(i=0,lp=LLaser;i<LLASER_MAX;i++,lp++){

		// 角度セットモードで、敵の角度と現在の角度が異なっていたら再度セット //
		if(lp->type==LLS_SETDEG && lp->e && lp->d!=lp->e->d){
			lp->d = lp->e->d;

			lp->lx = cosl(lp->d,lp->w>>6);
			lp->ly = sinl(lp->d,lp->w>>6);

			lp->wx = -(lp->ly);			// lx,ly を６４だけ回転
			lp->wy =   lp->lx ;			//

			lp->infx = cosl(lp->d,800);
			lp->infy = sinl(lp->d,800);

			_LLaserPointSet(lp);				// p[4] をセット
		}

		switch(lp->flag){
			// 太くなる場合 //
			case(LLF_OPEN):
				_LLaserXYSet(i);
				lp->w += lp->v;

				if((lp->w)>=(lp->wmax)){
					lp->w    = lp->wmax;
					lp->flag = LLF_NORM;
				}

				lp->lx = cosl(lp->d,lp->w>>6);
				lp->ly = sinl(lp->d,lp->w>>6);
				lp->wx = -(lp->ly);
				lp->wy =   lp->lx;

				_LLaserPointSet(lp);	// p[4] をセット
				_LLaserHitCheck(lp);
			break;

			// 細くなる場合 //
			case(LLF_CLOSE):case(LLF_CLOSEL):
				_LLaserXYSet(i);
				lp->w -= lp->v;

				if((lp->w)<=0){
					lp->w = 0;
					if(lp->flag == LLF_CLOSE){
						lp->flag = LLF_DISABLE;
						lp->e    = NULL;
					}
					else                      lp->flag = LLF_LINE;
				}

				lp->lx = cosl(lp->d,lp->w>>6);
				lp->ly = sinl(lp->d,lp->w>>6);
				lp->wx = -(lp->ly);
				lp->wy =   lp->lx;

				_LLaserPointSet(lp);	// p[4] をセット
				//_LLaserHitCheck(lp);
			break;

			// 直線状態 //
			case(LLF_LINE):
				_LLaserXYSet(i);
				// この部分にレーザー溜めエフェクトを仕掛ける //
				//fragment_set(lp->x,lp->y,FRG_LASER);
			break;

			// ノーマル //
			case(LLF_NORM):
				_LLaserXYSet(i);
				_LLaserHitCheck(lp);
			break;

			case(LLF_DISABLE):
			break;
		}
	}
}

extern void LLaserDraw(void)
{
	int				i,x,y;
	BYTE			c;
	LLASER_DATA		*lp;
	POINT			p[4];
	int				wx,wy,len;

	static const BYTE Table16Bit[16][3] = {
		{3, 0, 3}, {0, 2, 0}, {0, 0, 4}, {4, 2, 0},
		{0, 0, 1}
	};

	static const BYTE Table8BitA[16][3] = {
		{2, 0, 2}, {0, 2, 0}, {0, 1, 3}, {4, 2, 0},
		{0, 0, 1}
	};
	static const BYTE Table8BitB[16][3] = {
		{3, 0, 3}, {0, 4, 0}, {0, 1, 5}, {5, 3, 0},
		{2, 2, 4}
	};
	static const BYTE Table8BitC[16][3] = {
		{5, 4, 5}, {5, 5, 5}, {4, 4, 5}, {5, 5, 4},
		{4, 4, 5}
	};


	int n;
	POINT p2[256];

	GrpLock();

	for(i=0,lp=LLaser;i<LLASER_MAX;i++,lp++){
		c = lp->c;
		switch(lp->flag){
			// 太さを持った状態 //
			case(LLF_OPEN):case(LLF_NORM):
			case(LLF_CLOSE):case(LLF_CLOSEL):
				x = ((lp->x)>>6)+lp->lx;
				y = ((lp->y)>>6)+lp->ly;
				wx = lp->wx;
				wy = lp->wy;
				len = isqrt(wx*wx+wy*wy);

				if(len){
					/*
					p[0].x = p[1].x = lp->p[0].x ;//- wx*4/len;
					p[0].y = p[1].y = lp->p[0].y ;//- wy*4/len;
					p[3].x = p[2].x = lp->p[3].x ;//+ wx*4/len;
					p[3].y = p[2].y = lp->p[3].y ;//+ wy*4/len;
					p[1].x += lp->infx;
					p[1].y += lp->infy;
					p[2].x += lp->infx;
					p[2].y += lp->infy;
					*/
					if(DxObj.D3){
						//GrpSetColor(3,0,3);
						GrpSetColor(Table16Bit[c][0], Table16Bit[c][1], Table16Bit[c][2]);
						GrpSetAlpha(0,ALPHA_ONE);
						GrpGrdRectA(lp->p);
						p2[0].x = x;
						p2[0].y = y;
						p2[1].x = lp->p[0].x;
						p2[1].y = lp->p[0].y;
						p2[33].x = lp->p[3].x;
						p2[33].y = lp->p[3].y;
						for(n=2;n<33;n++){
							p2[n].x = p2[0].x + cosl(lp->d+64+128*(n-1)/32,len);
							p2[n].y = p2[0].y + sinl(lp->d+64+128*(n-1)/32,len);
						}
						GrpGrdPolygon(p2,34);
						break;
					}
					//GrpSetColor(2,0,2);
					GrpSetColor(Table8BitA[c][0], Table8BitA[c][1], Table8BitA[c][2]);
					GrpPolygon(lp->p,4);
				}
				else if(DxObj.D3) break;

				GrpCircleF(x,y,len);//(lp->w>>6)+4);

				//GrpSetColor(3,0,3);//lp->c;
				GrpSetColor(Table8BitB[c][0], Table8BitB[c][1], Table8BitB[c][2]);
				if(len){
					p[0].x = p[1].x = lp->p[0].x - wx/8;//+ wx*2/len;
					p[0].y = p[1].y = lp->p[0].y - wy/8;//+ wy*2/len;
					p[3].x = p[2].x = lp->p[3].x + wx/8;//- wx*2/len;
					p[3].y = p[2].y = lp->p[3].y + wy/8;//- wy*2/len;
					p[1].x += lp->infx;
					p[1].y += lp->infy;
					p[2].x += lp->infx;
					p[2].y += lp->infy;
					GrpPolygon(p,4);
				}
				GrpCircleF(x,y,len-len/8);//(lp->w>>6)+2);

				//GrpSetColor(5,4,5);//lp->c;
				GrpSetColor(Table8BitC[c][0], Table8BitC[c][1], Table8BitC[c][2]);
				if(len){
					p[0].x = p[1].x = lp->p[0].x - wx/4;//+ wx*2/len;
					p[0].y = p[1].y = lp->p[0].y - wy/4;//+ wy*2/len;
					p[3].x = p[2].x = lp->p[3].x + wx/4;//- wx*2/len;
					p[3].y = p[2].y = lp->p[3].y + wy/4;//- wy*2/len;
					p[1].x += lp->infx;
					p[1].y += lp->infy;
					p[2].x += lp->infx;
					p[2].y += lp->infy;
					GrpPolygon(p,4);
				}
				GrpCircleF(x,y,len-len/4);//lp->w>>6);
			break;

			// ライン状態の場合 //
			case(LLF_LINE):
				x = (lp->x)>>6;
				y = (lp->y)>>6;
				GrpSetColor(4,4,4);
				GrpLine(x,y,x+(lp->infx),y+(lp->infy));
			break;

			// 使用中で無い場合 //
			case(LLF_DISABLE):
			break;
		}
	}

	GrpUnlock();
}

extern void LLaserClear(void)
{
	int i;

	// 存在するレーザー全てを閉じる //
	for(i=0;i<LLASER_MAX;i++){
		if(LLaser[i].flag != LLF_DISABLE)
			LLaser[i].flag = LLF_CLOSE;
	}

	SndStop(2);
}

extern void LLaserSetup(void)
{
	int i;

	for(i=0;i<LLASER_MAX;i++){
		//memset(LLaser+i,0,sizeof(LLASER_DATA));
		LLaser[i].flag = LLF_DISABLE;
		LLaser[i].e    = NULL;
	}

	SndStop(2);
}

static void _LLaserPointSet(LLASER_DATA *lp)
{
	POINT *pp = lp->p;

	pp[1].x = pp[0].x = (lp->x>>6) + lp->wx + lp->lx;
	pp[1].y = pp[0].y = (lp->y>>6) + lp->wy + lp->ly;

	pp[2].x = pp[3].x = (lp->x>>6) - lp->wx + lp->lx;
	pp[2].y = pp[3].y = (lp->y>>6) - lp->wy + lp->ly;

	pp[1].x += lp->infx;
	pp[1].y += lp->infy;

	pp[2].x += lp->infx;
	pp[2].y += lp->infy;
}

static void _LLaserHitCheck(LLASER_DATA *lp)
{
//	long tx,ty,w1,w2,length;

	int		tx,ty;
	int		length,width;

	if(Viv.muteki) return;

	tx = Viv.x - lp->x;
	ty = Viv.y - lp->y;

	length = cosl(lp->d,tx) + sinl(lp->d,ty);
	width  = abs(-sinl(lp->d,tx) + cosl(lp->d,ty));

/*
	// 計算上の注意 : 座標の計算にはx64をつかう //
	// sinm(),cosm()を使っているので/256補正が必要となる //
	tx = ((lp->x)-(Viv.x));	ty = ((lp->y)-(Viv.y));
	length = -((cosm(lp->d)*tx+sinm(lp->d)*ty)>>8);
	tx <<= 8;	ty <<= 8;

	if(cosm(lp->d)==0)
		w1 = abs((length*cosm(lp->d)+tx)/(-sinm(lp->d)));
	else if(sinm(lp->d)==0)
		w1 = abs((length*sinm(lp->d)+ty)/( cosm(lp->d)));
	else{
		w2 = abs((length*cosm(lp->d)+tx)/(-sinm(lp->d)));
		w1 = abs((length*sinm(lp->d)+ty)/( cosm(lp->d)));
		w1 = (w1+w2)/2;	// 精度アップ
	}
*/
	if(length>0 && width<=(lp->w+64*15)){
		evade_add(LLASER_EVADE);
	}
	if(length>0 && width<=(lp->w)){
		MaidDead();
	}
}

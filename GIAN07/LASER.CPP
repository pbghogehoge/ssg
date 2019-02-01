/*************************************************************************************************/
/*   LASER.C   レーザーに関する処理(無限遠,反射,ショート)                                        */
/*                                                                                               */
/*************************************************************************************************/

#include "GIAN.H"
#include "DD_GRP3D.h"


/*
 * レーザーの座標管理について、今回は次のような構造体をもたせる事にする
 *
 *    ３-----------------Length---> >----------２
 *   Width                      < <             |
 *    +(x,y)                     > >            +
 *   Width                      < <             |
 *    ０-----------------Length---> >----------１
 *
 *
 * ...始点からみて反時計回りに０～３
 *
 *
 * -> 直線との距離を求める処理を三角関数で計算したものを最適化しています
 *
 * Laser(sx,sy) Saboten(x,y) とするまた、レーザー発射角をdegreeとすると
 * 長さ判定(length)及び、幅判定の値(width)は、
 * (tx = sx-x;	ty = sy-y;)
 * length   = -(cosm(degree)*tx/256+sinm(degree)*ty/256);
 * width(1) = (length*cosm(degree)+tx*256)/(-sinm(degree));
 * width(2) = (length*sinm(degree)+ty*256)/( cosm(degree));
 */


const int laser_color[16] ={
		RGB256(0,0,3),RGB256(0,4,0),RGB256(0,3,3),RGB256(3,0,0),
		RGB256(3,0,3),RGB256(3,3,0),RGB256(3,3,3),
		RGB256(0,0,5),RGB256(0,5,0),RGB256(0,5,5),RGB256(5,0,0),
		RGB256(5,0,5),RGB256(5,5,0),RGB256(5,5,5)
};

////グローバル変数////
LASER_CMD		LaserCmd;							// 標準レーザーコマンド構造体
LASER_DATA		Laser[LASER_MAX];					// レーザー格納用構造体
WORD			LaserInd[LASER_MAX];				// レーザー順番維持用配列
WORD			LaserNow;							// レーザーの本数
//REFLECTOR		Reflector[RT_MAX];					// 反射物_構造体
//WORD			ReflectorNow;						// 反射物の個数

static void easy_cmdL(void);						// 難易度：Ｅａｓｙ
static void hard_cmdL(void);						// 難易度：Ｈａｒｄ
static void luna_cmdL(void);						// 難易度：Ｌｕｎａｔｉｃ

static BYTE __fastcall laser_dir(WORD i);				// レーザーの進行方向をセットする
static void __fastcall slaser_pset(LASER_DATA *lp);		// ４点の座標をセットする(ショート用)
static void __fastcall SLdraw( LASER_DATA *lp);			// ショートレーザー描画

static void __fastcall Lmove(LASER_DATA *lp);			// 種類により分岐し座標を更新する
static void __fastcall laser_hitchk(LASER_DATA *lp);	// レーザーの当たり判定

static void __fastcall REFL_move(LASER_DATA *lp);		// 反射レーザーの移動
static int  __fastcall REFL_hit(LASER_DATA *lp);		// ﾘﾌﾚｸﾀｰとの当たり判定(ﾋｯﾄ->非０)

static void Indsort(WORD *ind,WORD *now);


void laser_set(void)
{
	switch(PlayRank.GameLevel){
		case(GAME_EASY):
			easy_cmdL();
		break;

		case(GAME_HARD):
			hard_cmdL();
		break;

		case(GAME_LUNATIC):
			luna_cmdL();
		break;
	}

	LaserCmd.v = (((LaserCmd.v>>1)*(PlayRank.Rank))>>(5+8)) + (LaserCmd.v>>1);

	laser_setEX();
}

void laser_setEX(void)
{
	LASER_DATA *lp;
	WORD i;

	for(i=0;i<LaserCmd.n;i++){
		if(LaserNow+1 == LASER_MAX) return;		// 最大数を越えた場合

		lp = Laser + (*(LaserInd+LaserNow));
		LaserNow++;

		lp->v    = LaserCmd.v;				// レーザーの速度
		lp->a    = LaserCmd.a;				// レーザーの加速度
		lp->d    = laser_dir(i);			// レーザーの角度

		if(LaserCmd.l2){
			lp->x = LaserCmd.x + cosl(lp->d,LaserCmd.l2);
			lp->y = LaserCmd.y + sinl(lp->d,LaserCmd.l2);
		}
		else{
			lp->x    = LaserCmd.x;			// 始点のｘ座標
			lp->y    = LaserCmd.y;			// 始点のｙ座標
		}

		lp->vx   = cosl(lp->d,lp->v);		// ｄとｖからｖｘを
		lp->vy   = sinl(lp->d,lp->v);		// ｄとｖからｖｙを

		lp->w    = LaserCmd.w;				// レーザーの太さ
		lp->lmax = LaserCmd.l;				// レーザーの長さの最終値

		lp->lx = lp->ly = 0;
		lp->wx = - sinl(lp->d,lp->w>>6);	// cosl(d+64,w)
		lp->wy =   cosl(lp->d,lp->w>>6);	// sinl(d+64,w)

		lp->l    = lp->count = 0;			// 長さとカウンタをゼロ初期化

		lp->c    = LaserCmd.c;				// レーザーの色を書き込む
		lp->type = LaserCmd.type;			// レーザーの種類を書き込む

		if(lp->type==LS_REF) lp->flag = LF_SHOT;
		else lp->flag = LF_NONE;					// フラグの初期化

		lp->evade = 0;						// かすり用フラグ

		lp->notr = LaserCmd.notr;			// 反射しないリフレクター

		slaser_pset(lp);					// ４点の座標をセットする(ショート用)
	}
}

void laser_move(void)
{
	LASER_DATA *lp;
	WORD i;

	for(i=0;i<LaserNow;i++){
		lp = Laser + LaserInd[i];
		Lmove(lp);
		lp->count++;
		if((lp->x)<GX_MIN||(lp->x)>GX_MAX||(lp->y)<GY_MIN||(lp->y)>GY_MAX)
			lp->flag = LF_DELETE;

		if(Viv.muteki==0 && !(lp->flag&(LF_CLEAR|LF_DELETE))) laser_hitchk(lp);
	}
	Indsort(LaserInd,&LaserNow);
}

void laser_draw(void)
{
	LASER_DATA		*lp;
	int				i;

	GrpLock();

	for(i=0;i<LaserNow;i++){
		lp = Laser + (*(LaserInd+i));
		switch(lp->type){
			// ノーマルショートレーザー＆反射レーザー //
			case(LS_SHORT):case(LS_REF):
				SLdraw(lp);
			break;

			/*
			// 無限遠レーザー＆Ｙ_正方向_無限遠レーザー //
			case(LS_LONG):case(LS_LONGY):
				Ldraw(lp);
			break;
			*/
		}
	}

	/*
	for(i=0;i<LaserNow;i++){
		lp = Laser + (*(LaserInd+i));
		if(lp->type==LS_REF || lp->type==LS_SHORT)
			SLdraw(lp);
	}
	*/

	GrpUnlock();
}

void laser_clear(void)
{
	WORD i;

	for(i=0;i<LaserNow;i++){
		if((Laser + (*(LaserInd+i)))->flag != LF_CLEAR){
			(Laser + (*(LaserInd+i)))->flag = LF_CLEAR;
			(Laser + (*(LaserInd+i)))->count = 0;
		}
	}
}

void laserind_set(void)
{
	WORD i;

	for(i=0;i<LASER_MAX;i++){
		LaserInd[i]=i;
		//memset(Laser+i,0,sizeof(LASER_DATA));
	}

	LaserNow = 0;
}

static void easy_cmdL(void)
{
	switch(LaserCmd.cmd&0x03){
		case(LC_WAY):
			if(LaserCmd.n>=3) LaserCmd.n-=2;	// 奇数・偶数は変化させない
			LaserCmd.dw += (LaserCmd.dw>>2);	// 幅を広げる
		break;

		case(LC_ALL):case(LC_RND):
			LaserCmd.n>>=1;						// 本数／２
		break;
	}

	LaserCmd.l -= (LaserCmd.l>>2);
}

static void hard_cmdL(void)
{
	switch(LaserCmd.cmd&0x03){
		case(LC_WAY):
			LaserCmd.n+=2;						// 奇数・偶数は変化させない
			LaserCmd.dw -= (LaserCmd.dw>>3);	// 幅を狭める
		break;

		case(LC_ALL):
			LaserCmd.n += (( (LaserCmd.n>>2)>6 ) ? 6 : (LaserCmd.n>>2) );
		break;

		case(LC_RND):
			LaserCmd.n += (LaserCmd.n>>1);		// 本数５０％アップ
		break;
	}

	LaserCmd.l += (LaserCmd.l>>2);
}

static void luna_cmdL(void)
{
	switch(LaserCmd.cmd&0x03){
		case(LC_WAY):
			LaserCmd.n+=4;					// 奇数・偶数は変化させない
			LaserCmd.dw -= (LaserCmd.dw/3);	// 幅を狭める
		break;

		case(LC_ALL):
			LaserCmd.n += (( (LaserCmd.n/3)>12 ) ? 12 : (LaserCmd.n/3) );
		break;

		case(LC_RND):
			LaserCmd.n<<=1;					// 本数２倍
		break;
	}

	LaserCmd.l += (LaserCmd.l>>1);
}

static BYTE __fastcall laser_dir(WORD i)
{
	BYTE deg = 0;

	if(LaserCmd.cmd & LS_ZSET)
		deg = atan8(Viv.x-LaserCmd.x,Viv.y-LaserCmd.y);

	deg += LaserCmd.d;	// 基本角のセット完了

	switch(LaserCmd.cmd & 0x03){
		case(LC_WAY):
			i++;
			if(LaserCmd.n&1)
			 return deg+(i>>1)*LaserCmd.dw*(1-((i&1)<<1));
			else
			 return deg-(LaserCmd.dw>>1)+(i>>1)*LaserCmd.dw*(1-((i&1)<<1));

		case(LC_ALL):
			return deg + (i<<8)/LaserCmd.n;

		case(LC_RND):
			return deg + rnd()%LaserCmd.dw - (LaserCmd.dw>>1);

		default:	// ここには絶対にこないと思うのだが...
			return 0;
	}
}

static void __fastcall slaser_pset(LASER_DATA *lp)
{
	lp->p[1].x = lp->p[0].x= (lp->x>>6)+lp->wx;
	lp->p[1].y = lp->p[0].y= (lp->y>>6)+lp->wy;

	lp->p[2].x = lp->p[3].x= (lp->x>>6)-lp->wx;
	lp->p[2].y = lp->p[3].y= (lp->y>>6)-lp->wy;

	lp->p[1].x += lp->lx;
	lp->p[1].y += lp->ly;
	lp->p[2].x += lp->lx;
	lp->p[2].y += lp->ly;
}

static void __fastcall SLdraw(LASER_DATA *lp)
{
	static POINT p[4];
	int x,y;

	int tx,ty;


	if(lp->flag==LF_CLEAR){
		//GrpSetColor(laser_color[lp->c]);
		GrpSetColor(1,0,5);
		GrpLine(lp->p[0].x,lp->p[0].y,lp->p[1].x,lp->p[1].y);
		GrpLine(lp->p[3].x,lp->p[3].y,lp->p[2].x,lp->p[2].y);
		return;
	}

	x = lp->x>>6;	tx = 0;//(lp->p[0].x - x)>>2;
	y = lp->y>>6;	ty = 0;//(lp->p[0].y - y)>>2;

/* 参考
 *  レンダリングステートから考えるに、どうやらαの設定を変えてしまうと、
 *  その時点で、αが有効になってしまうドライバが存在するようである。
 *  んなもんで、ここでは GrpSetAlpha を無効にしている
 */

	//GrpSetAlpha(0,ALPHA_ONE);
	//GrpSetColor(4,1,0);
//	GrpSetAlpha(128,ALPHA_ONE);		// 2000/09/06 に削除
	GrpSetColor(1,0,5);

	if(DxObj.D3){
		GrpGrdRect(lp->p);
	}
	else{
		GrpSetColor(1,0,5);
		GrpPolygon(lp->p,4);

		GrpSetColor(5,5,5);
		p[0].x = p[1].x = lp->p[0].x - lp->wx*3/4;//- wx*4/len;
		p[0].y = p[1].y = lp->p[0].y - lp->wy*3/4;//- wy*4/len;
		p[3].x = p[2].x = lp->p[3].x + lp->wx*3/4;//+ wx*4/len;
		p[3].y = p[2].y = lp->p[3].y + lp->wy*3/4;//+ wy*4/len;
		p[1].x += lp->lx;
		p[1].y += lp->ly;
		p[2].x += lp->lx;
		p[2].y += lp->ly;
		GrpPolygon(p,4);
		//GrpLine(lp->p[0].x,lp->p[0].y,lp->p[1].x,lp->p[1].y);
		//GrpLine(lp->p[2].x,lp->p[2].y,lp->p[3].x,lp->p[3].y);
		//GrpPolygon(lp->p,4);
		//GrpSetColor(5,5,5);
		//GrpLine(x,y,x+lp->lx,y+lp->ly);
	}
	//Grp_Polygon(temp,4,RGB256(5,5,5));
}

static void __fastcall Lmove(LASER_DATA *lp)
{
	//int i;
	//int ltempx,ltempy;

	if(lp->flag==LF_CLEAR){
		if(lp->l<lp->lmax){
			lp->l += lp->v;
			lp->w += 16;
			lp->lx = cosl(lp->d,lp->l>>6);
			lp->ly = sinl(lp->d,lp->l>>6);
			lp->p[1].x = lp->p[0].x + lp->lx;
			lp->p[1].y = lp->p[0].y + lp->ly;
			lp->p[2].x = lp->p[3].x + lp->lx;
			lp->p[2].y = lp->p[3].y + lp->ly;
		}
		else	lp->w += 64;

		lp->wx = -sinl(lp->d,lp->w>>6);
		lp->wy =  cosl(lp->d,lp->w>>6);
		slaser_pset(lp);

		if(lp->count>30) lp->flag = LF_DELETE;
		return;
	}

	switch(lp->type){
		case(LS_SHORT):	// ショートレーザー
			if((lp->l) < (lp->lmax)){
				lp->l += lp->v;						// のびのび
				lp->lx = cosl(lp->d,lp->l>>6);
				lp->ly = sinl(lp->d,lp->l>>6);
				lp->p[1].x = lp->p[0].x + lp->lx;
				lp->p[1].y = lp->p[0].y + lp->ly;
				lp->p[2].x = lp->p[3].x + lp->lx;
				lp->p[2].y = lp->p[3].y + lp->ly;
			}
			else{
				lp->x += lp->vx;
				lp->y += lp->vy;
				slaser_pset(lp);
			}
		return;

		case(LS_REF):	// 反射レーザー
			REFL_move(lp);		// 長いので関数にしよう！
		return;

		/*
		case(LS_LONG):	// 無限遠レーザー
		case(LS_LONGY):	// Ｙ_正方向_無限遠レーザー
			LONGL_move(lp);
		return;
		*/
	}
}

static void __fastcall laser_hitchk(LASER_DATA *lp)
{
	long tx,ty,w1,length;

	switch(lp->type){
		case(LS_SHORT):case(LS_REF):
 			// 計算上の注意 : 座標の計算にはx64をつかう //
 			// sinm(),cosm()を使っているので/256補正が必要となる //

			tx = Viv.x - lp->x;		ty = Viv.y - lp->y;
			length =  cosl(lp->d,tx) + sinl(lp->d,ty);
			w1     = abs(-sinl(lp->d,tx) + cosl(lp->d,ty));
/*			tx = ((lp->x)-(Viv.x));	ty = ((lp->y)-(Viv.y));
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
			if(length>0 && length<=(lp->l) && w1<=(lp->w)){
				MaidDead();
			}
			else if(length>0 && length<=(lp->l) && w1<=(lp->w+LASER_EVADE_WIDTH)){
				if(lp->evade) evade_add(0);
				else{
					lp->evade = 0xff;
					evade_add(SLASER_EVADE);
				}
			}
		break;

		/*
		case(LS_LONG):
		break;

		case(LS_LONGY):
		break;
		*/
	}
}

static void __fastcall REFL_move(LASER_DATA *lp)
{
	switch(lp->flag){
		case(LF_NONE):	// E  -->  R	普通に移動
			lp->x += lp->vx;
			lp->y += lp->vy;
			slaser_pset(lp);

			// ここにヒットチェックを記述だ！ //
			if(REFL_hit(lp)) lp->flag = LF_HIT;
		return;

		case(LF_SHOT):	// E-->    R	発射中
			lp->l += lp->v;
			lp->lx = cosl(lp->d,lp->l>>6);
			lp->ly = sinl(lp->d,lp->l>>6);
			lp->p[1].x = lp->p[0].x + lp->lx;
			lp->p[1].y = lp->p[0].y + lp->ly;
			lp->p[2].x = lp->p[3].x + lp->lx;
			lp->p[2].y = lp->p[3].y + lp->ly;
			if((lp->l) >= (lp->lmax)) lp->flag = LF_NONE;

			// ここにもヒットチェックがいるぞ！ //
			// 注意 ltemp:=l を必ず実行する事 //
			if(REFL_hit(lp)){
				lp->ltemp = lp->l;
				lp->flag  |= LF_HIT;	// LF_HIT or LF_NMOVE
			}
		return;

		case(LF_HIT):	// E    -->R	ヒット中
			if((lp->l) <= (lp->v)) lp->flag = LF_DELETE;
			else                   lp->l   -= lp->v;

			lp->x += lp->vx;
			lp->y += lp->vy;
			lp->lx = cosl(lp->d,lp->l>>6);
			lp->ly = sinl(lp->d,lp->l>>6);
			lp->p[0].x = lp->p[1].x - lp->lx;
			lp->p[0].y = lp->p[1].y - lp->ly;
			lp->p[3].x = lp->p[2].x - lp->lx;
			lp->p[3].y = lp->p[2].y - lp->ly;
		return;

		case(LF_NMOVE):	// E------>R	発射中＆ヒット中
			lp->ltemp += lp->v;
			if((lp->ltemp) >= (lp->lmax)) lp->flag = LF_HIT;
		return;
	}
}

static int  __fastcall REFL_hit(LASER_DATA *lp)
{
	//REFLECTOR *rp;
	LLASER_DATA *ll;
	int i;

	// レーザーの進行方向に対して先端??? //
	long lx = lp->x + cosl(lp->d,lp->l);	// レーザーの判定ポイントX
	long ly = lp->y + sinl(lp->d,lp->l);	// レーザーの判定ポイントY

	long tx,ty;								// 演算用座標
	long length,width;						// ヒットチェック用

	// ﾘﾌﾚｸﾀｰとの当たり判定(ﾋｯﾄ->非０) //
	// d' = - d + Rd*2    : d:Laser_Degree  Rd:Reflector_Degree //

	// 太レーザーに対して反射する //
	for(i=0;i<LLASER_MAX;i++){
		if(i == lp->notr)        continue;		// 前回反射したなら無視する
		ll = LLaser + i;
		if(ll->flag != LLF_NORM) continue;		// 完全オープンでなければ次へ

		tx = lx - ll->x;
		ty = ly - ll->y;
		length = cosl(ll->d,tx) + sinl(ll->d,ty);
		width  = abs(-sinl(ll->d,tx) + cosl(ll->d,ty));

		if(length>0 && width<=ll->w){
			LaserCmd.x = lx;
			LaserCmd.y = ly;
			LaserCmd.v = lp->v;
			LaserCmd.d = - (lp->d) + ((ll->d)<<1);
			LaserCmd.w = lp->w;		//
			LaserCmd.l = lp->lmax;	// 結構重要(同じレーザーだと思わせるため)
			LaserCmd.l2 = 0;//lp->v;	//
			LaserCmd.n = 1;			// 分裂したら少々恐いので...
			LaserCmd.c = lp->c;		// 色は同じでいいでしょ
			LaserCmd.cmd = LC_WAY;	//
			LaserCmd.type = LS_REF;	// ２回以上の反射を許可
			LaserCmd.notr = i;		// 自分には反射しない
			laser_setEX();			// 難易度変更無し
			return 1;				// 当たった
		}

	}

	return 0;	// はずれ
}

static void Indsort(WORD *ind,WORD *now)
{
	WORD i,temp,next;

	for(i=temp=next=0;i<(*now);i++){

		// 消去要請フラグが立っている->swap 立っていない-> counter++ //
		if(((Laser+(*(ind+i)))->flag) & LF_DELETE){

			// フラグの立っていないレーザーを検索する //
			// temp = 1 + i;  // ((temp>i) ? temp : i);
			for(temp=i+1;temp<(*now);temp++){
				if(((Laser+(*(ind+temp)))->flag & LF_DELETE)== 0){
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

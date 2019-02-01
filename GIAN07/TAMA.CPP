/*************************************************************************************************/
/*   TAMA.cpp   たまの発射などに関する関数群                                                     */
/*                                                                                               */
/*************************************************************************************************/

#include "GIAN.H"


////グローバル変数////
TAMA_CMD		TamaCmd;				// 標準・弾コマンド構造体
TAMA_DATA		Tama[TAMA_MAX];			// 弾の格納用構造体
WORD			Tama1Ind[TAMA_MAX];		// 小型弾の順番を維持するための配列
WORD			Tama2Ind[TAMA_MAX];		// 特殊弾の順番を維持するための配列
WORD			Tama1Now;				// 小型弾の弾数
WORD			Tama2Now;				// 特殊弾の弾数
WORD			Tama1Max;				// 小型弾の最大数
WORD			Tama2Max;				// 特殊弾の最大数
int				TamaSpeed;


////ローカルな関数////
static void __TamaSet(void);
static void easy_cmd(void);				// 難易度：Ｅａｓｙ
static void hard_cmd(void);				// 難易度：Ｈａｒｄ
static void luna_cmd(void);				// 難易度：Ｌｕｎａｔｉｃ

static void Indsort(WORD *ind,WORD *now);	// 弾消去の要求によりTama?Indを更新する
int __fastcall NewTamaSpeed(WORD i);
int __fastcall LineCmdNewTamaSpeed(WORD i);
int __fastcall TamaSpeedEx(BYTE d);
void __fastcall _TamaEffectDraw(TAMA_DATA *t);	// 弾をエフェクトとして描画？


/*
_inline Debug(DWORD old,int id)
{
	RndBuf[id] += (random_ref-old);
}
*/

void tama_set(void)
{
	int		v;

	// NORMAL の場合は変更しない(ゲーム中に増減する難易度は考案中) //
	// おそらく switch 中に記述する事になるかと... //
	switch(PlayRank.GameLevel){
		case(GAME_EASY):
			easy_cmd();
		break;

		case(GAME_HARD):
			hard_cmd();
		break;

		case(GAME_LUNATIC):
			luna_cmd();
		break;
	}

	//数値は単純に　(speed /2) *rank/32 + speed/2
	v = SPEEDM(TamaCmd.v);		// 速度の基本値をセットする(GIAN.H)
	if((TamaCmd.type & 0x0f) == T_NORM)
		TamaSpeed = (((v>>1)*(PlayRank.Rank))>>(5+8)) + (v>>1);
	else
		TamaSpeed = v;

	__TamaSet();
}

void tama_setEX(void)
{
	TamaSpeed = SPEEDM(TamaCmd.v);

	__TamaSet();
}

// 弾をセットする(ライン状に発射)
void tama_setLine(void)
{
//	DWORD temp;
	TAMA_DATA *t;					// 弾配列のアクセスに用いる

	WORD i;							// 雑用
	WORD setmax;					// セットする弾数(連射を考慮に入れる)

	WORD *indnow,*indmax,*indp;		// 上に同じ


	TamaSpeed = SPEEDM(TamaCmd.v);


	// "アクセスする領域" をセットする(小型弾 or 特殊弾)        //
	if((TamaCmd.c & 0xf0) == TAMA_SMALL)
		indnow=&Tama1Now,indmax=&Tama1Max,indp=&Tama1Ind[Tama1Now];
	else
		indnow=&Tama2Now,indmax=&Tama2Max,indp=&Tama2Ind[Tama2Now];

	setmax = (TamaCmd.n)*( (TamaCmd.cmd & TAMA_REN) ? TamaCmd.ns : 1);

	// その他パラメータのセット //
	TamaCmd.cmd = (TamaCmd.cmd & 0xf0) | TC_WAY;

	for(i=0;i<setmax;i++){
		if( (*indnow)+1 >= (*indmax) ) return;	// セットできない場合

		*indnow = *indnow + 1;					// 弾数をインクリメント
		t = Tama + (*(indp+i));					// 弾ポインタをセット

		t->x = t->tx = TamaCmd.x;				// X座標のセット
		t->y = t->ty = TamaCmd.y;				// Y座標のセット

		t->a = TamaCmd.a;						// 注意：サイズは char

		//temp = random_ref;
		t->d   = tama_dir(i);					// 弾の発射角度
		//Debug(temp,31);

		t->d16 = (t->d<<8);						// 角速度のある運動で使用

		//temp = random_ref;
		t->v = t->v0 = LineCmdNewTamaSpeed(i);	// 初速度のセット
		//Debug(temp,30);

		t->vx = cosl(t->d,t->v);				// 速度のＸ成分セット
		t->vy = sinl(t->d,t->v);				// 速度のＹ成分セット

		t->vd     = TamaCmd.vd;					// 角速度もしくはホーミング率
		t->c      = TamaCmd.c;					// 弾の色＆形状
		t->rep    = TamaCmd.rep;				// 繰り返し回数
		t->type   = TamaCmd.type;				// 弾の種類
		t->option = TamaCmd.option;				// 弾の属性(バイブ、反射等)
		t->effect = TamaCmd.cmd & 0xf0;			// 弾のエフェクト
		t->count  = 0;							// カウンタの初期化
		t->flag   = tama_flag();				// フラグの初期化
	}
}

// エキストラボス専用弾幕(角度が広くなると、遅くなる) //
void tama_setExtra01(void)
{
//	DWORD temp;
	TAMA_DATA *t;					// 弾配列のアクセスに用いる

	WORD i;							// 雑用
	WORD setmax;					// セットする弾数(連射を考慮に入れる)

	WORD *indnow,*indmax,*indp;		// 上に同じ


	TamaSpeed = SPEEDM(TamaCmd.v);


	// "アクセスする領域" をセットする(小型弾 or 特殊弾)        //
	if((TamaCmd.c & 0xf0) == TAMA_SMALL)
		indnow=&Tama1Now,indmax=&Tama1Max,indp=&Tama1Ind[Tama1Now];
	else
		indnow=&Tama2Now,indmax=&Tama2Max,indp=&Tama2Ind[Tama2Now];

	// "セットする弾数" をセットする //
	setmax = (TamaCmd.n)*( (TamaCmd.cmd & TAMA_REN) ? TamaCmd.ns : 1);

	// その他パラメータのセット //
	//TamaCmd.cmd = (TamaCmd.cmd & 0xf0);

	for(i=0;i<setmax;i++){
		if( (*indnow)+1 >= (*indmax) ) return;	// セットできない場合

		*indnow = *indnow + 1;					// 弾数をインクリメント
		t = Tama + (*(indp+i));					// 弾ポインタをセット

		t->x = t->tx = TamaCmd.x;				// X座標のセット
		t->y = t->ty = TamaCmd.y;				// Y座標のセット

		t->a = TamaCmd.a;						// 注意：サイズは char

		t->d   = tama_dir(i);					// 弾の発射角度
		t->d16 = (t->d<<8);						// 角速度のある運動で使用

		t->v = t->v0 = TamaSpeedEx(t->d);		// 初速度のセット

		t->vx = cosl(t->d,t->v);				// 速度のＸ成分セット
		t->vy = sinl(t->d,t->v);				// 速度のＹ成分セット

		t->vd     = TamaCmd.vd;					// 角速度もしくはホーミング率
		t->c      = TamaCmd.c;					// 弾の色＆形状
		t->rep    = TamaCmd.rep;				// 繰り返し回数
		t->type   = TamaCmd.type;				// 弾の種類
		t->option = TamaCmd.option;				// 弾の属性(バイブ、反射等)
		t->effect = TamaCmd.cmd & 0xf0;			// 弾のエフェクト
		t->count  = 0;							// カウンタの初期化
		t->flag   = tama_flag();				// フラグの初期化
	}
}

int __fastcall TamaSpeedEx(BYTE d)
{
	int		temp = 0;
	int		delta;

	switch(TamaCmd.v & 0xc0){
		case(TAMASP_RND1): temp = rnd()%16-8 ; /*DebugOut("2");*/	break;
		case(TAMASP_RND2): temp = rnd()%32-16; /*DebugOut("3");*/	break;
		case(TAMASP_RND3): temp = rnd()%64-32; /*DebugOut("4");*/	break;
	}

	// d と TamaCmd.d の値の離れ具合により、速度を変化させる //
	delta = TamaCmd.d - d;
	if(delta >  128) delta -= 256;
	if(delta < -128) delta += 256;

	return TamaSpeed - (TamaSpeed * abs(delta)) / 23 + temp;
}

static void __TamaSet(void)
{
//	DWORD temp;


	TAMA_DATA *t;					// 弾配列のアクセスに用いる

	WORD i;							// 雑用
	WORD setmax;					// セットする弾数(連射を考慮に入れる)

	WORD *indnow,*indmax,*indp;		// 上に同じ


	// "アクセスする領域" をセットする(小型弾 or 特殊弾)        //
	if((TamaCmd.c & 0xf0) == TAMA_SMALL)
		indnow=&Tama1Now,indmax=&Tama1Max,indp=&Tama1Ind[Tama1Now];
	else
		indnow=&Tama2Now,indmax=&Tama2Max,indp=&Tama2Ind[Tama2Now];

	// "セットする弾数" をセットする //
	setmax = (TamaCmd.n)*( (TamaCmd.cmd & TAMA_REN) ? TamaCmd.ns : 1);

	for(i=0;i<setmax;i++){
		if( (*indnow)+1 >= (*indmax) ) return;	// セットできない場合

		*indnow = *indnow + 1;					// 弾数をインクリメント
		t = Tama + (*(indp+i));					// 弾ポインタをセット

		t->x = t->tx = TamaCmd.x;				// X座標のセット
		t->y = t->ty = TamaCmd.y;				// Y座標のセット


		//temp = random_ref;
		t->v = t->v0 = NewTamaSpeed(i);			// 初速度のセット
		//Debug(temp,30);

		t->a = TamaCmd.a;						// 注意：サイズは char

		//temp = random_ref;
		t->d   = tama_dir(i);					// 弾の発射角度
		//Debug(temp,31);

		t->d16 = (t->d<<8);						// 角速度のある運動で使用

		t->vx = cosl(t->d,t->v);				// 速度のＸ成分セット
		t->vy = sinl(t->d,t->v);				// 速度のＹ成分セット

		t->vd     = TamaCmd.vd;					// 角速度もしくはホーミング率
		t->c      = TamaCmd.c;					// 弾の色＆形状
		t->rep    = TamaCmd.rep;				// 繰り返し回数
		t->type   = TamaCmd.type;				// 弾の種類
		t->option = TamaCmd.option;				// 弾の属性(バイブ、反射等)
		t->effect = TamaCmd.cmd & 0xf0;			// 弾のエフェクト
		t->count  = 0;							// カウンタの初期化
		t->flag   = tama_flag();				// フラグの初期化
	}
}

void tama_move(void)
{
	// ヒットチェック後にサボテンの生死判定をしているのは、死んでいる時間 //
	// よりも生きている時間のほうが長いからなのですが...                  //

	WORD i;
	TAMA_DATA *t;

	// 小型弾の処理 //
	for(i=0;i<Tama1Now;i++){
		t = Tama + *(Tama1Ind+i);
		if(t->effect==TE_NONE){
			tamaTmove(t);
			tamaOmove(t);
			if(((t->flag&TF_CLIP)==0)&&
				((t->x)<GX_MIN-4*64||(t->x)>GX_MAX+4*64||(t->y)<GY_MIN-4*64||(t->y)>GY_MAX+4*64))
				t->flag = TF_DELETE;
			t->count++;
			if(Viv.muteki) continue;
			if(HITCHK(t->x,Viv.x,TAMA_EVX_SMALL) && HITCHK(t->y,Viv.y,TAMA_EVY_SMALL)){
				TamaEvadeAdd(t);
			}
			if(HITCHK(t->x,Viv.x,TAMA_HITX) && HITCHK(t->y,Viv.y,TAMA_HITY)){
				t->flag = TF_DELETE;
				MaidDead();
			}
		}
		else{
			tamaEmove(t);
			t->count++;
		}
	}
	Indsort(Tama1Ind,&Tama1Now);

	// 大型弾＆特殊弾の処理 //
	for(i=0;i<Tama2Now;i++){
		t = Tama + *(Tama2Ind+i);
		if(t->effect==TE_NONE){
			tamaTmove(t);
			tamaOmove(t);
			if(((t->flag&TF_CLIP)==0)&&
				((t->x)<GX_MIN-8*64||(t->x)>GX_MAX+8*64||(t->y)<GY_MIN-8*64||(t->y)>GY_MAX+8*64))
				t->flag = TF_DELETE;
			t->count++;
			if(Viv.muteki) continue;
			if(HITCHK(t->x,Viv.x,TAMA_EVX_LARGE) && HITCHK(t->y,Viv.y,TAMA_EVY_LARGE)){
				TamaEvadeAdd(t);
			}
			if(HITCHK(t->x,Viv.x,TAMA_HITX) && HITCHK(t->y,Viv.y,TAMA_HITY)){
				t->flag = TF_DELETE;
				MaidDead();
			}
		}
		else{
			tamaEmove(t);
			t->count++;
		}
	}
	Indsort(Tama2Ind,&Tama2Now);
}

void tama_draw(void)
{
	WORD		i;
	TAMA_DATA	*t;
//	HRESULT		ddrval;
	RECT		src;
	int			x,y;
	int			dx,dy;
	BYTE		d;

	static const RECT	rcExtraTama[4] = {
		{128   , 384, 128+32, 384+32},
		{128+32, 384, 128+56, 384+24},
		{128+56, 384, 128+72, 384+16},
		{128+72, 384, 128+80, 384+ 8}
	};

	static const BYTE sizeExtraTama[4] = {
		16, 12, 8, 4
	};

	// 大型弾＆特殊弾(16*16) の描画 //
	for(i=0;i<Tama2Now;i++){
		t = Tama + *(Tama2Ind+i);

		x = (t->x >> 6)-8;	// -8 は座標の補正用です
		y = (t->y >> 6)-8;	// 上に同じ

		switch(t->effect){
			case(TE_DELETE):
				BltSetRect(&src,384+((t->count/6)<<4),104,16,16);
				GrpBlt(&src,x,y,GrTama);
			continue;

			case(TE_CIRCLE1):
				_TamaEffectDraw(t);
			continue;

			// その他は知らぬ //
		}

		switch(t->c&0xf0){
			case(TAMA_LARGE):	// 大型丸弾
				src.top    = 8;
				src.left   = ((t->c&0x0f)<<4)+384;
				src.bottom = 24;
				src.right  = src.left + 16;
			break;

			case(TAMA_EXTRA):
				d   = t->c & 3;
				src = rcExtraTama[d];
				x   = (t->x>>6) - sizeExtraTama[d];
				y   = (t->y>>6) - sizeExtraTama[d];
				GrpBlt(&src,x,y,GrEnemy);
			continue;

			case(TAMA_EXTRA2):
				d  = (BYTE)(t->d+4)/8;	// 256(-1) -> 32(-1) に変換
				src.top    = 320 + ((t->c & 3)<<4);	// (c mod 4) * 16
				src.left   = d * 16;
				src.bottom = src.top  + 16;
				src.right  = src.left + 16;
				//x   = (t->x>>6) - 8;	// サイズは１６で固定
				//y   = (t->y>>6) - 8;	// すなわち、そのままでＯＫ！
				GrpBlt(&src,x,y,GrEnemy);
			continue;

			case(TAMA_ANGLE):
			//default:		// 角度アニメーション系
				if(t->c != 32+5){
					src.top    = 24 + ((t->c&0x0f)<<4);
					src.left   = ((t->d+8)&0xf0) + 384;
					src.bottom = src.top  + 16;
					src.right  = src.left + 16;
				}
				else{
					//d  = (BYTE)(t->d+8)/8;
					d  = (BYTE)(t->d+4)/8;		// 修正 8 ごとで 32 分割だからズラシは 4
					dx = (d%8) * 32;
					dy = (d/8) * 32;
					src.top  = 304 + dy;
					src.left = 384 + dx;
					src.bottom = src.top  + 32;
					src.right  = src.left + 32;
					// 注意：すでに(x,y)から(8,8)が減算されているので、
					// ここでは(-16,-16)への補正のためにそれぞれ８を引く
					x-=8;	// ここであたり判定座標の
					y-=8;	// 補正を行うのだ
				}
			break;
		}

		GrpBlt(&src,x,y,GrTama);
	}

	// 小型弾(8*8) の描画 //
	for(i=0;i<Tama1Now;i++){
		t = Tama + *(Tama1Ind+i);

		x = (t->x >> 6)-4;	// -4 は座標の補正用です
		y = (t->y >> 6)-4;	// 上に同じ

		switch(t->effect){
			case(TE_DELETE):
				BltSetRect(&src,384+((t->count/6)<<3),120,8,8);
				GrpBlt(&src,x,y,GrTama);
			continue;

			case(TE_CIRCLE1):
				_TamaEffectDraw(t);
			continue;

			// その他は知らぬ //
		}

		if(t->c!=0x25){
			src.top    = 0;
			src.left   = ((t->c)<<3) + 384; //0; //(t->d+8)&0xf0;
			src.bottom = 8;
			src.right  = src.left + 8;
		}
		else{
			src.top    = 24 + ((t->c&0x0f)<<4);
			src.left   = ((t->d+8)&0xf0) + 384;
			src.bottom = src.top  + 16;
			src.right  = src.left + 16;
		}

		GrpBlt(&src,x,y,GrTama);
	}
}


// 弾をエフェクトとして描画？ //
void __fastcall _TamaEffectDraw(TAMA_DATA *t)
{
#define RCSET(x, y, w) {(x), (y), (x+w), (y+w)}

	static RECT Data[6][5] = {	// [色][パターン]
		{	// 赤 //
			RCSET(168, 344, 32), RCSET(232, 344, 28),
			RCSET(288, 344, 24), RCSET(336, 344, 20),
			RCSET(328, 416, 16),
		},
		{	// 青 //
			RCSET(168, 344+32, 32), RCSET(232, 344+28, 28),
			RCSET(288, 344+24, 24), RCSET(336, 344+20, 20),
			RCSET(328+16, 416, 16),
		},
		{	// 緑 //
			RCSET(168, 344+32*2, 32), RCSET(232, 344+28*2, 28),
			RCSET(288, 344+24*2, 24), RCSET(336, 344+20*2, 20),
			RCSET(328+16*2, 416, 16),
		},
		{	// 紫 //
			RCSET(168+32, 344, 32), RCSET(232+28, 344, 28),
			RCSET(288+24, 344, 24), RCSET(336+20, 344, 20),
			RCSET(328, 416+16, 16),
		},
		{	// 銀 //
			RCSET(168+32, 344+32, 32), RCSET(232+28, 344+28, 28),
			RCSET(288+24, 344+24, 24), RCSET(336+20, 344+20, 20),
			RCSET(328+16, 416+16, 16),
		},
		{	// 橙 //
			RCSET(168+32, 344+32*2, 32), RCSET(232+28, 344+28*2, 28),
			RCSET(288+24, 344+24*2, 24), RCSET(336+20, 344+20*2, 20),
			RCSET(328+16*2, 416+16, 16),
		},
	};
#undef RCSET

	static int  Width[5]     = {32/2, 28/2, 24/2, 20/2, 16/2};
	static RECT *Target[16*3] =
	{
		Data[0], Data[1], Data[2], Data[3], Data[4], Data[5], Data[0], Data[0],
		Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0],

		Data[0], Data[1], Data[2], Data[3], Data[4], Data[5], Data[0], Data[0],
		Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0],

		Data[0], Data[1], Data[5], Data[3], Data[4], Data[5], Data[0], Data[0],
		Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0], Data[0],
	};

	RECT	temp;
	int		ptn = (t->count/4) % 5;
	int		x,y;

	x = (t->x >> 6) - Width[ptn];
	y = (t->y >> 6) - Width[ptn];

	// [色][パターン]
	//temp = Data[(t->c&0x0f)%6][ptn];
	if(t->c >= 16*3) temp = Target[3][ptn];
	else             temp = Target[t->c][ptn];
	GrpBlt(&temp, x, y, GrTama);
}


void tama_clear(void)
{
	WORD i;

	for(i=0;i<Tama1Now;i++){
		if((Tama+ *(Tama1Ind+i))->effect!=TE_DELETE){
			(Tama + *(Tama1Ind+i))->effect = TE_DELETE;
			(Tama + *(Tama1Ind+i))->count  = 0;
			//(Tama + *(Tama1Ind+i))->c      = 0x25;
			(Tama + *(Tama1Ind+i))->d      = 0;
		}
	}

	for(i=0;i<Tama2Now;i++){
		if((Tama+ *(Tama2Ind+i))->effect!=TE_DELETE){
			(Tama + *(Tama2Ind+i))->effect = TE_DELETE;
			(Tama + *(Tama2Ind+i))->count  = 0;
			//(Tama + *(Tama2Ind+i))->c      = 0x25;
			(Tama + *(Tama2Ind+i))->d      = 0;
		}
	}
}

// 弾を得点化する(Ret : 得点)
DWORD tama2score(void)
{
	WORD		i;
	TAMA_DATA	*t;
	DWORD		sum = 0;
	DWORD		Score;

	Score = TAMA1_POINT + Viv.evade * 100;
	for(i=0;i<Tama1Now;i++){
		t = Tama + *(Tama1Ind+i);
		if(t->effect!=TE_DELETE){
			StringEffect2(t->x-64*4,t->y-64*4,Score);
			sum += Score;
			t->flag   = TF_DELETE;
			t->count  = 0;
			t->c      = 0x25;
			t->d      = 0;
		}
	}
	Indsort(Tama1Ind,&Tama1Now);

	Score = TAMA2_POINT + Viv.evade * 100;
	for(i=0;i<Tama2Now;i++){
		t = Tama + *(Tama2Ind+i);
		if(t->effect!=TE_DELETE){
			StringEffect2(t->x-64*8,t->y-64*8,Score);
			sum += Score;
			t->flag   = TF_DELETE;
			t->count  = 0;
			t->c      = 0x25;
			t->d      = 0;
		}
	}
	Indsort(Tama2Ind,&Tama2Now);

	return sum;
}

// 弾をアイテム化する //
void tama2item(BYTE n)
{
	WORD		i;
	TAMA_DATA	*t;
//	DWORD		sum = 0;
//	DWORD		Score;

//	Score = TAMA1_POINT + Viv.evade * 100;

	if(n == 0){
		tama_clear();
		return;
	}

	for(i=0;i<Tama1Now;i++){
		t = Tama + *(Tama1Ind+i);
		if(t->effect!=TE_DELETE){
			t->count  = 0;
			t->d      = 0;

			if(rnd()%n == 0){
				ItemSet(t->x, t->y, ITEM_SCORE);
				t->flag   = TF_DELETE;
				t->c      = 0x25;
			}
			else{
				t->effect = TE_DELETE;
				t->count  = 0;
				t->d      = 0;
			}
		}
	}
	Indsort(Tama1Ind,&Tama1Now);

//	Score = TAMA2_POINT + Viv.evade * 100;
	for(i=0;i<Tama2Now;i++){
		t = Tama + *(Tama2Ind+i);
		if(t->effect!=TE_DELETE){
			t->count  = 0;
			t->d      = 0;

			if(rnd()%n == 0){
				ItemSet(t->x, t->y, ITEM_SCORE);
				t->flag   = TF_DELETE;
				t->c      = 0x25;
			}
			else{
				t->effect = TE_DELETE;
				t->count  = 0;
				t->d      = 0;
			}
		}
	}
	Indsort(Tama2Ind,&Tama2Now);

//	return sum;
}

void tamaind_set(WORD tama1)
{
	int i;

	if(tama1>=TAMA_MAX) tama1 = TAMA_MAX-1;

	// 弾の最大数のセット //
	Tama1Max=tama1;
	Tama2Max=TAMA_MAX-tama1;

	// 弾のインデックス用配列の初期化 //
	for(i=0;i<tama1;i++)			Tama1Ind[i]       = i;
	for(i=tama1;i<TAMA_MAX;i++)		Tama2Ind[i-tama1] = i;

	//memset(Tama,0,sizeof(TAMA_DATA)*TAMA_MAX);

	Tama1Now = Tama2Now = 0;
}

static void easy_cmd(void)
{
	switch(TamaCmd.cmd&0x03){
		case(TC_WAY):
			if(TamaCmd.n>=3) TamaCmd.n-=2;	// 奇数・偶数は変化させない
			TamaCmd.dw += (TamaCmd.dw>>2);	// 幅を広げる
		break;

		case(TC_ALL):case(TC_RND):
			TamaCmd.n>>=1;					// 弾数／２
		break;
	}

	if(TamaCmd.ns>=2) TamaCmd.ns--;			// 連射数_減少
}

static void hard_cmd(void)
{
	switch(TamaCmd.cmd&0x03){
		case(TC_WAY):
			TamaCmd.n+=2;					// 奇数・偶数は変化させない
			TamaCmd.dw -= (TamaCmd.dw>>3);	// 幅を狭める
		break;

		case(TC_ALL):
			TamaCmd.n += (( (TamaCmd.n>>2)>6 ) ? 6 : (TamaCmd.n>>2) );
		break;

		case(TC_RND):
			TamaCmd.n += (TamaCmd.n>>1);		// 弾数５０％アップ
		break;
	}

	TamaCmd.ns++;		// 連射数_増加
}

static void luna_cmd(void)
{
	switch(TamaCmd.cmd&0x03){
		case(TC_WAY):
			TamaCmd.n+=4;					// 奇数・偶数は変化させない
			TamaCmd.dw -= (TamaCmd.dw/3);	// 幅を狭める
		break;

		case(TC_ALL):
			TamaCmd.n += (( (TamaCmd.n/3)>12 ) ? 12 : (TamaCmd.n/3) );
		break;

		case(TC_RND):
			TamaCmd.n<<=1;					// 弾数２倍
		break;
	}

	TamaCmd.ns+=2;		// 連射数_増加
}

BYTE __fastcall tama_dir(WORD i)
{
	BYTE deg;

	if(TamaCmd.cmd & TAMA_ZSET)
		deg = atan8(Viv.x-TamaCmd.x,Viv.y-TamaCmd.y);
	else
		deg = 0;

	deg += TamaCmd.d;	// 基本角のセット完了
	i = i%TamaCmd.n;	// 連射弾対策

	switch(TamaCmd.cmd & 0x03){
		case(TC_WAY):
			i++;
			if(TamaCmd.n&1)
				return deg+(i>>1)*TamaCmd.dw*(1-((i&1)<<1));
			else
				return deg-(TamaCmd.dw>>1)+(i>>1)*TamaCmd.dw*(1-((i&1)<<1));

		case(TC_ALL):
			return deg + (i<<8)/TamaCmd.n;

		case(TC_RND):
			//DebugOut("1");
			return deg + rnd()%TamaCmd.dw - (TamaCmd.dw>>1);

		default:
			return 0;	// 絶対無いけれど、warning がうるさいので...
	}
}

int __fastcall NewTamaSpeed(WORD i)
{
	int temp = 0;						// ランダム要素の設定用
	int vret = TamaSpeed; //SPEEDM(TamaCmd.v);		// 速度の基本値をセットする(GIAN.H)

	// 速度ランダムは基本値のｎ％変化とするべきかもしれないが... //
	switch(TamaCmd.v & 0xc0){
		case(TAMASP_RND1): temp = rnd()%16-8 ; /*DebugOut("2");*/	break;
		case(TAMASP_RND2): temp = rnd()%32-16; /*DebugOut("3");*/	break;
		case(TAMASP_RND3): temp = rnd()%64-32; /*DebugOut("4");*/	break;
	}

	if(TamaCmd.cmd & TAMA_REN)
		return vret + (vret>>3)*(i/TamaCmd.n) + temp;
	else
		return vret + temp;
}

int __fastcall LineCmdNewTamaSpeed(WORD i)
{
	int		vret = TamaSpeed;		// 速度の基本値をセットする(GIAN.H)
	BYTE	deg;					// 中心からの角度

	i = (i%TamaCmd.n)+1;	// 連射弾対策

	if(TamaCmd.n&1)
		deg = (i>>1)*TamaCmd.dw*(1-((i&1)<<1));
	else
		deg = -(TamaCmd.dw>>1)+(i>>1)*TamaCmd.dw*(1-((i&1)<<1));

	vret = cosDiv(deg, vret);

	if(TamaCmd.cmd & TAMA_REN)
		return vret + (vret>>3)*(i-1);
	else
		return vret;
}

int __fastcall tama_speed(WORD i)
{
	int temp = 0;						// ランダム要素の設定用
	int vret = SPEEDM(TamaCmd.v);		// 速度の基本値をセットする(GIAN.H)

	// 速度ランダムは基本値のｎ％変化とするべきかもしれないが... //
	switch(TamaCmd.v & 0xc0){
		case(TAMASP_RND1): temp = rnd()%16-8 ; /*DebugOut("2");*/	break;
		case(TAMASP_RND2): temp = rnd()%32-16; /*DebugOut("3");*/	break;
		case(TAMASP_RND3): temp = rnd()%64-32; /*DebugOut("4");*/	break;
	}

	if(TamaCmd.cmd & TAMA_REN)
		return vret + (vret>>3)*(i/TamaCmd.n) + temp;
	else
		return vret + temp;
}

BYTE __fastcall tama_flag(void)
{
	switch(TamaCmd.type){
		case(T_HOMING):case(T_HOMING_M):
		case(T_ROLL):case(T_ROLL_A):case(T_ROLL_R):case(T_SBHOMING):
		return TF_CLIP;

		default:
		return TF_NONE;
	}
}

void __fastcall tamaTmove(TAMA_DATA *t)
{
	short		deg_t;
	//ENEMY_DATA	*e;

	// (x,y)に直接アクセスするのではなく、(tx,ty)にアクセスする事！ //
	switch(t->type & 0x0f){
		case(T_NORM):		// 通常弾
			//MMX_ADD32(&t->tx,&t->vx);
			t->tx += t->vx;
			t->ty += t->vy;
		return;

		case(T_NORM_A):		// 加速弾
			t->v  += t->a;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if(t->rep == t->count){
				t->type = (t->type&0xf0) | T_NORM;	// 上位ビットは一応保存する
				t->vx   = cosl(t->d,t->v);
				t->vy   = sinl(t->d,t->v);
			}
		return;

		case(T_HOMING):		// ｎ回ホーミング
			t->v  += t->a;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if( (t->a>0) && (t->v>=t->v0) ){
				t->a = -(t->a);
				if(--(t->rep)==0){
					t->type = (t->type&0xf0) | T_NORM;
					t->flag &= (~TF_CLIP);
					t->vx   = cosl(t->d,t->v);
					t->vy   = sinl(t->d,t->v);
				}
			}
			if( (t->a<0) && (t->v<=0 ) ){
				t->a  = -(t->a);
				t->d  = atan8((Viv.x)-(t->x),(Viv.y)-(t->y));
			}
		return;

		case(T_HOMING_M):	// ｎ％ホーミング(ミサイル系？)
			// 最適化はしておりませんな... //
			if( (t->count>19) && (t->count%2==0) ){
				deg_t=atan8((Viv.x)-(t->x),(Viv.y)-(t->y))-(t->d);
				if(deg_t<-128) deg_t += 256;
				if(deg_t> 128) deg_t -= 256;
				t->d = t->d + deg_t*(t->vd)/255;
			}
			t->v  += t->a;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if(t->rep == t->count){
				t->type = (t->type&0xf0) | T_NORM;	// 上位ビットは一応保存する
				t->flag &= (~TF_CLIP);
				t->vx   = cosl(t->d,t->v);
				t->vy   = sinl(t->d,t->v);
			}
		return;

		case(T_ROLL):		// 回転弾
			t->d += (char)t->vd;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if(t->rep == t->count){
				t->type = (t->type&0xf0) | T_NORM;	// 上位ビットは一応保存する
				t->flag &= (~TF_CLIP);
				t->vx   = cosl(t->d,t->v);
				t->vy   = sinl(t->d,t->v);
			}
		return;

		case(T_ROLL_A):		// 回転弾(加速) 最初の加速度は"負"にして下さい！
			t->v  += t->a;
			if(t->a>0)	t->d += (char)t->vd;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if( (t->a<0) && (t->v<=0 ) ) t->a = -(t->a);
			if( (t->a>0) && (t->v>=t->v0) ){
				t->a = -(t->a);
				if(--(t->rep)==0){
					t->type = (t->type&0xf0) | T_NORM;
					t->flag &= (~TF_CLIP);
					t->vx   = cosl(t->d,t->v);
					t->vy   = sinl(t->d,t->v);
				}
			}
		return;

		case(T_ROLL_R):		// 回転弾(反転) 上と同じで加速度に注意！
			t->v += t->a;
			t->d += (char)t->vd;
			t->tx += cosl(t->d,t->v);
			t->ty += sinl(t->d,t->v);
			if( (t->a<0) && (t->v<=0 ) ){
				t->d   += 128;
				t->a    = -(t->a);
			}
			if( (t->a>0) && (t->v>=t->v0) ){
				t->a = -(t->a);
				if(--(t->rep)==0){
					t->type = (t->type&0xf0) | T_NORM;
					t->flag &= (~TF_CLIP);
					t->vx   = cosl(t->d,t->v);
					t->vy   = sinl(t->d,t->v);
				}
			}
		return;

		case(T_GRAVITY):	// 落下弾(上昇弾にもできるが...)
			t->vy += t->a;
			//MMX_ADD32(&t->tx,&t->vx);
			t->tx += t->vx;
			t->ty += t->vy;
		return;

		case(T_CHANGE):		// 角度強制変更弾
			//MMX_ADD32(&t->tx,&t->vx);
			t->tx += t->vx;
			t->ty += t->vy;
			if(t->rep == t->count){
				t->type = (t->type&0xf0) | T_NORM;	// 上位ビットは一応保存する
				t->d    = (BYTE)t->vd;
				t->vx   = cosl(t->d,t->v);
				t->vy   = sinl(t->d,t->v);
			}
		return;

		case(T_SBHOMING):	// サボテン用ホーミング(煙を吐き出すぞ！)
			if(t->count&1) fragment_set(t->x,t->y,FRG_SMOKE);
			t->tx += t->vx;
			t->ty += t->vy;
			if((t->count<130-60) && HomingFlag!=HOMING_DUMMY){
				deg_t=atan8(HomingX-(t->x),HomingY-(t->y))-(t->d);
			}
			else if(t->count<130-60){
				deg_t=atan8(0,(-20*64)-(t->y))-(t->d);
			}
			else{
				t->flag = TF_NONE;
				deg_t=0;
			}

			if(deg_t<-128) deg_t += 256;
			if(deg_t> 128) deg_t -= 256;
			//if(deg_t>-2 && deg_t<2){
			if(deg_t==0){
				if(t->vd) t->vd--;
				t->v += t->a;
			}
			else{
				if(t->vd<250) t->vd++;
				t->v -= t->a;
			}
			t->d  = t->d + deg_t*((BYTE)t->vd)/255;
			t->vx = cosl(t->d,t->v);
			t->vy = sinl(t->d,t->v);
		return;

		case(T_SBHBOMB):	// サボテン用ホーミングボム
			// ちゅうい : この case はダミーです決して実行されてはいけません //
			if(t->count>=49) t->flag = TF_DELETE;
		return;
	}
}

void __fastcall tamaOmove(TAMA_DATA *t)
{
	int op_temp = 0;

	// 分裂はとボムは消去要請フラグを立てる必要がある //
	// (x,y)に(tx,ty)の演算結果を利用して、値を代入する //
	switch(t->option & 0xf0){
		case(TOP_NONE):		// オプション無し
			t->x = t->tx;
			t->y = t->ty;
		return;

		case(TOP_WAVE):		// 波
			op_temp = sinl((BYTE)(t->count<<2),(t->option&0x0f)<<7);
			t->x    = t->tx - sinl(t->d,op_temp);
			t->y    = t->ty + cosl(t->d,op_temp);
		return;

		case(TOP_ROLL):		// 回転
			op_temp = (t->option&0x0f)<<8;
			t->x    = t->tx + cosl((BYTE)((t->d)+(t->count<<1)),op_temp);
			t->y    = t->ty + sinl((BYTE)((t->d)+(t->count<<1)),op_temp);
		return;

		case(TOP_PURU):		// ぷるぷる
		return;

		case(TOP_REFX):		// 反射Ｘ
			if((t->tx)<GX_MIN||(t->tx)>GX_MAX){
				t->d  = 128 - t->d;
				t->vx = -(t->vx);
				t->x  = t->tx + cosl(t->d,t->v);
				t->y  = t->ty + sinl(t->d,t->v);
				op_temp = (t->option&0x0f);
				if(op_temp == 0)
					t->option = TOP_NONE;
				else
					t->option = TOP_REFX | (op_temp-1);
			}
			else{
				t->x = t->tx;
				t->y = t->ty;
			}
		return;

		case(TOP_REFY):		// 反射Ｙ
			if((t->ty)<GY_MIN){
				t->d  = - t->d;
				t->vy = -(t->vy);
				t->x  = t->tx + cosl(t->d,t->v);
				t->y  = t->ty + sinl(t->d,t->v);
				op_temp = (t->option&0x0f);
				if(op_temp == 0)
					t->option = TOP_NONE;
				else
					t->option = TOP_REFY | (op_temp-1);
			}
			else{
				t->x = t->tx;
				t->y = t->ty;
			}
		return;

		case(TOP_REFXY):	// 反射ＸＹ
			if((t->tx)<GX_MIN||(t->tx)>GX_MAX){
				t->d  = 128 - t->d;
				t->vx = -(t->vx);
				t->x  = t->tx + cosl(t->d,t->v);
				t->y  = t->ty + sinl(t->d,t->v);
				op_temp = (t->option&0x0f);
				if(op_temp == 0)
					t->option = TOP_NONE;
				else
					t->option = TOP_REFXY | (op_temp-1);
			}
			else if((t->ty)<GY_MIN){
				t->d  = - t->d;
				t->vy = -(t->vy);
				t->x  = t->tx + cosl(t->d,t->v);
				t->y  = t->ty + sinl(t->d,t->v);
				op_temp = (t->option&0x0f);
				if(op_temp == 0)
					t->option = TOP_NONE;
				else
					t->option = TOP_REFXY | (op_temp-1);
			}
			else{
				t->x = t->tx;
				t->y = t->ty;
			}
		return;

		case(TOP_DIV):		// 分裂
			t->x = t->tx;
			t->y = t->ty;
			if((t->tx)<GX_MIN||(t->tx)>GX_MAX){
				op_temp   = 1;
				TamaCmd.d = 128 - (t->d);
			}
			else if((t->ty)<GY_MIN){
				op_temp   = 1;
				TamaCmd.d = - (t->d);
			}

			if(op_temp == 1){
				TamaCmd.x = t->tx + cosl(TamaCmd.d,t->v);
				TamaCmd.y = t->ty + sinl(TamaCmd.d,t->v);
				t->flag   = TF_DELETE;			// 消滅エフェクトに変更すべきか？
				TamaCmd.ns = 2;
				TamaCmd.c = (t->c)&0x0f;
				TamaCmd.cmd = (t->option & 0x0f) | TE_CIRCLE1;
				switch(TamaCmd.cmd&0x03){
					case(TC_WAY):
						TamaCmd.n = 3;
						TamaCmd.dw = 16;
						TamaCmd.v  = 13-2;
					break;
					case(TC_ALL):
						TamaCmd.n  = 10;
						TamaCmd.v  = 13;
						TamaCmd.d  = (BYTE)rnd();
						if(TamaCmd.cmd&TAMA_REN) TamaCmd.v -= 2;
					break;
					case(TC_RND):
						TamaCmd.n = 4;
						TamaCmd.dw = 128-32;		// 128以上だと画面外に...
						TamaCmd.v  = 13|TAMASP_RND2;	// 速度ランダムあり
					break;
				}
				if(TamaCmd.cmd&TAMA_ZSET) TamaCmd.d=0,TamaCmd.dw-=6;
				TamaCmd.type = T_NORM;
				TamaCmd.option = TOP_NONE;
				SndPlayEX(12, TamaCmd.x, 0);
				tama_set();		// 難易度で変化させるところがポイント
			}
		return;

		case(TOP_BOMB):		// ボム
		return;
	}
}

void __fastcall tamaEmove(TAMA_DATA *t)
{
	// TE_NONE:エフェクト無しはこの関数にこないので記述しても意味無し //
	// TE_DELETE:消去要請フラグを立てる事を忘れないように！ //
	switch(t->effect & 0xf0){
		case(TE_ROLL1):
		return;

		case(TE_ROLL2):
		return;

		case(TE_WARN):
		return;

		case(TE_ROCK):
		return;

		case(TE_CIRCLE1):
			t->x = (t->tx += (t->vx>>1));
			t->y = (t->ty += (t->vy>>1));

			if(t->count>=5*4-1){
				t->effect = 0;
			}
		return;

		case(TE_CIRCLE2):
		return;

		case(TE_DELETE):
			t->x += (t->vx>>1);
			t->y += (t->vy>>1);
			//t->d+=4;
			if(t->count>=47){
				t->flag = TF_DELETE;
			}
		return;
	}
}

static void Indsort(WORD *ind,WORD *now)
{
	WORD i,temp,next;

	// かなり複雑なのでバグに注意！ //
	for(i=temp=next=0;i<(*now);i++){

		// 消去要請フラグが立っている->swap 立っていない-> counter++ //
		if(((Tama+(*(ind+i)))->flag) & TF_DELETE){

			// フラグの立っていない弾を検索する //
			// temp = 1 + i;  // ((temp>i) ? temp : i);
			for(temp=i+1;temp<(*now);temp++){
				if(!(((Tama+(*(ind+temp)))->flag) & TF_DELETE)){
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

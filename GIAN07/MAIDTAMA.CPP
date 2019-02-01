/*                                                                           */
/*   MaidTama.cpp   メイドさんなショットの処理                               */
/*                                                                           */
/*                                                                           */

#include "Gian.h"


///// [ひみつの関数] /////
static void Indsort(WORD *ind,WORD *now);
static void MTamaSet(void);

static void SetT_A0(void);		// めいどたまＴＹＰＥ－Ａ
static void SetT_A1(void);
static void SetT_A2(void);
static void SetT_A3(void);
static void SetT_A4(void);
static void SetT_A5(void);
static void SetT_A6(void);
static void SetT_A7(void);
static void SetT_A8(void);

static void SetT_B0(void);		// めいどたまＴＹＰＥ－Ｂ
static void SetT_B1(void);
static void SetT_B2(void);
static void SetT_B3(void);
static void SetT_B4(void);
static void SetT_B5(void);
static void SetT_B6(void);
static void SetT_B7(void);
static void SetT_B8(void);

static void SetT_C0(void);		// めいどたまＴＹＰＥ－Ｃ
static void SetT_C1(void);
static void SetT_C2(void);
static void SetT_C3(void);
static void SetT_C4(void);
static void SetT_C5(void);
static void SetT_C6(void);
static void SetT_C7(void);
static void SetT_C8(void);

static void SetT_D0(void);		// めいどたまＴＹＰＥ－Ｄ
static void SetT_D1(void);
static void SetT_D2(void);
static void SetT_D3(void);
static void SetT_D4(void);
static void SetT_D5(void);
static void SetT_D6(void);
static void SetT_D7(void);
static void SetT_D8(void);

static void SetWideBomb(void);
static void SetHomingBomb(void);
static void SetLaserBomb(void);
static void SetCactusBomb(void);



TAMA_DATA	MaidTama[MAIDTAMA_MAX];			// 自機ショットの格納用構造体
WORD		MaidTamaInd[MAIDTAMA_MAX];		// 弾の順番を維持するための配列(TAMA.CPP互換)
WORD		MaidTamaNow;					// 現在の数

BYTE TogeDamage[4*2+2] =
{
	// MainWeapon		// SubWeapon
	TDM_WIDE_MAIN,		TDM_WIDE_SUB,		// TYPE_A(WIDE)
	TDM_HOMING_MAIN,	TDM_HOMING_SUB,		// TYPE_B(HOMING)
	TDM_LASER_MAIN,		TDM_LASER_SUB,		// TYPE_C
	1,					1					// ホーミングボム用
};

static void (*MaidTamaFunc[4][9])(void) =
{
	{SetT_A0,SetT_A1,SetT_A2,SetT_A3,SetT_A4,SetT_A5,SetT_A6,SetT_A7,SetT_A8},
	{SetT_B0,SetT_B1,SetT_B2,SetT_B3,SetT_B4,SetT_B5,SetT_B6,SetT_B7,SetT_B8},
	{SetT_C0,SetT_C1,SetT_C2,SetT_C3,SetT_C4,SetT_C5,SetT_C6,SetT_C7,SetT_C8},
	{SetT_D0,SetT_D1,SetT_D2,SetT_D3,SetT_D4,SetT_D5,SetT_D6,SetT_D7,SetT_D8}
};

static void (*MaidBombFunc[4])(void) =
{SetWideBomb, SetHomingBomb, SetLaserBomb, SetCactusBomb};

#define WIDE_BOMB_TIME			(60*4)
#define HOMING_BOMB_TIME		(60*3)
#define LASER_BOMB_TIME			(60*2)
#define CACTUS_BOMB_TIME		(0)

static BYTE MaidBombTime[4] =
{WIDE_BOMB_TIME, HOMING_BOMB_TIME, LASER_BOMB_TIME, CACTUS_BOMB_TIME};


#define MAID_TAMA_START		18	//12
#define MAID_MAIN_SHOT		6	//4
#define MAID_SUB_SHOT		9	//6



// たま発射！！ //
FVOID MaidTamaSet(void)
{
	// この関数では、前回の発射状態 (Viv_St) を参照して、発射可能であるならば //
	// 発射し、そうでなければ、単にリターンする。                             //
	// なお、弾のセットには TAMA.cpp 内の関数と互換のものを使用する           //

	if((Key_Data&KEY_TAMA) && Viv.toge_time==0 && Viv.muteki<MAID_MOVE_DISABLE_TIME){
		Viv.toge_time = MAID_TAMA_START;
	}

	// ボムの発動条件を満たしていれば、発動! //
	if((Key_Data&KEY_BOMB) && Viv.bomb_time==0 /*&& Viv.muteki==0*/
		&& Viv.bomb && SclInfo.MsgFlag==FALSE)
	{
		//if(Viv.weapon == 0) EnterBombPalette();

		Viv.bomb_time = MaidBombTime[Viv.weapon&3];		// 装備ごとに変更せよ
		Viv.muteki    = BOMBMUTEKI_VAL;
		Viv.bomb--;
		PlayRankAdd(-500);					// 難易度ダウン
	}

	if(Viv.bomb_time){
		Viv.bomb_time--;
		MaidBombFunc[Viv.weapon]();

		//if(Viv.bomb_time == 0 && Viv.weapon == 0)
		//	LeaveBombPalette();
	}

	if(Viv.toge_time){
		MaidTamaFunc[Viv.weapon&3][(Viv.exp+1)>>5]();
		Viv.toge_time--;
	}

	// レーザーを装備している場合 //
	if(Viv.weapon == 2  &&  Viv.lay_time){
		Viv.lay_time--;
		if(Viv.lay_time<64)				Viv.lay_grp = 0;
		else if(Viv.lay_time<64+50)		Viv.lay_grp = 1;
		else if(Viv.lay_time<64+100)	Viv.lay_grp = 2;
		else if(Viv.lay_time<64+150)	Viv.lay_grp = 3;
		else							Viv.lay_grp = 4;
		//Viv.lay_grp = (Viv.lay_time+63)>>6;
	}
}

// 弾移動＆ヒットチェック //
FVOID MaidTamaMove(void)
{
	// この関数では、TAMA.cpp の敵弾の移動処理を使用する。もちろん、         //
	// 当たり判定については、敵に対してのものとする事！！                    //
	// 当たり判定は、この弾の座標を与えることで ENEMY.cpp 内の関数が判別して //
	// 敵に当たっているかをチェックするものとする。                          //

	int				i;
	TAMA_DATA		*t;

	for(i=0;i<MaidTamaNow;i++){
		t = MaidTama + *(MaidTamaInd+i);
		if(t->c == TID_HOMING_BOMB_B){
			enemy_damage(t->x,t->y,TogeDamage[t->c]);
			t->count++;
			if(t->count>=19) t->flag = TF_DELETE;
			continue;
		}
		if(t->effect==TE_NONE){
			tamaTmove(t);
			tamaOmove(t);
			t->count++;
			if(((t->flag&TF_CLIP)==0)&&((t->x)<GX_MIN||(t->x)>GX_MAX||(t->y)<GY_MIN||(t->y)>GY_MAX))
				t->flag = TF_DELETE;

			if(enemy_damage(t->x,t->y,TogeDamage[t->c])){
				if(t->c == TID_HOMING_BOMB_A){
					TamaSTDForm(TID_HOMING_BOMB_B);
					TamaCmd.type = T_SBHBOMB;
					TamaSetXY(t->x,t->y);
					TamaSetDeg(-64,16);
					TamaSetSpd(10,0);
					TamaSetNum(1,0);
					MTamaSet();
				}
				t->flag = TF_DELETE;
				fragment_set(t->x,t->y,FRG_HIT);
			}
		}
		else
			tamaEmove(t);
	}
	Indsort(MaidTamaInd,&MaidTamaNow);


	// レーザーの当たり判定 //
	if(Viv.weapon == 2  &&  Viv.lay_grp){
		//x = (Viv.opx>>6)+4 -8 + SBOPT_DX;
		//y = (Viv.opy>>6)-20;
		enemy_damage2(Viv.opx+(SBOPT_DX<<6),Viv.opy,Viv.lay_grp/3+1);
		enemy_damage2(Viv.opx-(SBOPT_DX<<6),Viv.opy,Viv.lay_grp/3+1);
	}
}

// ナニな弾描画 //
FVOID MaidTamaDraw(void)
{
	// ここでは、さすがにTAMA.cpp 内の関数を使用するわけにはいかないので、 //
	// 独自に描画ルーチンを展開する。                                      //

	int				i,x,y;
	TAMA_DATA		*t;
	RECT			src,ltemp;
	static RECT		HomingBomb[5] = {
		{520,104,520+8 ,104+8},
		{528,104,528+16,104+16},
		{544,104,544+24,104+24},
		{568,104,568+32,104+32},
		{600,104,600+40,104+40}
	};

	for(i=0;i<MaidTamaNow;i++){
		t = MaidTama + *(MaidTamaInd+i);

		x = (t->x >> 6)-8;	// -8 は座標の補正用です
		y = (t->y >> 6)-8;	// 上に同じ

		// 弾の種類により、描画指定用矩形をセットする //
		switch(t->c){
			case(TID_WIDE_MAIN):	BltSetRect(&src,384+((t->d+8)&0xf0),176,16,16);	break;
			case(TID_WIDE_SUB):		BltSetRect(&src,384+((t->d+8)&0xf0),192,16,16);	break;
			case(TID_HOMING_MAIN):	BltSetRect(&src,384+((t->d+8)&0xf0),208,16,16);	break;
			case(TID_HOMING_SUB):	BltSetRect(&src,384+((t->d+8)&0xf0),224,16,16);	break;
			case(TID_HOMING_BOMB_A):BltSetRect(&src,384+((t->d+8)&0xf0),288,16,16);	break;
			case(TID_LASER_SUB):	BltSetRect(&src,384+((t->d+8)&0xf0),256,16,16);	break;

			case(TID_HOMING_BOMB_B):
				src = HomingBomb[(t->count/4)%5];
			break;
		}

		// 完全判定付きクリッピング //
		GrpBlt(&src,x,y,GrTama);
	}

	// レーザーの描画 //
	if(Viv.weapon == 2  &&  Viv.lay_grp){
		BltSetRect(&ltemp,384+((Viv.lay_grp-1)<<4),240,8,16);

		x = (Viv.opx>>6)+4 -8 + SBOPT_DX;
		y = (Viv.opy>>6)-20;
		src = ltemp;GrpBlt(&src,x,y,GrTama);

		x = (Viv.opx>>6)+4 -8 - SBOPT_DX;
		y = (Viv.opy>>6)-20;
		src = ltemp;GrpBlt(&src,x,y,GrTama);

		BltSetRect(&ltemp,384+8+((Viv.lay_grp-1)<<4),240,8,16);
		for(i=(Viv.opy>>6)-36;i>-16;i-=16){
			x = (Viv.opx>>6)+4 -8 + SBOPT_DX;
			y = i;
			src = ltemp;
			GrpBlt(&src,x,y,GrTama);
		}
		for(i=(Viv.opy>>6)-36;i>-16;i-=16){
			x = (Viv.opx>>6)+4 -8 - SBOPT_DX;
			y = i;
			src = ltemp;
			GrpBlt(&src,x,y,GrTama);
		}
	}
}

// 弾ハッシュテーブル初期化 //
FVOID MaidTamaIndSet(void)
{
	int		i;

	// この配列を初期化することで全ての弾を初期化する事になる //
	for(i=0;i<MAIDTAMA_MAX;i++){
		MaidTamaInd[i] = i;
		//memset(MaidTama+i,0,sizeof(TAMA_DATA));
	}

	// 現在の個数を０初期化するのを忘れずに //
	MaidTamaNow = 0;
}

static void MTamaSet(void)
{
	WORD i;
	TAMA_DATA *t;

	for(i=0;i<TamaCmd.n;i++){
		if(MaidTamaNow+1 >= MAIDTAMA_MAX) return;			// セットできない場合

		MaidTamaNow = MaidTamaNow + 1;						// 弾数をインクリメント
		t = MaidTama + (*(MaidTamaInd+MaidTamaNow-1));		// 弾ポインタをセット

		t->x = t->tx = TamaCmd.x;					// X座標のセット
		t->y = t->ty = TamaCmd.y;					// Y座標のセット

		t->v = t->v0 = tama_speed(i);			// 初速度のセット
		t->a = TamaCmd.a;						// 注意：サイズは char

		t->d   = tama_dir(i);					// 弾の発射角度
		t->d16 = (t->d<<8);						// 角速度のある運動で使用

		t->vx = cosl(t->d,t->v);				// 速度のＸ成分セット
		t->vy = sinl(t->d,t->v);				// 速度のＹ成分セット

		t->vd     = TamaCmd.vd;					// 角速度もしくはホーミング率
		t->c      = TamaCmd.c;					// 弾のＩＤ
		t->rep    = TamaCmd.rep;				// 繰り返し回数
		t->type   = TamaCmd.type;				// 弾の種類
		t->option = TamaCmd.option;				// 弾の属性(バイブ、反射等)
		t->effect = 0;//TamaCmd.cmd & 0xf0;			// 弾のエフェクト
		t->count  = 0;							// カウンタの初期化
		t->flag   = tama_flag();				// フラグの初期化
	}
}

static void Indsort(WORD *ind,WORD *now)
{
	WORD i,temp,next;

	for(i=temp=next=0;i<(*now);i++){

		// 消去要請フラグが立っている->swap 立っていない-> counter++ //
		if(((MaidTama+(*(ind+i)))->flag) & TF_DELETE){

			// フラグの立っていない弾を検索する //
			// temp = 1 + i;  // ((temp>i) ? temp : i);
			for(temp=i+1;temp<(*now);temp++){
				if(!(((MaidTama+(*(ind+temp)))->flag) & TF_DELETE)){
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

#define IsMainShot(t)	((t)==MAID_MAIN_SHOT||(t)==MAID_MAIN_SHOT*2||(t)==MAID_MAIN_SHOT*3)
#define IsSubShot(t)	( ((t)==0 || (t)==MAID_SUB_SHOT) && Viv.bomb_time==0)

IVOID MLaserSet(WORD time)
{
	if(Viv.bomb_time || Viv.muteki>MAID_MOVE_DISABLE_TIME){
		Viv.lay_time = 0;
		Viv.lay_grp  = 0;
		return;
	}

	if(Viv.lay_time == 0){
		Viv.lay_time = time;
		SndPlayEX(SOUND_ID_SBLASER,Viv.x,0);
	}
}

// ショットＴＹＰＥ－Ａ //
static void SetT_A0(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット単発のみ //
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
	}
}

static void SetT_A1(void)
{
	char dd;

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64+5,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-5,0);
		MTamaSet();
	}

	if(IsMainShot(Viv.toge_time)){
		// 軽く振り分けるメインショット //
		Viv.toge_ex += 32;
		dd = (char)sinl(Viv.toge_ex,6);
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
	}
}

static void SetT_A2(void)
{
	char dd;


	if(IsMainShot(Viv.toge_time)){
		// 中央にショット２連 //
		Viv.toge_ex += 32;
		dd = (char)sinl(Viv.toge_ex,6);

		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x-(6*64),Viv.y);
		TamaSetDeg(-64+dd,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
		TamaCmd.x += (12*64);
		MTamaSet();
	}

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		TamaSetDeg(-64+5,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-5,0);
		MTamaSet();
	}
}

static void SetT_A3(void)
{
	char dd;

	if(IsMainShot(Viv.toge_time)){
		// 中央にショット３ＷＡＹ //
		Viv.toge_ex+=32;
		dd = (char)sinl(Viv.toge_ex,6);
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,4);
		TamaSetSpd(54,0);
		TamaSetNum(3,0);
		MTamaSet();
	}

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64+5,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-5,0);
		MTamaSet();
	}
}

static void SetT_A4(void)
{
	char dd;

	if(IsMainShot(Viv.toge_time)){
		// 中央にショット３ＷＡＹ //
		Viv.toge_ex+=32;
		dd = (char)sinl(Viv.toge_ex,6);
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,4);
		TamaSetSpd(54,0);
		TamaSetNum(3,0);
		MTamaSet();
	}

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64+8,7);//(-64+5,7);
		TamaSetSpd(54,0);
		TamaSetNum(2,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-8,7);//(-64-5,7);
		MTamaSet();
	}
}

static void SetT_A5(void)
{
	SetT_A4();
}

static void SetT_A6(void)
{
	char dd;

	if(IsMainShot(Viv.toge_time)){
		// 中央にショット４ＷＡＹ //
		Viv.toge_ex+=32;
		dd = (char)sinl(Viv.toge_ex,6);
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,3);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64+10,8);//-64+6,4);
		TamaSetSpd(54,0);
		TamaSetNum(3,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-10,8);//(-64-6,4);
		MTamaSet();
	}
}

static void SetT_A7(void)
{
	SetT_A6();
}

static void SetT_A8(void)
{
	char dd;

	if(IsMainShot(Viv.toge_time)){
		// 中央にショット４ＷＡＹ //
		Viv.toge_ex+=32;
		dd = (char)sinl(Viv.toge_ex,6);
		TamaSTDForm(TID_WIDE_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,3);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	if(IsSubShot(Viv.toge_time)){
		// オプションのショット(右) //
		TamaSTDForm(TID_WIDE_SUB);
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64+12,8);//(-64+7,4);
		TamaSetSpd(54,0);
		TamaSetNum(4,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(-64-12,8);//(-64-7,4);
		MTamaSet();
	}
}

// ショットＴＹＰＥ－Ｂ //
static void SetT_B0(void)
{
	char dd;

	if(IsMainShot(Viv.toge_time)){
		// 軽く振り分けるメインショット //
		Viv.toge_ex+=32;
		dd = (char)sinl(Viv.toge_ex,4);
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64+dd,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
	}
	//Viv.toge_time = 3;
}

static void SetT_B1(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット２連 //
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x-(6*64),Viv.y);
		TamaSetDeg(-64,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
		TamaCmd.x += (12*64);
		MTamaSet();
	}
	//Viv.toge_time = 4;
	//if((++Viv.toge_ex)&7) return;

	if(IsSubShot(Viv.toge_time)){
		// ホーミング弾 //
		// オプションのショット(右) //
		TamaSTDForm(TID_HOMING_SUB);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(28,4);
		TamaSetDeg(64-5,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(64+5,0);
		MTamaSet();
	}
}

static void SetT_B2(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット３ＷＡＹ //
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,7);
		TamaSetSpd(54,0);
		TamaSetNum(3,0);
		MTamaSet();
	}

	//Viv.toge_time = 4;
	//if((++Viv.toge_ex)&7) return;

	if(IsSubShot(Viv.toge_time)){
		// ホーミング弾 //
		// オプションのショット(右) //
		TamaSTDForm(TID_HOMING_SUB);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(28,4);
		TamaSetDeg(64-5,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(64+5,0);
		MTamaSet();
	}
}

static void SetT_B3(void)
{
	SetT_B2();
}

static void SetT_B4(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット５ＷＡＹ //
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,7);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	//Viv.toge_time = 4;
	//if((++Viv.toge_ex)&7) return;

	if(IsSubShot(Viv.toge_time)){
		// ホーミング弾 //
		// オプションのショット(右) //
		TamaSTDForm(TID_HOMING_SUB);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(28,4);
		TamaSetDeg(64-5,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(64+5,0);
		MTamaSet();
	}
}

static void SetT_B5(void)
{
	SetT_B4();
}

static void SetT_B6(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット５ＷＡＹ //
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,7);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	//Viv.toge_time = 4;
	//if((++Viv.toge_ex)&3) return;

	if(IsSubShot(Viv.toge_time)){
		// ホーミング弾 //
		// オプションのショット(右) //
		TamaSTDForm(TID_HOMING_SUB);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(28,4);
		TamaSetDeg(64-5,0);
		TamaSetNum(1,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(64+5,0);
		MTamaSet();
	}
}

static void SetT_B7(void)
{
	SetT_B6();
}

static void SetT_B8(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット５ＷＡＹ //
		TamaSTDForm(TID_HOMING_MAIN);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,7);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	//Viv.toge_time = 4;
	//if((++Viv.toge_ex)&3) return;

	if(IsSubShot(Viv.toge_time)){
		// ホーミング弾 //
		// オプションのショット(右) //
		TamaSTDForm(TID_HOMING_SUB);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.opx+SBOPT_DX*64,Viv.opy);
		TamaSetSpd(28,4);
		TamaSetDeg(64-22,30);
		TamaSetNum(2,0);
		MTamaSet();

		// オプションのショット(左) //
		TamaSetXY(Viv.opx-SBOPT_DX*64,Viv.opy);
		TamaSetDeg(64+22,30);
		MTamaSet();
	}
}

// ショットＴＹＰＥ－Ｃ //
static void SetT_C0(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット単発のみ //
		TamaSTDForm(TID_LASER_SUB);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
	}
}

static void SetT_C1(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央に２列ショット //
		TamaSTDForm(TID_LASER_SUB);
		TamaSetXY(Viv.x-(6*64),Viv.y);
		TamaSetDeg(-64,0);
		TamaSetSpd(54,0);
		TamaSetNum(1,0);
		MTamaSet();
		TamaCmd.x += (12*64);
		MTamaSet();
	}

	MLaserSet(64+50);
}

static void SetT_C2(void)
{
	SetT_C1();
}

static void SetT_C3(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット３ＷＡＹ //
		TamaSTDForm(TID_LASER_SUB);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,6);
		TamaSetSpd(54,0);
		TamaSetNum(3,0);
		MTamaSet();
	}

	MLaserSet(64+100);
}

static void SetT_C4(void)
{
	SetT_C3();
}

static void SetT_C5(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット４ＷＡＹ(中央は２列で) //
		TamaSTDForm(TID_LASER_SUB);
		TamaSetSpd(54,0);

		TamaSetDeg(-64-5,10);
		TamaSetXY(Viv.x-(6*64),Viv.y);
		TamaSetNum(2,0);
		MTamaSet();

		TamaSetDeg(-64+5,10);
		TamaCmd.x += (12*64);
		MTamaSet();
	}

	MLaserSet(64+150);
}

static void SetT_C6(void)
{
	SetT_C5();
}

static void SetT_C7(void)
{
	SetT_C5();
}

static void SetT_C8(void)
{
	if(IsMainShot(Viv.toge_time)){
		// 中央にショット５ＷＡＹ //
		TamaSTDForm(TID_LASER_SUB);
		TamaSetXY(Viv.x,Viv.y);
		TamaSetDeg(-64,6);
		TamaSetSpd(54,0);
		TamaSetNum(5,0);
		MTamaSet();
	}

	MLaserSet(64+200);
}

// ショットＴＹＰＥ－Ｄ //
static void SetT_D0(void)
{
}

static void SetT_D1(void)
{
}

static void SetT_D2(void)
{
}

static void SetT_D3(void)
{
}

static void SetT_D4(void)
{
}

static void SetT_D5(void)
{
}

static void SetT_D6(void)
{
}

static void SetT_D7(void)
{
}

static void SetT_D8(void)
{
}



static void SetWideBomb(void)
{
	int		dx,dy,l;
	BYTE	d;

	if(Viv.bomb_time > WIDE_BOMB_TIME-30) return;

	d  = (BYTE)(Viv.bomb_time*3);
	l  = (WIDE_BOMB_TIME - Viv.bomb_time)*26;		// 16-32
	dx = GX_MID + 64*70/2 + cosl(d, l<<1);
	dy = GY_MID - 64*90/2 + sinl(d<<1, l);

	fragment_set(dx, dy, FRG_STAR1);
	fragment_set(dx, dy, FRG_STAR1);
	fragment_set(dx, dy, FRG_STAR2);

	enemy_damage4(1);
}

static void SetHomingBomb(void)
{
	if(Viv.bomb_time%30 == 1){
		TamaSTDForm(TID_HOMING_BOMB_A);
		TamaCmd.type = T_SBHOMING;
		TamaCmd.rep  = 64;
		TamaCmd.vd   = 5;
		TamaSetXY(Viv.x,Viv.y);
		TamaSetSpd(28,4);
		TamaSetDeg(64,16);
		TamaSetNum(8,1);
		MTamaSet();

		// 欠陥があるので、廃止 //
		//ObjectLockOn(&HomingX, &HomingY, 32*64, 32*64);
	}
}

// こいつは、Set というよりも、 HitCheck 的な役割を果たす //
static void SetLaserBomb(void)
{
	BYTE		LaserDeg;
	BYTE		d;
	int			ox,oy;
	int			i;

	LaserDeg = GetLaserDeg();

	ox = Viv.opx + (SBOPT_DX*64);
	oy = Viv.opy;
	for(i=-3;i<=3;i++){
		d = GetRightLaserDeg(LaserDeg, i);
		enemy_damage3(ox, oy, d);
	}

	ox = Viv.opx - (SBOPT_DX*64);
	oy = Viv.opy;
	for(i=-3;i<=3;i++){
		d = GetLeftLaserDeg(LaserDeg, i);
		enemy_damage3(ox, oy, d);
	}
}

static void SetCactusBomb(void)
{
}

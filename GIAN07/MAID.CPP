/*                                                                           */
/*   Maid.cpp   メイドさん関連の処理                                         */
/*                                                                           */
/*                                                                           */

#include "Maid.h"
#include "DD_GRP3D.h"


MAID			Viv;					// 麗しきメイドさん構造体
DWORD			EvadeRate[256];			// かすり得点レート


extern void WideBombDraw(void)
{
	//RECT	src = {0, 0, 382, 480};
	static RECT data[6] = {
		{0,   0, 210, 240}, {210,   0, 210*2, 240}, {210*2,   0, 210*3, 240},
		{0, 240, 210, 480}, {210, 240, 210*2, 480}, {210*2, 240, 210*3, 480}
	};

	RECT	src;
	int		x,y;
	int		t;

#define BX_MIN		(X_MIN+100)
#define BY_MIN		(Y_MIN+100)

	if(Viv.weapon!=0 || Viv.bomb_time==0) return;

	x = BX_MIN;
	y = BY_MIN;

	if(Viv.bomb_time > 80){
//		x = BX_MIN + (Viv.bomb_time-30*7) * 11;	if(x < BX_MIN) x = BX_MIN;
//		y = BY_MIN + (Viv.bomb_time-30*7) * 11;	if(y < BY_MIN) y = BY_MIN;
		t = ((60*4-Viv.bomb_time)/4);
		if(t < 0) t = 0;
		if(t > 5) t = 5;
	}
	else{
//		x = BX_MIN - (80-Viv.bomb_time) * 8;
//		y = BY_MIN - (80-Viv.bomb_time) * 8;
		t = (Viv.bomb_time/4); if(t > 5) t = 5;
	}

	src = data[t];
	GrpBlt(&src, x, y, GrBomber);
}

void LaserBombDraw(void)
{
	BYTE			d,c;
	POINT			p[4];
	int				i,w;
	int				lx,ly;
	int				wx,wy;
	BYTE			LaserDeg;

	LaserDeg = GetLaserDeg();

	GrpLock();
	if(LaserDeg<58){
		for(w=3;w>0;w--){
			for(i=-3;i<=3;i++){
				d = GetRightLaserDeg(LaserDeg, i); //64+48 - LaserDeg*3 + i*(64-LaserDeg)/2;
				lx = cosl(d,850);	ly = sinl(d,850);
				wx = cosl(d+64,w);	wy = sinl(d+64,w);
				p[0].x = (Viv.opx>>6) + SBOPT_DX +  0 + wx;
				p[0].y = (Viv.opy>>6)            +  0 + wy;
				p[3].x = (Viv.opx>>6) + SBOPT_DX +  0 - wx;
				p[3].y = (Viv.opy>>6)            +  0 - wy;
				p[2].x = (Viv.opx>>6) + SBOPT_DX + lx - wx;
				p[2].y = (Viv.opy>>6)            + ly - wy;
				p[1].x = (Viv.opx>>6) + SBOPT_DX + lx + wx;
				p[1].y = (Viv.opy>>6)            + ly + wy;
				if(DxObj.Bpp==16){
					GrpSetColor(0,0,5);
					GrpSetAlpha(0,ALPHA_ONE);
					GrpGrdRectA(p);
				}
				else{
					switch(w){
						case(1):	GrpSetColor(4,4,5);	break;
						case(2):	GrpSetColor(2,2,5);	break;
						case(3):	GrpSetColor(0,0,5);	break;
					}
					GrpPolygon(p,4);
				}
			}
			for(i=-3;i<=3;i++){
				d = GetLeftLaserDeg(LaserDeg, i); //64-48 + LaserDeg*3 + i*(64-LaserDeg)/2;
				lx = cosl(d,850);	ly = sinl(d,850);
				wx = cosl(d+64,w);	wy = sinl(d+64,w);
				p[0].x = (Viv.opx>>6) - SBOPT_DX +  0 + wx;
				p[0].y = (Viv.opy>>6)            +  0 + wy;
				p[3].x = (Viv.opx>>6) - SBOPT_DX +  0 - wx;
				p[3].y = (Viv.opy>>6)            +  0 - wy;
				p[2].x = (Viv.opx>>6) - SBOPT_DX + lx - wx;
				p[2].y = (Viv.opy>>6)            + ly - wy;
				p[1].x = (Viv.opx>>6) - SBOPT_DX + lx + wx;
				p[1].y = (Viv.opy>>6)            + ly + wy;
				if(DxObj.Bpp==16){
					GrpSetColor(0,0,5);
					GrpSetAlpha(0,ALPHA_ONE);
					GrpGrdRectA(p);
				}
				else{
					switch(w){
						case(1):	GrpSetColor(4,4,5);	break;
						case(2):	GrpSetColor(2,2,5);	break;
						case(3):	GrpSetColor(0,0,5);	break;
					}
					GrpPolygon(p,4);
				}
			}
			if(DxObj.Bpp==16) break;
		}
	}
	else if(LaserDeg<150){
		c = 0;
		for(w=12-(LaserDeg-64)/8;w>0;w-=2,c++){
			for(i=-3;i<=3;i++){
				d = GetRightLaserDeg(LaserDeg, i);// 64+48 - 58*3 + i*(64-min(62,LaserDeg))/2;
				lx = cosl(d,850);	ly = sinl(d,850);
				wx = cosl(d+64,w);	wy = sinl(d+64,w);
				p[0].x = (Viv.opx>>6) + SBOPT_DX +  0 + wx;
				p[0].y = (Viv.opy>>6)            +  0 + wy;
				p[3].x = (Viv.opx>>6) + SBOPT_DX +  0 - wx;
				p[3].y = (Viv.opy>>6)            +  0 - wy;
				p[2].x = (Viv.opx>>6) + SBOPT_DX + lx - wx;
				p[2].y = (Viv.opy>>6)            + ly - wy;
				p[1].x = (Viv.opx>>6) + SBOPT_DX + lx + wx;
				p[1].y = (Viv.opy>>6)            + ly + wy;
				if(DxObj.Bpp==16){
					GrpSetColor(0,0,5);
					GrpSetAlpha(0,ALPHA_ONE);
					GrpGrdRectA(p);
				}
				else{
					GrpSetColor(c,c,5);
					GrpPolygon(p,4);
				}
			}
			for(i=-3;i<=3;i++){
				d = GetLeftLaserDeg(LaserDeg, i); //64-48 + 58*3 + i*(64-min(62,LaserDeg))/2;
				lx = cosl(d,850);	ly = sinl(d,850);
				wx = cosl(d+64,w);	wy = sinl(d+64,w);
				p[0].x = (Viv.opx>>6) - SBOPT_DX +  0 + wx;
				p[0].y = (Viv.opy>>6)            +  0 + wy;
				p[3].x = (Viv.opx>>6) - SBOPT_DX +  0 - wx;
				p[3].y = (Viv.opy>>6)            +  0 - wy;
				p[2].x = (Viv.opx>>6) - SBOPT_DX + lx - wx;
				p[2].y = (Viv.opy>>6)            + ly - wy;
				p[1].x = (Viv.opx>>6) - SBOPT_DX + lx + wx;
				p[1].y = (Viv.opy>>6)            + ly + wy;
				if(DxObj.Bpp==16){
					GrpSetColor(0,0,5);
					GrpSetAlpha(0,ALPHA_ONE);
					GrpGrdRectA(p);
				}
				else{
					GrpSetColor(c,c,5);
					GrpPolygon(p,4);
				}
			}
			if(DxObj.Bpp==16) break;
		}
	}

	GrpUnlock();
}

extern void MaidDraw(void)
{
	static RECT VivBit[4][2] = {
		{{480,128,480+24,128+24},{504,128,504+24,128+24}},	// ワイド
		{{480,152,480+24,152+24},{504,152,504+24,152+24}},	// ホーミング
		{{528,152,528+24,152+24},{552,152,552+24,152+24}},	// レーザー
		{{480,152,480+24,152+24},{504,152,504+24,152+24}},	// 仮
	};

	static BYTE draw_flag  = 0;
	static BYTE draw_flag2 = 0;

	int		x  = (Viv.x>>6)-16;
	int		y  = (Viv.y>>6)-24;
	int		ox = (Viv.opx>>6) - 12;
	int		oy = (Viv.opy>>6) - 12;
	RECT	src;

	draw_flag = 1 - draw_flag;
	draw_flag2++;

	if(Viv.muteki == VIVDEAD_VAL) draw_flag = 0;

	if(Viv.muteki==0 || draw_flag){
		BltSetRect(&src,384+Viv.GrpID*32,128,16*2,16*3);
		GrpBlt(&src,x,y,GrTama);
	}

	if( ((Viv.exp+1)>>5) ){
		if(Viv.muteki < VIVDEAD_VAL){
			src = VivBit[Viv.weapon&3][(draw_flag2>>2)&1];
			GrpBlt(&src,ox+SBOPT_DX,oy,GrTama);
			src = VivBit[Viv.weapon&3][(draw_flag2>>2)&1];
			GrpBlt(&src,ox-SBOPT_DX,oy,GrTama);
		}
	}

	if(Viv.bomb_time && Viv.weapon==2)
		LaserBombDraw();
}

// 各種ステータスを描画する //
extern void StateDraw(void)
{
	int		i,temp;
	//RECT	src = {0,80,128,80+16};
	RECT	src = {0,80,128,80+24};
	char	buf[100];

	// 残りかすり時間ゲージの表示 //
	if(Viv.evade_c){
		GrpLock();
		GrpSetColor(5,1,0);
		GrpSetAlpha(128,ALPHA_ONE);
		for(i=0;i<=10;i++){
			temp = 128+9+(Viv.evade_c>>2) + (5-i);
			if(temp>128+8)
				GrpBoxA(128+8,16+3+i,temp,16+3+i+1);
				//GrpBoxA(128+8,16+3,128+9+(Viv.evade_c>>2),16+3+10);
		}
		GrpUnlock();
	}
	GrpBlt(&src,128,16,GrTama);
	sprintf(buf,"%3d",Viv.evade);
	GrpPut57(128 + 95,16 + 91-80,buf);

	// 得点表示 //
	sprintf(buf,"%9d",Viv.score);
	GrpPut16c(128,0,buf);

	// 仮の残機＆残りボム表示 //
	sprintf(buf,"       Bomb %1d", Viv.bomb);
	GrpPut16c(280,0,buf);

	// 残りメイド数表示 //
	for(i=0; i<Viv.left; i++){
		BltSetRect(&src, 608, 432, 16, 16);
		GrpBlt(&src, 280+i*14, 0, GrTama);
	}
}

extern void MaidMove(void)
{
	int		vx,vy,v;
	int		VivSpeed = 64*18;
	char		buf[100];

	// かすり残り時間を減らす //
	if(Viv.evade_c){
		if(Viv.bomb_time && Viv.evade_c>=2) Viv.evade_c -= 2;
		else                                Viv.evade_c -= 1;

		if(Viv.evade_c==0){
			sprintf(buf,"%3d Evade  %7dPts",Viv.evade,Viv.evadesc);
			StringEffect(180,40,buf);
			score_add(Viv.evadesc);
			Viv.evade   = 0;
			Viv.evadesc = 0;
		}
	}

	// 無敵時間を減らす(ボム中は減らさない) //
	if(Viv.muteki && Viv.bomb_time==0) Viv.muteki--;

	// 得点変化処理 //
	if(Viv.dscore>=100000)		Viv.score+=100000	,Viv.dscore-=100000;
	else if(Viv.dscore>=20000)	Viv.score+=20000	,Viv.dscore-=20000;
	else if(Viv.dscore>=2000)	Viv.score+=2000		,Viv.dscore-=2000;
	else if(Viv.dscore>=200)	Viv.score+=200		,Viv.dscore-=200;
	else if(Viv.dscore>=20)		Viv.score+=20		,Viv.dscore-=20;
	else if(Viv.dscore>=10)		Viv.score+=10		,Viv.dscore-=10;

	// 押しっぱなし減速を有効にするのか //
	if(ConfigDat.InputFlags & INPF_Z_SPDDOWN_ENABLE){
		if(Key_Data & KEY_TAMA){
			if(Viv.ShiftCounter < 8) Viv.ShiftCounter++;
			else                     Key_Data = Key_Data | KEY_SHIFT;
		}
		else{
			Viv.ShiftCounter = 0;
		}
	}

	if(Viv.muteki < MAID_MOVE_DISABLE_TIME){
		vx = vy = 0;
		v = (Key_Data & KEY_SHIFT) ? (VivSpeed/3) : VivSpeed;
		if(Key_Data & KEY_UP)		vy-=v;
		if(Key_Data & KEY_DOWN)		vy+=v;
		if(Key_Data & KEY_LEFT)		vx-=v;
		if(Key_Data & KEY_RIGHT)	vx+=v;

		if(vx && vy){
			Viv.x += (vx/6);
			Viv.y += (vy/6);
		}
		else{
			Viv.x += (vx>>2);
			Viv.y += (vy>>2);
		}

		if(Viv.y<SY_MIN)		Viv.y = SY_MIN;
		else if(Viv.y>SY_MAX)	Viv.y = SY_MAX;

		if(Viv.x<SX_MIN)		Viv.x = SX_MIN;
		else if(Viv.x>SX_MAX)	Viv.x = SX_MAX;
	}
	else{
		vx = 0;
		vy = -(64+32);
		Viv.y += vy;
	}

	if(vx>0)		Viv.GrpID = 2;
	else if(vx<0)	Viv.GrpID = 0;
	else            Viv.GrpID = 1;

	Viv.opx = Viv.x;
	Viv.opy = Viv.y;

	// オプションの処理 //
	if(Viv.vx<0) Viv.vx+=64;
	if(Viv.vx>0) Viv.vx-=64;
	if(Viv.vy<0) Viv.vy+=64;
	if(Viv.vy>0) Viv.vy-=64;

	if(vx<0&&Viv.vx< 6*64)	Viv.vx+=2*64;
	if(vx>0&&Viv.vx>-6*64)	Viv.vx-=2*64;
	if(vy<0&&Viv.vy< 10*64)	Viv.vy+=2*64;
	if(vy>0&&Viv.vy>-10*64)	Viv.vy-=2*64;

	Viv.opx = Viv.x + Viv.vx;
	Viv.opy = Viv.y + Viv.vy + 64*6;

	// 弾＆ボムのセット //
	MaidTamaSet();

	if(Viv.bomb_time){
		tama_clear();
		laser_clear();
		//if(Viv.bomb_time%60==0) fragment_set(Viv.x,Viv.y,FRG_FATCIRCLE);
	}

	Viv.BuzzSound = FALSE;
}

// 初期化 //
extern void MaidSet(void)
{
	int i;
	int sct[4] = {5,10,15,30};

	MaidNextStage();

	Viv.score     = 0;
	Viv.dscore    = 0;
	Viv.exp       = 0;
	Viv.exp2      = 0;
	Viv.bomb      = ConfigDat.BombStock;
	Viv.left      = ConfigDat.PlayerStock;
	Viv.credit    = 4;		//5;

	Viv.bomb_time = 0;
	Viv.evade_c   = Viv.evade = 0;
	Viv.evadesc   = 0;
	Viv.evade_sum = 0;

	Viv.GrpID     = 1;

	Viv.muteki    = VIVDEAD_VAL;

	Viv.bGameOver = FALSE;

	Viv.toge_ex   = 0;
	Viv.toge_time = 0;
	Viv.lay_time  = 0;
	Viv.lay_grp   = 0;
	Viv.ShiftCounter = 0;

	Viv.BuzzSound = FALSE;

	for(i=0;i<256;i++)
		EvadeRate[i] = i*sct[ConfigDat.GameLevel];
}

// 次のステージの準備 //
extern void MaidNextStage(void)
{
	Viv.x         = Viv.opx = SX_START;
	Viv.y         = Viv.opx = SY_START;
	Viv.vx        = 0;
	Viv.vy        = 0;

	Viv.toge_ex   = 0;
	Viv.toge_time = 0;
	Viv.lay_time  = 0;
	Viv.lay_grp   = 0;

	Viv.muteki    = VIVDEAD_VAL;
	Viv.bomb_time = 0;
	Viv.ShiftCounter = 0;

	Viv.BuzzSound = FALSE;
}

extern void MaidDead(void)
{
	int		i;

#ifdef PBG_DEBUG
	if(!DebugDat.Hit) return;
#endif

	fragment_set(Viv.x,Viv.y,FRG_FATCIRCLE);

	for(i=0; i<50; i++)
		fragment_set(Viv.x,Viv.y,FRG_HEART);

	SndPlay(SOUND_ID_DEAD);

	// 座標系セット //
	Viv.x         = Viv.opx = SX_START;
	Viv.y         = Viv.opx = SY_START;
	Viv.vx        = 0;
	Viv.vy        = 0;

	// かすり系リセット //
//	Viv.evade_c = 0;
//	Viv.evade   = 0;
//	Viv.evadesc = 0;

	Viv.lay_time  = 0;
	Viv.lay_grp   = 0;

	Viv.bomb = ConfigDat.BombStock;
	Viv.muteki = VIVDEAD_VAL;

	PlayRankAdd(-2560);

	if(Viv.left){
		// 残機の残っている場合 //
		Viv.left -= 1;
	}
	else{
		// 残機の残っていない場合 //
		Viv.bGameOver = TRUE;
		Viv.score    += Viv.dscore;			// 得点吐き出し

		// かすり系リセット //
		Viv.evade_c = 0;
		Viv.evade   = 0;
		Viv.evadesc = 0;

		GameOverInit();						// ゲームオーバーへと
	}

	tama_clear();
	laser_clear();
}

// かすりゲージを上昇させる(エフェクトはメイド中心) //
extern FVOID evade_add(BYTE n)
{
	evade_addEx(Viv.x,Viv.y,n);
}

// 指定座標からエフェクト発生＆ゲージ上昇 //
extern FVOID evade_addEx(int x,int y,BYTE n)
{
	int		i;

	PlayRankAdd(n<<2);

	if(n){
		if(Viv.BuzzSound == FALSE){
			SndPlayEX(SOUND_ID_BUZZ,x,0);
			Viv.BuzzSound = TRUE;
		}
		fragment_set(x,y,FRG_EVADE);
		fragment_set(x,y,FRG_EVADE);
		fragment_set(x,y,FRG_EVADE);
	}

	for(i=0;i<n;i++){
		if(Viv.evade==999){
			Viv.evade_c = 1;
			return;
		}
		Viv.evade   += 1;
		Viv.evade_sum += 1;
		Viv.evadesc += Viv.evade*20;
	}

	if(Viv.evade)
		Viv.evade_c  = EVADETIME_MAX;
}

// スコアを上昇させる //
extern void score_add(int sc)
{
	Viv.dscore += sc;
}

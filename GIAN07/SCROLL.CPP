/*                                                                           */
/*   SCROLL.cpp   スクロール処理                                             */
/*                                                                           */
/*                                                                           */

#include "SCROLL.h"



SCROLL_INFO		ScrollInfo;			// スクロールに関する情報
SCL_INFO		SclInfo;			// ＳＣＬに関する情報
RECT			rcMapChip[1200];	// マップパーツＩＤに対する矩形

static void enemy_set(void);			// 敵をセットする
static void _PutEnemy(BYTE *p);			// p:SCL_ENEMY以降の敵配置データ
static void InitMapChipRect(void);		// スクロールに関する情報の初期化を行う

static PBGMAP *ScNextLine(PBGMAP *p);		// 次の行にＧＯ！！
static PBGMAP *ScBeforeLine(PBGMAP *p);		// 前の行にＧＯ！！

static FVOID ScrollCmdDummy(void);			// 特殊スクロール無し
static FVOID ScrollCmdStg2Boss(void);		// ２面ボスのスクロール
static FVOID ScrollCmdRasterOpen(void);		// ラスタースクロールオープン
static FVOID ScrollCmdRasterClose(void);	// ラスタースクロールクローズ
static FVOID ScrollCmdStg3Boss(void);		// ゲイツ雲

static FVOID ScrollCmdStg6Cube(void);		// ６面の３Ｄキューう゛
static FVOID ScrollCmdStg6RndEcl(void);		// ６面のランダムＥＣＬ列
static FVOID ScrollCmdStg6Raster(void);		// ６面ラスター

static FVOID ScrollCmdStg3Star(void);		// ３面高速星

static FVOID Stg3BossMapDraw(void);			// ゲイツ雲描画

static FVOID ScrollCmdStg4Rock(void);				// ４面岩



// デバッグ用マクロ //
#ifdef _DEBUG
	#define SCL_DEBUG(s)					\
	{										\
		GrpPut16(10,10+16*11,s);			\
	}
#else
	#define SCL_DEBUG(s)
#endif



// 背景を動かす(１フレーム分) //
FVOID ScrollMove(void)
{
	int			i;

	enemy_set();			// 敵をセット
	ScrollInfo.ExCmd();		// 特殊スクロール発動!!

	// 振動エフェクトを動作させる(これは、特殊スクロールとは別物) //
	if(ScrollInfo.IsQuake) ScrollInfo.IsQuake+=2;

	// 標準のスクロールスピードだけカウンタを進める //
	if(ScrollInfo.DataHead==NULL) return;
	if(ScrollInfo.Count>=ScrollInfo.InfEnd) return;

	// スクロールしない場合は、リターンする //
	if(ScrollInfo.ScrollSpeed==0) return;

	ScrollInfo.Count += ScrollInfo.ScrollSpeed;

	if(ScrollInfo.ScrollSpeed>0){
		// 通常のスクロール //
		for(i=0;i<ScrollInfo.NumLayer;i++){
			ScrollInfo.LayerCount[i]+= ScrollInfo.ScrollSpeed;
			while(ScrollInfo.LayerCount[i]>=ScrollInfo.LayerWait[i]){
				ScrollInfo.LayerCount[i] -= ScrollInfo.LayerWait[i];
				ScrollInfo.LayerDy[i]  = (ScrollInfo.LayerDy[i]+1)%16;//& 0x0f;
				if(ScrollInfo.LayerDy[i]==0)
					ScrollInfo.LayerPtr[i] = ScNextLine(ScrollInfo.LayerPtr[i]);
			}
		}
	}
	else{
		// 逆方向のスクロール //
		for(i=0;i<ScrollInfo.NumLayer;i++){
			ScrollInfo.LayerCount[i]+= ScrollInfo.ScrollSpeed;
			while(ScrollInfo.LayerCount[i]<0){
				if(ScrollInfo.LayerDy[i]==0)
					ScrollInfo.LayerPtr[i] = ScBeforeLine(ScrollInfo.LayerPtr[i]);
				ScrollInfo.LayerCount[i] += ScrollInfo.LayerWait[i];
				ScrollInfo.LayerDy[i]  = (ScrollInfo.LayerDy[i]+15)%16;//& 0x0f;
			}
		}
	}
}

static PBGMAP *ScNextLine(PBGMAP *p)
{
	int		i;

	for(i=0;i<MAP_WIDTH;){
		if(*p != MAPDATA_NONE) p++,i++;
		else{
			i = i + (*(p+1));
			p = p + 2;
		}
	}

	return p;
}

static PBGMAP *ScBeforeLine(PBGMAP *p)
{
	int		i;

	for(i=0;i<MAP_WIDTH;){
		if(*(p-2) != MAPDATA_NONE) p--,i++;
		else{
			i = i + (*(p-1));
			p = p - 2;
		}
	}

	return p;
}

// p:SCL_ENEMY以降の敵配置データ //
static void enemy_set(void)
{
	RECT		MWinRect;
	int			x,y;
	BYTE		id;
	BOOL		bFlag = TRUE;
	char		*mtitle;
	BYTE		*cmd;
	DWORD		temp;
//	BIT_DEVICE	*in;				// やばやば...
	char		buf[100];
	BOOL		CtrlFlag = FALSE;

	// ウィンドウの表示位置を設定する //
	if(ConfigDat.GraphFlags&GRPF_WINDOW_UPPER)	SetRect(&MWinRect,128,16,640-128,96);
	else										SetRect(&MWinRect,128,400,640-128,480);

	while(bFlag){
		cmd = SCL_Now;
		switch(cmd[0]){
			case(SCL_KEY):		// キー入力待ち
			break;

			case(SCL_TIME):
				temp = (*(DWORD *)(&cmd[1]));
				// メッセージウィンドウがオープンしている場合 //
				if(SclInfo.MsgFlag){
					if(DemoplaySaveEnable||DemoplayLoadEnable)       CtrlFlag = TRUE;
					else if(ConfigDat.GraphFlags & GRPF_MSG_DISABLE) CtrlFlag = TRUE;
					else if(GetAsyncKeyState(VK_CONTROL)&0x8000)     CtrlFlag = TRUE;

					if(CtrlFlag) GameCount += (temp-GameCount)/3;
					else if((Key_Data&KEY_RETURN)
						||((Key_Data&KEY_TAMA)&&(ConfigDat.InputFlags&INPF_Z_MSKIP_ENABLE))){
						if(!SclInfo.ReturnFlag){
							GameCount = temp;
							SclInfo.ReturnFlag = TRUE;
						}
					}
					else SclInfo.ReturnFlag = FALSE;
				}
/*
				if((GetAsyncKeyState(VK_CONTROL)&0x8000)&&SclInfo.MsgFlag)
					GameCount+=(temp-GameCount)/3;
				else if((Key_Data&KEY_RETURN) && SclInfo.ReturnFlag==FALSE){
					GameCount  = temp;
					SclInfo.ReturnFlag = TRUE;
				}
				if(!(Key_Data&KEY_RETURN) && SclInfo.ReturnFlag) SclInfo.ReturnFlag = FALSE;
*/
				if(temp>GameCount) bFlag    = FALSE;
				else               SCL_Now += 5;		// cmd(1)+time(4)
				SCL_DEBUG("--- SCL_TIME ---");
			break;

			case(SCL_ENEMY):
				if(BossNow==0) _PutEnemy(cmd+1);		// ボス出現中は出て来ちゃダメ
				SCL_Now += 6;	// cmd(1)+x(2)+y(2)+id(1)
				SCL_DEBUG("--- SCL_ENEMY ---");
			break;

			case(SCL_BOSS):		// ボスをセットする(X(16),Y(16),ID(8))
				x  = (*(short *)(&cmd[1]));		// ボス初期Ｘ
				y  = (*(short *)(&cmd[1+2]));	// ボス初期Ｙ
				id = cmd[1+2+2];				// ボスＩＤ
				BossSet(x,y,id);
				SCL_Now += (1+2+2+1);	// cmd+x+y+id
			break;

			case(SCL_BOSSDEAD):		// ボスを強制的に破壊する(Level2 命令Only)
				BossKillAll();
				SCL_Now++;
			break;

			case(SCL_MWOPEN):	// メッセージウィンドウを開く
				if(!(DemoplaySaveEnable||DemoplayLoadEnable
					 ||(ConfigDat.GraphFlags&GRPF_MSG_DISABLE) )){
					MWinOpen(&MWinRect);
				}
				SclInfo.MsgFlag = TRUE;
				SCL_Now++;
			break;

			case(SCL_MWCLOSE):	// メッセージウィンドウを閉じる
				if(!(DemoplaySaveEnable||DemoplayLoadEnable
					 ||(ConfigDat.GraphFlags&GRPF_MSG_DISABLE) )){
					MWinClose();
				}
				SclInfo.MsgFlag = FALSE;
				SCL_Now++;
			break;

			case(SCL_MSG):		// メッセージを出力する
				//MWinCmd(MWCMD_SMALLFONT);
				MWinMsg((char *)cmd+1);
				SCL_Now += (strlen((char *)cmd+1)+2);
			break;

			case(SCL_FACE):		// 顔を表示する
				MWinFace(cmd[1]);
				SCL_Now += 2;
			break;

			case(SCL_LOADFACE):	// 顔グラをロードする(SurfaceID,FileNo)
				LoadFace(cmd[1],cmd[2]);
				SCL_Now += 3;
			break;

			case(SCL_NPG):		// 新しいページに変更する
				MWinCmd(MWCMD_NEWPAGE);
				SCL_Now++;
			break;

			case(SCL_END):		// カウントも変更させずにリターンする
				/*
				MWinOpen(&MWinRect);
				MWinCmd(MWCMD_NEWPAGE);
				MWinCmd(MWCMD_LARGEFONT);
				MWinMsg("ＳＣＬ完了ですの");
				SCL_DEBUG("--- SCL_END ---");
				*/
			return;

			case(SCL_SSP):		// スクロールスピード変更
				ScrollSpeed(*(short *)&cmd[1]);
				SCL_Now += 3;
			break;

			case(SCL_MUSIC):
//				if(!(/*DemoplaySaveEnable||*/DemoplayLoadEnable)){
				if(!IsDemoplay){
					Mid_Stop();
					if(LoadMusic(cmd[1])){
						Mid_Play();
						mtitle = Mid_GetTitle();
						if(mtitle) sprintf(buf,"♪ %.100s",mtitle);
						SetMusicTitle(460,buf);
					}
				}
				SCL_Now += 2;
			break;

			case(SCL_DELENEMY):
				enemyind_set();
				SCL_Now++;
			break;

			case(SCL_EFC):
				switch(cmd[1]){
				case(SEFC_WARN):
					//effect_set(0,0,EFC_WARNBOSS,GameStage);
					SndPlayEX(8,GX_MID,DSBPLAY_LOOPING);
					WarningEffectSet();
					//StringEffect3(GameStage);
				break;

				case(SEFC_WARNSTOP):	SndStop(8);						break;	// Warning 停止
				case(SEFC_MUSICFADE):	Mid_FadeOut(120);				break;	// 曲の停止
				case(SEFC_STG2BOSS):	ScrollCommand(SCMD_STG2BOSS);	break;	// ２面ボスScroll
				case(SEFC_RASTERON):	ScrollCommand(SCMD_RASTER_ON);	break;	// ラスターON
				case(SEFC_RASTEROFF):	ScrollCommand(SCMD_RASTER_OFF);	break;	// ラスターOFF
				case(SEFC_STG3BOSS):	ScrollCommand(SCMD_STG3BOSS);	break;	// ３面ボス雲
				case(SEFC_STG3RESET):	ScrollCommand(SCMD_STG3RESET);	break;	// ３面リセット
				case(SEFC_CFADEIN):		ScreenEffectSet(SCNEFC_CFADEIN);break;	// ○フェードIn
				case(SEFC_CFADEOUT):	ScreenEffectSet(SCNEFC_CFADEOUT);break;	// ○フェードOut
				case(SEFC_STG6CUBE):	ScrollCommand(SCMD_STG6CUBE);	break;	// ６面キューブ
				case(SEFC_STG6RNDECL):	ScrollCommand(SCMD_STG6RNDECL);	break;	// ６面ＥＣＬ羅列
				case(SEFC_STG4ROCK):	ScrollCommand(SCMD_STG4ROCK);	break;	// ４面岩
				case(SEFC_STG4LEAVE):	ScrollCommand(SCMD_STG4LEAVE);	break;	// ４面岩画面外へ
				case(SEFC_WHITEIN):		ScreenEffectSet(SCNEFC_WHITEIN);break;	// ホワイトイン
				case(SEFC_WHITEOUT):	ScreenEffectSet(SCNEFC_WHITEOUT);break;	// ホワイトアウト
				case(SEFC_LOADEX01):	LoadGraph(GRAPH_ID_EXBOSS1);	break;
				case(SEFC_LOADEX02):	LoadGraph(GRAPH_ID_EXBOSS2);	break;
				case(SEFC_STG6RASTER):	ScrollCommand(SCMD_STG6RASTER);	break;	// ６面ラスター
				}
				SCL_Now += 2;
			break;

			case(SCL_WAITEX):	// 特殊待ち <cmd1>,<opt4>
				switch(cmd[1]){
					case(SWAIT_BOSSHP):		// 残りＨＰ
						if(GetBossHPSum()<=(*(DWORD *)(&cmd[2]))) break;
					return;
					case(SWAIT_BOSSLEFT):	// 残りボス数
						if(BossNow<=(*(DWORD *)(&cmd[2]))) break;
					return;
				}
				SCL_Now += (1+1+4);
			break;

			case(SCL_STAGECLEAR):	// ステージクリア
				if(ConfigDat.StageSelect){
					ReplaySave();
					GameExit(TRUE);
					return;
				}
				if(DemoplayLoadEnable) return;
				// ステージクリア処理をここに記述 //
				GameNextStage();	// 本当はエラーチェックが必要!!
			return;

			case(SCL_GAMECLEAR):
				if(ConfigDat.StageSelect){
					ReplaySave();
					GameExit(TRUE);
					return;
				}
				if(DemoplayLoadEnable) return;

				if(GameStage == 6) GameStage = 7;
				if(ConfigDat.GameLevel != GAME_EASY){
					switch(Viv.weapon){
						case(0):	ConfigDat.ExtraStgFlags |= 1;	break;
						case(1):	ConfigDat.ExtraStgFlags |= 2;	break;
						case(2):	ConfigDat.ExtraStgFlags |= 4;	break;
					}
				}
				ConfigSave();
				EndingInit();
			return;

			case(SCL_EXTRACLEAR):
				if(ConfigDat.StageSelect){
					ReplaySave();	// 終了はしない
				}
				if(DemoplayLoadEnable) return;

				NameRegistInit(TRUE);
			return;

			case(SCL_MAPPALETTE):	// マップパーツ用Surface からパレットを
				LoadPaletteFromMap();	// BitDeapth 判定は、関数側に任せる
				SCL_Now++;
			break;

			case(SCL_ENEMYPALETTE):
				LoadPaletteFromEnemy();	// BitDeapth 判定は、関数側に任せる
				SCL_Now++;
			break;

			default:	// 未実装 or ばぐ
				MWinOpen(&MWinRect);
				MWinCmd(MWCMD_NEWPAGE);
				MWinCmd(MWCMD_LARGEFONT);
				MWinMsg("バグ発生だにょ");
				SCL_DEBUG("---- SCL !BUG! ---");
			return;
		}
	}

	GameCount++;

	if((GameCount&0x3f)==0){
		if(GameStage == GRAPH_ID_EXSTAGE) PlayRankAdd(1);
		else                              PlayRankAdd(1+GameStage/3);
	}
}

static void _PutEnemy(BYTE *p)
{
/*
 * [メモ]
 *  p[0-1]:EnemyX  p[2-3]:EnemyY  p[4]:EnemyID
 *  ECL_Head[0-3]:Num  ECL_Head[n*4-(n*4+3)] (n>1):StartAddr(ABS)
 */
	DWORD			n;
	ENEMY_DATA		*e;
	short			x,y;

	if(EnemyNow+1>=ENEMY_MAX) return;

	e = Enemy+ (*(EnemyInd+EnemyNow));
	EnemyNow++;

	n      = 4 + (((BYTE)p[4])<<2);

	x   = (*(short *)(&p[0]));	//((int)(*(short *)(&p[0])))*64;
	y   = (*(short *)(&p[2]));	//((int)(*(short *)(&p[2])))*64;
	InitEnemyDataSTD(e,x,y,n);

/*
	e->x   = (*(short *)(&p[0]));	//((int)(*(short *)(&p[0])))*64;
	e->y   = (*(short *)(&p[2]));	//((int)(*(short *)(&p[2])))*64;

	e->x = (e->x==X_RNDV) ? GX_RND : (e->x<<6);
	e->y = (e->y==Y_RNDV) ? GY_RND : (e->y<<6);
	e->cmd = (*(DWORD *)(&ECL_Head[n]));

	e->call_addr = e->cmd;

	e->hp       = 0xffffffff;
	e->amp      = 0;
	e->anm_ptn  = 0;
	e->anm_sp   = 0;
	e->anm_c    = 0;
	e->count    = 0;
	e->evscore  = 0;
	e->d        = 64;
	e->flag     = EF_DAMAGE|EF_DRAW|EF_HITSB;

	e->tama_c   = rnd();//&0xff;
	e->t_rep    = 0;			// 弾の発射間隔(０：自動発射しない)
	e->g_width  = 0;
	e->g_height = 0;

	e->item     = 0;

	e->rep_c    = 0;
	e->cmd_c    = 0;
	e->v        = 64;
	e->vd       = 0;
	e->vx       = cosl(e->d,e->v);
	e->vy       = sinl(e->d,e->v);

	e->LLaserRef = 0;

	e->t_cmd.c      = 0;
	e->t_cmd.cmd    = TC_WAY;
	e->t_cmd.d      = 64;
	e->t_cmd.n      = 1;
	e->t_cmd.option = TE_NONE;
	e->t_cmd.type   = T_NORM;
	e->t_cmd.v      = 3;
	e->t_cmd.x      = 0;
	e->t_cmd.y      = 0;

	e->t_cmd.dw     = 16;
	e->t_cmd.ns     = 1;
	e->t_cmd.rep    = 0;
	e->t_cmd.vd     = 0;


	// 変数用レジスタの初期化 //
	e->GR[0] = e->GR[1] = e->GR[2] = e->GR[3] = 0;
	e->GR[4] = e->GR[5] = e->GR[6] = e->GR[7] = 0;

	// 割り込みベクタの初期化 //
	InitECLInterrupt(e);
*/
}

/*
static void ExDraw(void)
{
	int			x,y;
	int			dx,dy;
	int			infx,infy;
	int			i,j;

	int			ox,oy;

	static BYTE		deg = 0;
	static int		len = 0;
	static BYTE		count = 0;
	static char		flag = 1;

	ox = cosl(deg,len*flag)+320;
	oy = sinl(deg,len*flag)+240;

	count++;

	if(count==0)	flag *= -1;

	if(count>60)	len = (len+4)%64;
	else			deg+=4;

	GrpLock();
	GrpSetColor(0,3,0);

	dx = cosl(deg+64,64);		infx = cosl(deg,800);
	dy = sinl(deg+64,64);		infy = sinl(deg,800);
	for(i=-10;i<=10;i++){
		x = ox + dx*i;
		y = oy + dy*i;
		GrpLine(x-infx,y-infy,x+infx,y+infy);
	}

	dx = cosl(deg,64);		infx = cosl(deg+64,800);
	dy = sinl(deg,64);		infy = sinl(deg+64,800);
	for(i=-10;i<=10;i++){
		x = ox + dx*i;
		y = oy + dy*i;
		GrpLine(x-infx,y-infy,x+infx,y+infy);
	}

	GrpUnlock();
}
*/

// 背景を描画する //
FVOID ScrollDraw(void)
{
	PBGMAP	*p;
	RECT	src;
	int		i,j,k,x,y;
	int		dx = 0;				// 振動用
	int		RasterDx;			// ラスター用

	if(ScrollInfo.DataHead==NULL) return;

	// 例外処理：ワイドショット用ボム発動中 //
	//if(Viv.bomb_time && Viv.weapon==0){
	//	return;
	//}

	// 特殊描画モード //
	if(ScrollInfo.ExCmd == ScrollCmdStg3Boss){
		Stg3BossMapDraw();
		return;
	}
	else if(ScrollInfo.ExCmd == ScrollCmdStg6Cube){
		Draw3DCube();
		return;
	}
	else if(ScrollInfo.ExCmd == ScrollCmdStg6RndEcl){
		DrawEffectFakeECL();
		return;
	}
	else if(ScrollInfo.ExCmd == ScrollCmdStg6Raster){
		DrawStg6Raster();
		return;
	}
	else if(ScrollInfo.ExCmd == ScrollCmdStg3Star){
		DrawStg3Star();
		return;
	}

	// 振動エフェクト用 //
	//dx = sinl(ScrollInfo.IsQuake*4,2);
	//if(ScrollInfo.IsQuake) dx = sinl(ScrollInfo.IsQuake*8+i*6,(256-ScrollInfo.IsQuake)>>2);	//4
	if(ScrollInfo.IsQuake) dx = sinl(ScrollInfo.IsQuake*16,(256-ScrollInfo.IsQuake)>>5);	//4

	// 全てのレイヤーの表示 //
	for(k=0;k<ScrollInfo.NumLayer;k++){
		p = ScrollInfo.LayerPtr[k];
		for(i=29;i>=-1;i--){
			RasterDx = (k==0) ? ScrollInfo.RasterDx[i+1] : 0;
			for(j=0;j<MAP_WIDTH;){
				// 通常の描画 //
				if(*p != MAPDATA_NONE){
					x   = (j<<4) + X_MIN + dx + RasterDx;
					y   = (i<<4) + ScrollInfo.LayerDy[k];
					src = rcMapChip[*p];
					GrpBlt(&src,x,y,GrMap);
					p++,j++;
				}
				// 何もない場合 //
				else{
					j = j + (*(p+1));
					p = p + 2;
				}
			}
		}
	}

	if(ScrollInfo.ExCmd == ScrollCmdStg4Rock){
		DrawStg4Rock();
	}
/*
	if(ScrollInfo.ExCmd==ScrollCmdStg2Boss){
		ExDraw();
	}
*/
}

// スクロールスピードを変更する(引数:(1)スクロール速度) //
FVOID ScrollSpeed(int speed)
{
	//if(speed<0) speed = 0;		// 逆方向スクロール禁止！！

	ScrollInfo.ScrollSpeed = speed;
}

// ＳＣＬ用コマンド実行関数(引数:(1)スクロールコマンド) //
FVOID ScrollCommand(BYTE cmd)
{
	switch(cmd){
		case(SCMD_QUAKE):		// 振動エフェクト
			ScrollInfo.IsQuake = 2;
		break;

		case(SCMD_STG2BOSS):	// ２面ボス
			ScrollInfo.ExCmd   = ScrollCmdStg2Boss;
			ScrollInfo.ExCount = 0;
		break;

		case(SCMD_STG3BOSS):	// ３面ボス
			ScrollInfo.ExCmd   = ScrollCmdStg3Boss;
			ScrollInfo.ExCount = 0;
			//InitStg3Cloud();
		break;

		case(SCMD_STG3RESET):
			ScrollInfo.ExCmd   = ScrollCmdDummy;
			ScrollInfo.ExCount = 0;
		break;

		case(SCMD_STG6CUBE):
			ScrollInfo.ExCmd   = ScrollCmdStg6Cube;
			ScrollInfo.ExCount = 0;
			Init3DCube();
		break;

		case(SCMD_STG6RNDECL):
			ScrollInfo.ExCmd   = ScrollCmdStg6RndEcl;
			ScrollInfo.ExCount = 0;
			InitEffectFakeECL();
		break;

		case(SCMD_STG4ROCK):
			ScrollInfo.ExCmd   = ScrollCmdStg4Rock;
			ScrollInfo.ExCount = 0;
			InitStg4Rock();
		break;

		case(SCMD_STG4LEAVE):
			if(ScrollInfo.ExCmd != ScrollCmdStg4Rock) break;
			SendCmdStg4Rock(STG4ROCK_LEAVE, 0);
		break;

		case(SCMD_STG6RASTER):
			ScrollInfo.ExCmd   = ScrollCmdStg6Raster;
			ScrollInfo.ExCount = 0;
			InitStg6Raster();
		break;

		case(SCMD_STG3STAR):
			ScrollInfo.ExCmd = ScrollCmdStg3Star;
			ScrollInfo.ExCount = 0;
			InitStg3Star();
			ScreenEffectSet(SCNEFC_WHITEIN);
		break;

		case(SCMD_RASTER_ON):	// ラスタースクロール開始
			ScrollInfo.ExCmd       = ScrollCmdRasterOpen;
			ScrollInfo.RasterDeg   = 0;
			ScrollInfo.RasterWidth = 0;
		break;

		case(SCMD_RASTER_OFF):	// ラスタースクロール終了
			ScrollInfo.ExCmd       = ScrollCmdRasterClose;
			//ScrollInfo.RasterDeg   = 0;
			//ScrollInfo.RasterWidth = 0;
		break;
	}
}

// 特殊スクロール無し //
static FVOID ScrollCmdDummy(void)
{
	// 何もしないよぉ... //
}

// ２面ボスのスクロール //
static FVOID ScrollCmdStg2Boss(void)
{
/*
	SSP	-810	TR	10			SSP	-630	TR	10
	SSP	-450	TR	10			SSP	-270	TR	10
	SSP	-180	TR	10			SSP	-90		TR	10
	SSP	0		TR	10			SSP	90		TR	10
	SSP	180		TR	10			SSP	450		TR	10
	SSP	630		TR	10			SSP	810		TR	300
*/
	// 特殊タイマーにより分岐を行う //
	switch(ScrollInfo.ExCount){
		// 正方向->逆方向 //
		case(0):	ScrollSpeed(1512);		break;
		case(20):	ScrollSpeed(1200);		break;
		case(40):	ScrollSpeed(900);		break;
		case(60):	ScrollSpeed(600);		break;
		case(80):	ScrollSpeed(300);		break;
		case(100):	ScrollSpeed(150);		break;
		//case(120):	ScrollSpeed(0);			break;
		case(140):	ScrollSpeed(-150);		break;
		case(160):	ScrollSpeed(-300);		break;
		case(180):	ScrollSpeed(-600);		break;
		case(200):	ScrollSpeed(-900);		break;
		case(220):	ScrollSpeed(-1200);		break;
		case(240):	ScrollSpeed(-1512);		break;

		// 逆方向->正方向 //
		case(440):	ScrollSpeed(-1512);		break;
		case(460):	ScrollSpeed(-1200);		break;
		case(480):	ScrollSpeed(-900);		break;
		case(500):	ScrollSpeed(-600);		break;
		case(520):	ScrollSpeed(-300);		break;
		case(540):	ScrollSpeed(-150);		break;
		//case(560):	ScrollSpeed(0);			break;
		case(580):	ScrollSpeed(150);		break;
		case(600):	ScrollSpeed(300);		break;
		case(620):	ScrollSpeed(600);		break;
		case(640):	ScrollSpeed(900);		break;
		case(660):	ScrollSpeed(1200);		break;
		case(680):	ScrollSpeed(1512);		break;
	}

	ScrollInfo.ExCount = (ScrollInfo.ExCount+1)%880;
}

// ラスタースクロールオープン //
static FVOID ScrollCmdRasterOpen(void)
{
	int		i,j;

	// ちょっと重いかもね... //
	for(i=j=0;i<31;i++,j+=16){
		ScrollInfo.RasterDx[i] = (char)sinl(ScrollInfo.RasterDeg+j,ScrollInfo.RasterWidth);
	}

	ScrollInfo.RasterDeg+=2;

	if(ScrollInfo.RasterWidth<2) ScrollInfo.RasterWidth++;
}

// ラスタースクロールクローズ //
static FVOID ScrollCmdRasterClose(void)
{
	int		i,j;

	// ちょっと重いかもね... //
	for(i=j=0;i<31;i++,j+=2){
		ScrollInfo.RasterDx[i] = (char)sinl(ScrollInfo.RasterDeg+j,ScrollInfo.RasterWidth);
	}

	ScrollInfo.RasterDeg += 8;

	ScrollInfo.RasterWidth--;

	if(ScrollInfo.RasterWidth==0){
		ScrollInfo.ExCmd = ScrollCmdDummy;
	}
}

// ゲイツ雲 //
static FVOID ScrollCmdStg3Boss(void)
{
	ScrollInfo.ExCount = (ScrollInfo.ExCount+200)%208;
//	MoveStg3Cloud();
}

// ゲイツ雲描画 //
static FVOID Stg3BossMapDraw(void)
{
	int		x,y;
	RECT	src;

	x = X_MIN;
	y = Y_MIN - ScrollInfo.ExCount;

	for(; y<Y_MAX; y+=208){
		SetRect(&src, 0, 272, (640-256), 272+208);
		GrpBlt(&src,x,y,GrMap);
	}

//	DrawStg3Cloud();
}


// ６面の３Ｄキューう゛ //
static FVOID ScrollCmdStg6Cube(void)
{
	Move3DCube();
}

// ６面のランダムＥＣＬ列 //
static FVOID ScrollCmdStg6RndEcl(void)
{
	MoveEffectFakeECL();
}

// ６面ラスター //
static FVOID ScrollCmdStg6Raster(void)
{
	MoveStg6Raster();
}

// ４面岩 //
static FVOID ScrollCmdStg4Rock(void)
{
	MoveStg4Rock();
}

// ３面高速星 //
static FVOID ScrollCmdStg3Star(void)
{
	ScrollInfo.ExCount++;

	if(ScrollInfo.ExCount == 32){
		ScreenEffectSet(SCNEFC_WHITEOUT);
	}

	MoveStg3Star();
}


// マップデータをロードする(BMP含む) //
FBOOL ScrollInit(void)
{
	//BIT_DEVICE			*in;
	ScrollSaveHeader	*LayerInfo;
	int					i;
	static BOOL			bInitialized = FALSE;

	SclInfo.MsgFlag    = FALSE;
	SclInfo.ReturnFlag = FALSE;

	if(!bInitialized){
		InitMapChipRect();
		bInitialized = TRUE;
	}

/*
	// 読み込みの準備 //
	in = FilStartR("GIAN_MAP.DAT",BD_FILE_READ);
	if(in==NULL) return FALSE;

	// もしすでにロードが行われていた場合、メモリを解放する //
	if(ScrollInfo.DataHead!=NULL){
		LocalFree(ScrollInfo.DataHead);
		ScrollInfo.DataHead = NULL;
	}
	// そうでなければ、スクロールに関する初期化を行う //
	else if(!InitScrollInfo()) return FALSE;

	// 解凍を行う(後で、ステージを考慮したものに変更すること) //
	ScrollInfo.DataHead = MemExpand(in,0);
	if(ScrollInfo.DataHead==NULL) return FALSE;
	FilEnd(in);
*/
	// 標準のスクロールスピード(マップエディタと同じ) //
	ScrollInfo.ScrollSpeed = TIME_PER_FRAME;					// 標準のスクロール速度
	ScrollInfo.NumLayer    = *(DWORD *)ScrollInfo.DataHead;		// レイヤーの数
	ScrollInfo.Count       = 0;					// スクロール用カウンタ
	ScrollInfo.InfStart    = 0;					// 無限ループ開始時刻
	ScrollInfo.State       = SST_NORMAL;		// 状態(デフォルトの標準で..)
	ScrollInfo.IsQuake     = 0;					// 振動中ではない
	ScrollInfo.ExCmd       = ScrollCmdDummy;	// 特殊コマンド
	ScrollInfo.ExCount     = 0;
	ScrollInfo.RasterWidth = 0;
	ScrollInfo.RasterDeg   = 0;

	// ラスタースクロールの初期化 //
	for(i=0;i<31;i++){
		ScrollInfo.RasterDx[i] = 0;
	}

	// レイヤー情報のロード(長さからループ用変数を調整すること) //
	LayerInfo = (ScrollSaveHeader *)(ScrollInfo.DataHead + sizeof(DWORD));
	for(i=0;i<ScrollInfo.NumLayer;i++){
		ScrollInfo.LayerHead[i]  = (PBGMAP *)(ScrollInfo.DataHead+LayerInfo[i].Address);	// 先頭
		ScrollInfo.LayerPtr[i]   = (PBGMAP *)(ScrollInfo.DataHead+LayerInfo[i].Address);	// 現在
		ScrollInfo.LayerWait[i]  = LayerInfo[i].ScrollWait;		// レイヤーの重み
		ScrollInfo.LayerCount[i] = 0;
		ScrollInfo.LayerDy[i]    = 0;
	}

	// 無限ループ終了時刻 //
	ScrollInfo.InfEnd  = 16*(LayerInfo[i-1].Length-1280/16)*LayerInfo[i-1].ScrollWait;

	return TRUE;
}

// スクロールに関する情報の初期化を行う //
static void InitMapChipRect(void)
{
	int		i,x,y;

	// マップチップ用矩形の準備 //
	for(i=0;i<1200;i++){
		x = (i%(640/16))<<4;		// マップエディタと同様の演算式
		y = (i/(640/16))<<4;		// マップエディタと同様の演算式
		SetRect(&rcMapChip[i],x,y,x+16,y+16);
	}
}

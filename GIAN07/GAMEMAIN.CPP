/*                                                                           */
/*   GameMain.cpp   ウィンドウシステム切り替えなどの処理                     */
/*                                                                           */
/*                                                                           */

#include "GAMEMAIN.h"


extern HWND hWndMain;
DWORD		DrawCount = 0;
char		*MidiTitle;
DWORD		MidiPlayID = 0;
char		MidiTempo  = 0;
DWORD		DemoTimer  = 0;
BYTE		WeaponKeyWait = 0;
DWORD		GameOverTimer = 0;

BYTE TempGameLevel = 0;

NR_NAME_DATA	CurrentName;	// ネームレジスト準備用データ
BYTE			CurrentRank;	// ネームレジスト用順位データ
BYTE			CurrentDif;		// 現在の難易度(スコアネーム表示用)

MAID		VivTemp;

BOOL IsDemoplay = FALSE;


FVOID TitleProc(void);
FVOID WeaponSelectProc(void);	// 装備選択
FVOID GameProc(void);
FVOID GameOverProc0(void);		// ゲームオーバー出現用
FVOID GameOverProc(void);		// ゲームオーバー
FVOID PauseProc(void);
FVOID MusicRoomProc(void);		// 音楽室
FVOID DemoProc(void);			// デモプレイ
FVOID NameRegistProc(void);		// お名前入力

FVOID ReplayProc(void);


FBOOL SProjectInit(void);	// 西方Ｐｒｏｊｅｃｔ初期化部
FVOID SProjectProc(void);	// 西方Ｐｒｏｊｅｃｔ表示動作部

FVOID ScoreNameProc(void);

FVOID GameSTD_Init(void);		// ゲームを立ち上げる際に必ず行う初期化関数群
FBOOL DemoInit(void);			// デモプレイの初期化を行う

FVOID GameDraw(void);
FVOID GameMove(void);
FBOOL IsDraw(void);

FVOID ScoreDraw(void);		// スコアの描画

FVOIDFN(*GameMain)(void) = TitleProc;




// スコアネーム表示の準備を行う //
extern FBOOL ScoreNameInit(void)
{
	if(GameStage == GRAPH_ID_EXSTAGE) CurrentDif = GAME_EXTRA;
	else                              CurrentDif = ConfigDat.GameLevel;

	CurrentRank = (BYTE)SetScoreString(NULL, CurrentDif);
	if(CurrentRank == 0) return GameExit();

	MWinForceClose();
	GrpCls();
	GrpFlip();

	if(!LoadGraph(GRAPH_ID_NAMEREGIST)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}

	GrpSetClip(0,0,640,480);

	Key_Read();
	while(Key_Data) Key_Read();

	GameMain = ScoreNameProc;

	return TRUE;
}

// スコアネームの表示 //
FVOID ScoreNameProc(void)
{
	static const char *ExString[5] = {"Easy","Normal","Hard","Lunatic","Extra"};

	Key_Read();

	switch(Key_Data){
		case(KEY_RETURN):case(KEY_TAMA):case(KEY_BOMB):case(KEY_ESC):
			SndPlay(SOUND_ID_CANCEL);
			GameExit(FALSE);
		return;

		case(KEY_UP):case(KEY_LEFT):
			if(ScoreString[4].bMoveEnable) break;
			SndPlay(SOUND_ID_SELECT);
			CurrentDif  = (CurrentDif+4)%5;
			CurrentRank = (BYTE)SetScoreString(NULL, CurrentDif);
		break;

		case(KEY_DOWN):		case(KEY_RIGHT):
			if(ScoreString[4].bMoveEnable) break;
			SndPlay(SOUND_ID_SELECT);
			CurrentDif  = (CurrentDif+1)%5;
			CurrentRank = (BYTE)SetScoreString(NULL, CurrentDif);
		break;
	}

	GrpCls();
	ScoreDraw();
	GrpPut16(320,450,ExString[CurrentDif]);
	GrpFlip();
}

// スコアの描画 //
FVOID ScoreDraw(void)
{
	int		i,gx,gy,v;
	RECT	src;

	for(i=0;i<5;i++){
		v = (ScoreString[i].x - ((50  + i*24) << 6))/12;
		if(v > 64*2){
			//v = max(v,20*64);
			ScoreString[i].x -= v;
		}
		else
			ScoreString[i].bMoveEnable = FALSE;

		BltSetRect(&src,0,64+32*i,400,32);
		GrpBlt(&src,ScoreString[i].x>>6, ScoreString[i].y>>6, GrEnemy);
		//GrpBlt(&src,50+i*24,100+i*48,GrEnemy);

		gx = (ScoreString[i].x>>6) + 88;
		gy = (ScoreString[i].y>>6) + 4;
		GrpPut16c2(gx, gy, ScoreString[i].Name);

		gx = (ScoreString[i].x>>6) + 232-16;
		gy = (ScoreString[i].y>>6) + 4;
		GrpPut16c2(gx, gy, ScoreString[i].Score);

		gx = (ScoreString[i].x>>6) + 120;
		gy = (ScoreString[i].y>>6) + 25;
		GrpPutScore(gx, gy, ScoreString[i].Evade);

		// いや、時間が無いのは分かるんだけどさぁ... //
		gx = (ScoreString[i].x>>6) + 224;
		gy = (ScoreString[i].y>>6) + 25;
		if(ScoreString[i].Stage[0] == '7'){
			SetRect(&src, 288, 88, 288+16, 88+8);
			GrpBlt(&src, gx, gy-1, GrTama);
		}
		else
			GrpPutScore(gx, gy, ScoreString[i].Stage);

		gx = (ScoreString[i].x>>6) + 224 + 80;
		gy = (ScoreString[i].y>>6) + 25;
		BltSetRect(&src, 0, 400+ScoreString[i].Weapon*8, 48, 8);
		GrpBlt(&src, gx, gy-1, GrEnemy);
	}
}


#define NR_EXCHAR_BACK		0
#define NR_EXCHAR_END		-1
#define NR_EXCHAR_ERROR		-2

// 座標から選択文字を取得する //
char GetAddr2Char(int x, int y)
{
	// 大文字 //
	if(y == 0){
		return (char)('A' + (x % 26));
	}
	// 小文字 //
	else if (y == 1){
		return (char)('a' + (x % 26));
	}
	// その他記号など //
	else{
		switch(x){
		case 0:		return '0';
		case 1:		return '1';
		case 2:		return '2';
		case 3:		return '3';
		case 4:		return '4';
		case 5:		return '5';
		case 6:		return '6';
		case 7:		return '7';
		case 8:		return '8';
		case 9:		return '9';
		case 10:	return '!';
		case 11:	return '?';
		case 12:	return '#';
		case 13:	return '\\';
		case 14:	return '<';
		case 15:	return '>';
		case 16:	return '=';
		case 17:	return ',';
		case 18:	return '+';
		case 19:	return '-';
		case 20:	return ' ';		// SPACE
		//case 21:
		case 22:	return NR_EXCHAR_BACK;
		//case 23:
		case 24:	return NR_EXCHAR_END;
		//case 25:
		default:	return NR_EXCHAR_ERROR;
		}
	}
}

// お名前入力 //
FVOID NameRegistProc(void)
{
	// <- DemoInit() を修正するのだぞ
	RECT	src = {0,0,400,64};
	int		gx,gy,len;
	int		Level;
	static int	x,y;
	static BYTE key_time;
	static BYTE count;
	static BYTE time;
	//char	buf[100],
	char	c;

	Key_Read();

	if(key_time==0){
		key_time = 8;//16;

		switch(Key_Data){
			case(KEY_UP):
				y = (y+2)%3;
				SndPlay(SOUND_ID_SELECT);
			break;

			case(KEY_DOWN):
				y = (y+1)%3;
				SndPlay(SOUND_ID_SELECT);
			break;

			case(KEY_LEFT):
				if(y==2 && x>20)	x = (x-2)%26;
				else				x = (x+25)%26;
				SndPlay(SOUND_ID_SELECT);
			break;

			case(KEY_RIGHT):
				if(y==2 && x>=20)	x = (x+2)%26;
				else				x = (x+1)%26;
				SndPlay(SOUND_ID_SELECT);
			break;

			case(KEY_BOMB):
				SndPlay(SOUND_ID_CANCEL);
			goto BACK_NR_PROC;

			case(KEY_TAMA):case(KEY_RETURN):
				SndPlay(SOUND_ID_SELECT);

				// 最後の文字まで来ていた場合 //
				if(strlen(ScoreString[CurrentRank-1].Name)==NR_NAME_LEN-1){
					switch(c = GetAddr2Char(x,y)){
						case(NR_EXCHAR_END): case(NR_EXCHAR_ERROR):
						goto EXIT_NR_PROC;

						case(NR_EXCHAR_BACK):
						goto BACK_NR_PROC;

						default:
							x = 24;
							y = 2;
						break;
					}

					break;
				}

				// それ以外の場合 //
				switch(c = GetAddr2Char(x,y)){
					case(NR_EXCHAR_END): case(NR_EXCHAR_ERROR):
					goto EXIT_NR_PROC;

					case(NR_EXCHAR_BACK):
					goto BACK_NR_PROC;

					default:
						len = strlen(ScoreString[CurrentRank-1].Name);
						ScoreString[CurrentRank-1].Name[len  ] = c;
						ScoreString[CurrentRank-1].Name[len+1] = '\0';
					break;
				}
			break;

			// １文字前に戻る //
			BACK_NR_PROC:
				len = strlen(ScoreString[CurrentRank-1].Name);
				if(len) ScoreString[CurrentRank-1].Name[len-1] = '\0';
			break;

			// ネームレジスト終了処理 //
			EXIT_NR_PROC:
				if(strlen(ScoreString[CurrentRank-1].Name)==0)
					strcpy(ScoreString[CurrentRank-1].Name, "Vivit!");

				ScoreString[CurrentRank-1].Name[NR_NAME_LEN-1] = '\0';

				if(GameStage == GRAPH_ID_EXSTAGE) Level = GAME_EXTRA;
				else                              Level = ConfigDat.GameLevel;
				strcpy(CurrentName.Name, ScoreString[CurrentRank-1].Name);
				SaveScoreData(&CurrentName, Level);

				while(Key_Data) Key_Read();
				x = y = key_time = 0;
				GameExit();
			return;
		}

		if(x>20 && y==2)	x &= (~1);
	}
	else key_time--;

	if(Key_Data==0) key_time = 0;

	count = (count+1)%24;
	time++;

	GrpCls();

	GrpLock();

	GrpSetColor(2,0,0);
	gx = ScoreString[CurrentRank-1].x >> 6;
	gy = ScoreString[CurrentRank-1].y >> 6;
	GrpBox(gx,gy,gx+400,gy+32);

	if(time%64 > 32){
		GrpSetColor(4,0,0);
		len = min(strlen(ScoreString[CurrentRank-1].Name), NR_NAME_LEN-2);
		gx += (len*16 + 88);
		gy += (4);
		GrpBox(gx,gy,gx+14,gy+16);
	}

	GrpUnlock();

	GrpBlt(&src,120,0,GrEnemy);

	ScoreDraw();

	// 名前入力用文字列群 //
	SetRect(&src,0,432,416,480);
	GrpBlt(&src,112,420,GrEnemy);

	// カーソル //
	if(x>=20 && y==2)	BltSetRect(&src,432,432+((count>>3)<<4),32,16);
	else				BltSetRect(&src,416,432+((count>>3)<<4),16,16);
	GrpBlt(&src,112+(x<<4),420+(y<<4),GrEnemy);

	//sprintf(buf,"(%2d,%2d)", x, y);
	//GrpPut16(0,0,buf);

/*
	GrpPut16(400,100,temps);
	for(i=0; i<5; i++){
		GrpPut16(100, 100+i*32, ScoreString[i].Score);
		if(CurrentRank == i+1) GrpPut16(85, 100+i*32, "!!");
	}
*/
	GrpFlip();
}

// お名前入力の初期化 //
FBOOL NameRegistInit(BOOL bNeedChgMusic)
{
	int		i;
	BYTE	Level;

	for(i=0;i<NR_NAME_LEN;i++) CurrentName.Name[i] = '\0';
	CurrentName.Score  = Viv.score;
	CurrentName.Evade  = Viv.evade_sum;
	CurrentName.Weapon = Viv.weapon;
	if(GameStage == GRAPH_ID_EXSTAGE) CurrentName.Stage = 1;
	else                              CurrentName.Stage = GameStage;

	// デバッグ用... //
	SndStop(8);			// ワーニング音を止める
	SndStopAll();

	// ハイスコアで無いならばタイトルに移行する //
	if(GameStage == GRAPH_ID_EXSTAGE) Level = GAME_EXTRA;
	else                              Level = ConfigDat.GameLevel;
	CurrentRank = (BYTE)SetScoreString(&CurrentName, Level);
	if(CurrentRank == 0) return GameExit();

	MWinForceClose();
	GrpCls();
	GrpFlip();

	if(!LoadGraph(GRAPH_ID_NAMEREGIST)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}

	GrpSetClip(0,0,640,480);

	GameMain = NameRegistProc;

	if(bNeedChgMusic){
		Mid_Stop();
		LoadMusic(19);
		Mid_Play();
	}

	return TRUE;
}

// ゲームを立ち上げる際に必ず行う初期化関数群 //
FVOID GameSTD_Init(void)
{
	MWinForceClose();
	//GrpCls();
	//GrpFlip();

	BossDataInit();

	//MaidSet();
	MaidTamaIndSet();
	enemyind_set();
	tamaind_set(400+200);		// 小型弾に４００
	laserind_set();
	LLaserSetup();
	HLaserInit();
	SEffectInit();
	CEffectInit();
	ObjectLockOnInit();
	ItemIndSet();
	fragment_setup();
	ScreenEffectInit();
	ScreenEffectSet(SCNEFC_CFADEIN);

	ExBombEfcInit();

	InitWarning();
	WarningEffectInit();
	//WarningEffectSet();

	Mid_Tempo(0);

	//DrawCount = 0;
}


// ゲームの初期化をする //
extern FBOOL GameInit(void)
{
	GrpCls();
	GrpFlip();

	GameSTD_Init();
	PlayRankReset();

	MaidSet();

	GrpSetClip(0,0,640,480);

	WeaponKeyWait = 1;
	Viv.weapon    = 0;
	GameMain = WeaponSelectProc;

	VivTemp = Viv;

	return TRUE;
}

// エキストラステージを始める //
extern FBOOL GameExstgInit(void)
{
	GrpCls();
	GrpFlip();

	TempGameLevel       = ConfigDat.GameLevel;
	ConfigDat.GameLevel = GAME_HARD;

	GameSTD_Init();
	PlayRankReset();

	MaidSet();

	GrpSetClip(0,0,640,480);

	WeaponKeyWait = 1;
	Viv.weapon    = 0;
	GameMain = WeaponSelectProc;
	GameStage = GRAPH_ID_EXSTAGE;

	VivTemp = Viv;

	return TRUE;
}

// 次のステージに移行する //
extern FBOOL GameNextStage(void)
{
#ifdef PBG_DEBUG
	DemoplaySave();
#endif

	GameStage++;

	// エンディングに移行する //
	if(GameStage>=STAGE_MAX){
		GameStage = STAGE_MAX;	// 後で変更のこと
	}

	GameSTD_Init();
	MaidNextStage();

	if(!LoadGraph(GameStage)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}
	if(!LoadStageData(GameStage)){
		ErrInsert("ENEMY.DAT が破壊されています");
		return FALSE;
	}

	return TRUE;
}


// リプレイ用の初期化を行う //
extern FBOOL GameReplayInit(int Stage)
{
	MaidSet();

//	rnd_seed_set(timeGetTime());
	GameStage = Stage;

	if(!ReplayLoad(GameStage)){
		//ErrInsert("デモプレイデータが存在せず");
		return FALSE;
	}

	PlayRankReset();

	GrpCls();
	GrpFlip();
	GameSTD_Init();

	if(!LoadGraph(GameStage)){
		ErrInsert("GRAPH.DAT が破壊されています");
		ReplayLoadCleanup();
		return FALSE;
	}
	if(!LoadStageData(GameStage)){
		ErrInsert("ENEMY.DAT が破壊されています");
		ReplayLoadCleanup();
		return FALSE;
	}

	if(GameStage == GRAPH_ID_EXSTAGE){
		Viv.credit = 0;
	}

	GrpSetClip(X_MIN,Y_MIN,X_MAX,Y_MAX);
	GameMain = ReplayProc;

	Mid_FadeOut(240);

	return TRUE;
}

FVOID ReplayProc(void)
{
	static BYTE ExTimer;
	RECT	rc;

	ExTimer = (ExTimer+1)%128;

	Key_Read();
	if(Key_Data==KEY_ESC) Key_Data = KEY_ESC;
	else         Key_Data = ReplayMove();

	// ＥＳＣが押されたら即、終了 //
	if(Key_Data & KEY_ESC){
		ReplayLoadCleanup();
		GameExit();
		return;
	}

	GameMove();

	if(GameMain != ReplayProc){
		ReplayLoadCleanup();		// 後始末
		GameExit();					// 強制終了させる(ゲームオーバー対策)
		return;
	}

	if(IsDraw()){
		GameDraw();

		BltSetRect(&rc,312,80,32,8);
		GrpBlt(&rc,128,470,GrTama);
		if(ExTimer<64+32){
			GrpLock();
			GrpSetAlpha(128, ALPHA_NORM);
			GrpSetColor(0,0,0);
			GrpBoxA(128+45-3, 470+4-1, 128+45+72, 470+4+5);
			GrpUnlock();
			BltSetRect(&rc,312,88,72,8);
			GrpBlt(&rc,128+45,470+4,GrTama);
		}
		GrpFlip();
	}
}


// デモプレイの初期化を行う //
FBOOL DemoInit(void)
{
//	return NameRegistInit();

	GrpCls();
	GrpFlip();

	GameSTD_Init();

	MaidSet();

	rnd_seed_set(timeGetTime());
	GameStage = (rnd()%STAGE_MAX)+1;

//	GameStage = (rnd()%3)+1;		// この部分は体験版(コミケ)だけ //
//	GameStage = 1;					// こっちはＨＰ体験版用         //

	if(!DemoplayLoad(GameStage)){
		//ErrInsert("デモプレイデータが存在せず");
		return FALSE;
	}

	PlayRankReset();

	if(!LoadGraph(GameStage)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}
	if(!LoadStageData(GameStage)){
		ErrInsert("ENEMY.DAT が破壊されています");
		return FALSE;
	}

	GrpSetClip(X_MIN,Y_MIN,X_MAX,Y_MAX);
	GameMain = DemoProc;

	return TRUE;
}

void FadeoutPalette(PALETTEENTRY *Src, BYTE a)
{
	int				i;
	PALETTEENTRY	temp[256];

	for(i=0; i<256; i++){
		temp[i].peRed   = ((WORD)Src[i].peRed   * a) / 255;
		temp[i].peGreen = ((WORD)Src[i].peGreen * a) / 255;
		temp[i].peBlue  = ((WORD)Src[i].peBlue  * a) / 255;
	}

	GrpSetPalette(temp);
}

// 西方Ｐｒｏｊｅｃｔ表示動作部 //
FVOID SProjectProc(void)
{
	static WORD timer = 0;

	RECT	rc = {0,0,320,42};
	BYTE	d;
	int		x, y;

	timer = timer + 1;

	if(timer >= 256){
		timer   = 0;

		GameExit();
		return;
	}

	if(IsDraw()){
		GrpCls();//Ex(NULL, 255);

		GrpBlt(&rc,320-320/2,240+40,GrMap);

		if(timer < 64){
			if(DxObj.Bpp == 16){
				GrpLock();
				GrpSetAlpha((255-timer)*4, ALPHA_NORM);
				GrpSetColor(0,0,0);
				GrpBoxA(320-320/2,240+40,320-320/2+320,240+40+42);
				GrpUnlock();
			}
			else{
				FadeoutPalette(SProjectPalette, timer * 4);
			}
		}
		else if(timer > 192){
			if(DxObj.Bpp == 16){
				GrpLock();
				GrpSetAlpha(timer*4, ALPHA_NORM);
				GrpSetColor(0,0,0);
				GrpBoxA(320-320/2,240+40,320-320/2+320,240+40+42);
				GrpUnlock();
			}
			else{
				FadeoutPalette(SProjectPalette, (255-timer) * 4);
			}
		}
		else{
			d = timer - 64;
			x = 320 + sinl(d-64, 240);
			y = 295 + sinl(d*2, 20);
			GrpDrawLensBall(Lens, x, y);
		}

		GrpFlip();
	}
}


// 西方Ｐｒｏｊｅｃｔ表示の初期化 //
FBOOL SProjectInit(void)
{
	if(!LoadGraph(GRAPH_ID_SPROJECT)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}

	GameMain = SProjectProc;

	return TRUE;
}


// 音楽室の初期化をする //
extern FBOOL MusicRoomInit(void)
{
	MWinForceClose();
	GrpCls();
	GrpFlip();

	if(!LoadGraph(GRAPH_ID_MUSICROOM)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}

	GrpSetClip(0,0,640,480);

	MidiPlayID = 0;
	MidiTempo = 0;

	Mid_Tempo(0);

	Mid_TableInit();
/*
	Mid_Stop();
*/
	if(!LoadMusicRoomComment(0)){
		ErrInsert("ENEMY.DAT がはかいされています");
		GameExit();
		return FALSE;
	}
/*
	if(!LoadMusic(0)){
		ErrInsert("MUSIC.DAT が破壊されています");
		GameExit();
		return FALSE;
	}
*/
	MidiTitle = Mid_GetTitle();
//	Mid_Play();

	GameMain = MusicRoomProc;

	return TRUE;
}

// ゲームを再開する(ESC 抜けから) //
extern FVOID GameRestart(void)
{
	GameMain = GameProc;
}

// ゲームから抜ける //
extern FBOOL GameExit(BOOL bNeedChgMusic)
{
	RECT	rc;

	GrpCls();
	GrpFlip();

	if(GameStage == GRAPH_ID_EXSTAGE){
		ConfigDat.GameLevel = TempGameLevel;
	}

	if(!LoadGraph(GRAPH_ID_TITLE)){
		ErrInsert("GRAPH.DAT が破壊されています");
		return FALSE;
	}
	GrpSetClip(0,0,640,480);

	LLaserSetup();		// 音を止める
	SndStop(8);			// ワーニング音を止めるのだ

	InitMainWindow();

	//LoadFace(0,0);
	MWinForceClose();
	SetRect(&rc,128+8,400+16  +20  ,640-128-8,480);
	MWinOpen(&rc);
	//MWinFace(0);

	DemoTimer = 0;

	GameStage = 0;

	if(GameMain != DemoProc){
		if(bNeedChgMusic){
			Mid_Stop();
			LoadMusic(0);
			Mid_Play();
		}
	}

	GameMain  = TitleProc;

	return TRUE;
}

// ゲームオーバーの前処理
extern FVOID GameOverInit(void)
{
	StringEffect3();

	GameOverTimer = 120;

	GameMain = GameOverProc0;
}

// コンティニューを行う場合
extern FVOID GameContinue(void)
{
	Viv.evade_sum = 0;
	Viv.left      = ConfigDat.PlayerStock;
	Viv.score     = (Viv.score%10 + 1);

	GameMain = GameProc;

	// ここに入らなかったらバグなのだが... //
	if(Viv.credit){
		// クレジットの残っている場合(コンティニュー Y/N 処理へ) //
		Viv.credit -= 1;
	}
}

FVOID GameProc(void)
{
	Key_Read();

#ifdef PBG_DEBUG
	if(DebugDat.DemoSave) DemoplayRecord(Key_Data);
#endif

	// リプレイ時の保存処理 //
	if(ConfigDat.StageSelect) ReplayRecord(Key_Data);

	if(Key_Data & KEY_ESC){
		if(ConfigDat.StageSelect){
			ReplaySave();
			GameExit(TRUE);
			return;
		}

		InitExitWindow();
		GameMain = PauseProc;
		return;
	}
/*
	static BYTE count;
	if(count) count--;
	if((Key_Data & KEY_TAMA) && count==0){
		CEffectSet(Viv.x,Viv.y,CEFC_CIRCLE2);//STAR);
		count = 30;
	}
	if((Key_Data & KEY_BOMB) && count==0){
		CEffectSet(Viv.x,Viv.y,CEFC_CIRCLE1);//STAR);
		count = 30;
	}
*/
	GameMove();
	if(GameMain != GameProc) return;

	if(IsDraw()){
		GameDraw();
		if(DemoplaySaveEnable){
			RECT	rc;
			BltSetRect(&rc,288,80,24,8);
			GrpBlt(&rc,128,470,GrTama);
		}
		GrpFlip();
	}
}

// ゲームオーバー出現用
FVOID GameOverProc0(void)
{
	GameOverTimer--;

	if(GameOverTimer == 0){
		Key_Data = 0;
		while(Key_Data == 0) Key_Read();
		while(Key_Data != 0) Key_Read();

		// ステージセレクトが有効な場合 //
		if(ConfigDat.StageSelect){
			ReplaySave();
			if(GameStage == GRAPH_ID_EXSTAGE) NameRegistInit(TRUE);
			else                              GameExit(TRUE);
			return;
		}

		if(Viv.credit == 0){
			NameRegistInit(TRUE);
			//GameExit();
			return;			// 仮
		}

		InitContinueWindow();
		GameMain = GameOverProc;
		return;
	}
	else{
		fragment_move();
		SEffectMove();
	}

	if(IsDraw()){
		GameDraw();
		GrpFlip();
	}
}

// ゲームオーバー
FVOID GameOverProc(void)
{
	Key_Read();
	CWinMove(&ContinueWindow);
	if(GameMain != GameOverProc){
		SEffectInit();
		return;
	}

	if(IsDraw()){
		GameDraw();
		CWinDraw(&ContinueWindow);
		/*
		if(DemoplaySaveEnable){
			RECT	rc;
			BltSetRect(&rc,288,80,24,8);
			GrpBlt(&rc,128,470,GrTama);
		}*/
		GrpFlip();
	}
}

// デモプレイ
FVOID DemoProc(void)
{
	static BYTE ExTimer;

	ExTimer = (ExTimer+1)%128;

	Key_Read();
	if(Key_Data) Key_Data = KEY_ESC;
	else         Key_Data = DemoplayMove();

	IsDemoplay = TRUE;

	// ＥＳＣが押されたら即、終了 //
	if(Key_Data & KEY_ESC){
		DemoplayLoadCleanup();
		IsDemoplay = FALSE;
		GameExit();
		return;
	}

	GameMove();

	if(GameMain != DemoProc){
		DemoplayLoadCleanup();		// 後始末
		IsDemoplay = FALSE;
		GameExit();					// 強制終了させる(ゲームオーバー対策)
		return;
	}

	if(IsDraw()){
		GameDraw();
		if(ExTimer<64) GrpPut16(200,200,"D E M O   P L A Y");
		GrpFlip();
	}
}

// 装備選択 //
FVOID WeaponSelectProc(void)
{
	RECT	src[4],rc;
	int		i,x,y;
	BYTE	d;
	char	buf[100];

	static char deg  = 0;
	static char spd  = 0;
	static WORD	count = 0;

	BltSetRect(src+0,0,344,56,48);
	BltSetRect(src+1,0,392,56,48);
	BltSetRect(src+2,56,344,56,48);
	BltSetRect(src+3,56,392,56,48);

	deg += spd;
	if(deg>=85 || deg<=-85){
	//if(deg>=64 || deg<=-64){
		//if(spd<0) Viv.weapon = (Viv.weapon+3)%4;
		//else      Viv.weapon = (Viv.weapon+1)%4;
		if(spd<0) Viv.weapon = (Viv.weapon+2)%3;
		else      Viv.weapon = (Viv.weapon+1)%3;
		spd = 0;
		deg = 0;
		SndPlay(SOUND_ID_BUZZ);
	}

	Key_Read();
	if(WeaponKeyWait){
		if(!Key_Data) WeaponKeyWait = 0;
		else          Key_Data      = 0;
	}

	switch(Key_Data){
		case(KEY_RIGHT):
			if(spd<0){
				//Viv.weapon = (Viv.weapon+3)%4;
				//deg+=64;
				Viv.weapon = (Viv.weapon+2)%3;
				deg+=85;
			}
			spd  = 3;
		break;

		case(KEY_LEFT):
			if(spd>0){
				//Viv.weapon = (Viv.weapon+1)%4;
				//deg-=64;
				Viv.weapon = (Viv.weapon+1)%3;
				deg-=85;
			}
			spd  = -3;
		break;

		case(KEY_TAMA):case(KEY_RETURN):
			if(spd) break;
			if(GameStage == GRAPH_ID_EXSTAGE){
				if(!((1<<Viv.weapon) & ConfigDat.ExtraStgFlags)) break;
			}

			VivTemp.weapon = Viv.weapon;
			Viv = VivTemp;
			MaidTamaIndSet();
			count = 0;

			SndPlay(SOUND_ID_SELECT);
			if(GameStage != GRAPH_ID_EXSTAGE){
			#ifdef PBG_DEBUG
				GameStage = DebugDat.StgSelect;
				if(GameStage==2) Viv.exp = 160;
				if(GameStage>=3) Viv.exp = 255;
				if(DebugDat.DemoSave) DemoplayInit();
			#else
				// リプレイ用の処理を追加 //
				if(ConfigDat.StageSelect){
					GameStage = ConfigDat.StageSelect;
					if(GameStage==2) Viv.exp = 160;
					if(GameStage>=3) Viv.exp = 255;
					ReplayInit();
				}
				else{
					GameStage = 1;
				}
			#endif
			}
			else{
				Viv.credit = 0;
				Viv.left   = 2;
				Viv.exp    = 255;
				if(ConfigDat.StageSelect){
					ReplayInit();
				}
			}

			if(!LoadGraph(GameStage)){
				ErrInsert("GRAPH.DAT が破壊されています");
				return;
			}
			if(!LoadStageData(GameStage)){
				ErrInsert("ENEMY.DAT が破壊されています");
				return;
			}

			Mid_FadeOut(240);
			GrpSetClip(X_MIN,Y_MIN,X_MAX,Y_MAX);

			GameMain = GameProc;
		return;

		case(KEY_ESC):case(KEY_BOMB):
			if(spd) break;
			SndPlay(SOUND_ID_CANCEL);
			GameExit(FALSE);
		return;
	}

	count = (count+1)%(256+128);

	if(IsDraw()){
		GrpCls();

		SetRect(&rc,0,264-8,224,296-24);
		GrpBlt(&rc,320-112,20,GrTama);

		BltSetRect(&rc,0,272,64,24);
		GrpBlt(&rc,120-32,260-12,GrTama);

		d = ((count/8)%2)<<3;
		BltSetRect(&rc,72,272+d,56,8);
		GrpBlt(&rc,400-28+4,420,GrTama);

		for(i=0;i<3;i++){
		//for(i=0;i<4;i++){
			//d = (-i+Viv.weapon)*64 + deg - 64;
			d = (-i+Viv.weapon)*85 + deg - 64;
			x = 120 + cosl(d,90) - 56/2;
			y = 260 + sinl(d,110) - 48/2;
			GrpBlt(src+i,x,y,GrTama);
		}

		GrpLock();
		GrpSetColor(0,0,1);
		GrpSetAlpha(128, ALPHA_NORM);
		for(i=0;i<3;i++){
			if(GameStage != GRAPH_ID_EXSTAGE) continue;
			if((1<<i) & ConfigDat.ExtraStgFlags) continue;

			d = (-i+Viv.weapon)*85 + deg - 64;
			x = 120 + cosl(d,90) - 56/2;
			y = 260 + sinl(d,110) - 48/2;
			GrpBoxA(x, y, x+56, y+48);
		}
		GrpUnlock();

		Viv.exp = min(count,255);
		if(Viv.exp < 31) Viv.lay_time = Viv.lay_grp =  0;

		HomingFlag=HOMING_DUMMY;
		Key_Data = KEY_TAMA;

		Viv.muteki = 0;
		Viv.x = 400*64 + sinl((count/3)*6,60*64);
		Viv.y = 350*64 + sinl((count/3)*4,30*64);

		MaidMove();
		MaidTamaMove();

		GrpSetClip(400-110,400-300+2,400+110,400+10);
		for(x=400-110-2; x<400+110; x+=32){
			for(y=400-300+2+(count*2)%32-32; y<400+10; y+=32){
				d = Viv.weapon<<4;
				BltSetRect(&rc,224,256,32,32);
				//BltSetRect(&rc,d,296-24,16,16);
				GrpBlt(&rc,x,y,GrTama);
			}
		}
		MaidDraw();
		MaidTamaDraw();

		BltSetRect(&rc,72,272+16,56,8);
		GrpBlt(&rc,468,400,GrTama);
		sprintf(buf,"%d",((WORD)Viv.exp+1)>>5);
		GrpPutScore(500,400,buf);

		GrpLock();
		GrpSetClip(0,0,640,480);

		GrpSetColor(0,0,4);
		GrpLine(400-110,400-300,400+110,400-300);
		GrpLine(400-110,400+10,400+110,400+10);
		GrpLine(400-110,400-300,400-110,400+10);
		GrpLine(400+110,400-300,400+110,400+10);

		if(abs((char)deg)<=25){
			GrpSetColor(2,2,5);
			GrpCircle(120,150,39+10-2*abs((char)deg));
			GrpSetColor(4,4,5);
			GrpCircle(120,150,41+10-2*abs((char)deg));
		}
		GrpUnlock();

/*
		HDC		hdc;
		char	buf[100];
		DxObj.Back->GetDC(&hdc);
		sprintf(buf,"Viv.weapon = %d",Viv.weapon);
		TextOut(hdc,0,0,buf,strlen(buf));
		DxObj.Back->ReleaseDC(hdc);

		#ifdef PBG_DEBUG
			OutputDebugMsg();
		#endif
*/
		GrpFlip();
	}
}


FVOID TitleProc(void)
{
	RECT	src = {0,0,640,396};
	//RECT	src = {0,0,350,403};
	//RECT	src = {0,0,195,256};
	//RECT	src = {0,0,275,256};

/*
	// 鳩プロテクト? //
	if((GetAsyncKeyState(VK_F1) & 0x80000000)
		&& (GetAsyncKeyState(VK_F10) & 0x8000000))
		PostMessage(hWndMain,WM_SYSCOMMAND,SC_CLOSE,0);
*/

	Key_Read();
	if(Key_Data==0) DemoTimer += 1;
	else            DemoTimer  = 0;
	if(MainWindow.SelectDepth!=0) DemoTimer = 0;

	if(DemoTimer==60*10){	// 60*3
		DemoInit();
		return;
	}

	CWinMove(&MainWindow);
	MWinHelp(&MainWindow);
	MWinMove();
	if(GameMain!=TitleProc) return;

	if(MainWindow.State==CWIN_DEAD){
		switch(MainWindow.Select[0]){
			case(0):
				GameInit();
			return;

			default:
				// DestroyWindow->PostMessage
				// タスクバーにアイコンが残るのを防止だっけ？
				PostMessage(hWndMain,WM_SYSCOMMAND,SC_CLOSE,0);
			return;
		}
	}

	if(IsDraw()){
		GrpCls();
		GrpBlt(&src,0,42,GrEnemy);
		//GrpBlt(&src,320-175,77,GrEnemy);
		MWinDraw();
		CWinDraw(&MainWindow);
		#ifdef PBG_DEBUG
			OutputDebugMsg();
		#endif

		GrpFlip();
	}
}

FVOID PauseProc(void)
{
	RECT	rc;

	Key_Read();

	CWinMove(&ExitWindow);
	if(GameMain!=PauseProc) return;

	if(IsDraw()){
		GameDraw();

		GrpGetClip(&rc);
		GrpSetClip(0,0,640,480);
		CWinDraw(&ExitWindow);
		GrpSetClip(rc.left, rc.top, rc.right, rc.bottom);

		GrpFlip();
	}
}

// 音楽室 //
FVOID MusicRoomProc(void)
{
	RECT	src;
	HDC		hdc;
	HFONT	oldfont;
	char	buf[100];
	int		m,s;
	int		i;
	static WORD Old_Key;
	static BYTE DevChgWait;

	Key_Read();

	if(Key_Data!=Old_Key){
		switch(Key_Data){
			case(KEY_ESC):case(KEY_BOMB):
				DevChgWait = 0;
				GameExit();
			return;

			case(KEY_RIGHT):
				MidiPlayID+=2;
			case(KEY_LEFT):
				Mid_Stop();
				Mid_TableInit();
				MidiPlayID = (MidiPlayID+MusicNum-1)%MusicNum;
				LoadMusic(MidiPlayID);
				LoadMusicRoomComment(MidiPlayID);
				MidiTitle = Mid_GetTitle();
				Mid_Play();
			break;
		}

		Old_Key = Key_Data;
	}

	switch(Key_Data){
		case(KEY_UP):
			if(MidiTempo<100) MidiTempo++;
			Mid_Tempo(MidiTempo);
		break;
		case(KEY_DOWN):
			if(MidiTempo>-100) MidiTempo--;
			Mid_Tempo(MidiTempo);
		break;

		case(KEY_SHIFT):
			MidiTempo = 0;
			Mid_Tempo(0);
		break;
	}

	if(GetAsyncKeyState('F')&0x8000) Mid_FadeOut(120);

	if(GetAsyncKeyState('D')&0x8000){
		if(DevChgWait==0){
			Mid_ChgDev(1);
			Mid_TableInit();
			DevChgWait = 1;
		}
	}
	else{
		// 押されていなければ再び有効化する //
		DevChgWait = 0;
	}

	// フェードなどで停止した場合 //
	if(Mid_Dev.state==MIDST_STOP){
		Mid_TableInit();
	}

	if(IsDraw()){
		GrpCls();
		SetRect(&src,0,0,640,392);
		GrpBlt(&src,0,0,GrEnemy);

		SetRect(&src,0,392,640,432);
		GrpBlt(&src,8,410,GrEnemy);
		//GrpDrawSpect(0,480);
		GrpDrawSpect(352,128);
		GrpDrawNote();

		m = (Mid_PlayTime/1000)/60;
		s = (Mid_PlayTime/1000)%60;
		sprintf(buf,"%02d : %02d",m,s);
		GrpPut7B(560,40+2+2,buf);
		//TextOut(hdc,561,40+2,buf,strlen(buf));

		sprintf(buf,"%07d",Mid_PlayTime);
		GrpPut7B(560,64+2+2,buf);
		//TextOut(hdc,561,64+2,buf,strlen(buf));

		sprintf(buf,"%3d",MidiTempo);
		GrpPut7B(560,112+2+2,buf);
		//TextOut(hdc,561,112+2,buf,strlen(buf));

		if(DxObj.Back->GetDC(&hdc)==DD_OK){
			oldfont = (HFONT)SelectObject(hdc,WinGrpInfo.NormalFont);

			//SetTextColor(hdc,RGB(255*5/5,255*2/5,255*1/5));
			SetBkMode(hdc,TRANSPARENT);

			sprintf(buf,"#%02d %.26s",MidiPlayID+1,MTitleBuffer);
			SetTextColor(hdc,RGB(51,102,153));
			TextOut(hdc,401,144+2,buf,strlen(buf));
			SetTextColor(hdc,RGB(153,204,255));
			TextOut(hdc,400,144+2,buf,strlen(buf));
			SelectObject(hdc,oldfont);
			oldfont = (HFONT)SelectObject(hdc,WinGrpInfo.SmallFont);

			for(i=0; i<12; i++){
				TextOut(hdc, 400-40, 144+30+i*16, MCommentBuffer[i], strlen(MCommentBuffer[i]));
			}

			sprintf(buf,"%.13s",Mid_Dev.name[Mid_Dev.NowID]);
			TextOut(hdc,540+2,96-3,buf,strlen(buf));

			strcpy(buf,"秋霜玉    Version 1.005     ★デモ対応版＃★");
			TextOut(hdc,200-50,460,buf,strlen(buf));

			SelectObject(hdc,oldfont);
			DxObj.Back->ReleaseDC(hdc);
		}

		GrpFlip();
	}
}

/*
_inline XAdd(DWORD old,int id)
{
	RndBuf[id] += (random_ref-old);
}
*/


FVOID GameMove(void)
{
	MWinMove();

	ScrollMove();

	BossMove();
	enemy_move();
	ItemMove();
	tama_move();
	laser_move();
	LLaserMove();
	HLaserMove();
	fragment_move();
	SEffectMove();
	CEffectMove();
	ExBombEfcMove();
	ObjectLockMove();

	WarningEffectMove();
	ScreenEffectMove();

	// この２行の位置を変更しました //
	MaidMove();
	MaidTamaMove();
}


FVOID GameDraw(void)
{
//	RECT	rc;


	GrpCls();

	ScrollDraw();
	CEffectDraw();

	BossDraw();

	WideBombDraw();		// 多分、ここで良いと思うが...

	ExBombEfcDraw();

	enemy_draw();

	MaidTamaDraw();

	MaidDraw();

	if(DxObj.Bpp==8) LLaserDraw();

	ObjectLockDraw();

	fragment_draw();
	ItemDraw();

	if(DxObj.Bpp==16) LLaserDraw();

	HLaserDraw();
	laser_draw();
	tama_draw();

	static BYTE test = 0;

	//if((Key_Data&KEY_UP  ) && test<64) test++;
	//if((Key_Data&KEY_DOWN) && test!=0 ) test--;
	WarningEffectDraw();
	//MoveWarning(test++);
	//DrawWarning();

	SEffectDraw();
	StateDraw();

	BossHPG_Draw();
	ScreenEffectDraw();

//	GrpGetClip(&rc);
//	GrpSetClip(0,0,640,480);
	MWinDraw();
//	GrpSetClip(rc.left, rc.top, rc.right, rc.bottom);

#ifdef PBG_DEBUG
	OutputDebugMsg();
#else
	StdStatusOutput();
#endif
/*
	HDC		hdc;
	char	buf[100];
	MID_DATA	dat;

	if(DxObj.Back->GetDC(&hdc) == DD_OK){
		Mid_GetData(&dat);
		sprintf(buf, "c1 : %I64u", dat.nticks);
		//sprintf(buf, "timebase : %d", dat.timebase);
		TextOut(hdc, 0, 0, buf, strlen(buf));
		//sprintf(buf, "tempo    : %d", dat.tempo);
		//TextOut(hdc, 0, 30, buf, strlen(buf));

		DxObj.Back->ReleaseDC(hdc);
	}
*/
}

FBOOL IsDraw(void)
{
	if(ConfigDat.DrawMode){
		DrawCount++;
		if(DrawCount%ConfigDat.DrawMode) return FALSE;
	}

	return TRUE;
}

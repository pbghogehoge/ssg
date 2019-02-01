/*                                                                           */
/*   LOADER.cpp   グラフィック、サウンド等のロード                           */
/*                                                                           */
/*                                                                           */

#include "LOADER.h"
#include "Lens.h"


// グローバル変数 //
GRP GrTama   = NULL;			// 弾など
GRP GrEnemy  = NULL;			// 雑魚など
GRP GrMap    = NULL;			// 背景
GRP GrMTitle = NULL;			// 曲のタイトル加工用
GRP GrBomber = NULL;			// ボム用グラフィック用
GRP GrLens   = NULL;			// レンズエフェクト用

LensInfo	*Lens = NULL;

DWORD		MusicNum = 0;			// 曲数
FACE_DATA	FaceData[FACE_MAX];		// 顔グラ用
ENDING_GRP	EndingGrp[6];			// エンディング用

char MCommentBuffer[12][50];
char MTitleBuffer[50];




/*
static BOOL			bIsBombPalette = FALSE;
static PALETTEENTRY	tempPalette[256];
*/

static PALETTEENTRY EnemyPalette[256];
PALETTEENTRY SProjectPalette[256];



// 秘密の関数 //
static FVOID SetAnimeRect(ANIME_DATA *anm,int x, int y, int w, int h);
static FVOID SetAnimeRect2(ANIME_DATA *anm,int x1, int y1, int x2, int y2);
static FVOID SetAnimeTH(ANIME_DATA *anm, int x, int y, int n);


// Surface を作成する //
BOOL InitSurface(void)
{
	int			i;

//	bIsBombPalette = FALSE;

	GrTama = GrpCreateSurface(640,480,SURFACE_ID_SYSTEM);
	if(GrTama==NULL) return FALSE;

	GrMap = GrpCreateSurface(640,480,SURFACE_ID_MAPCHIP);
	if(GrMap==NULL) return FALSE;

	GrEnemy = GrpCreateSurface(640,480,SURFACE_ID_ENEMY);
	if(GrEnemy==NULL) return FALSE;

	for(i=0;i<FACE_MAX;i++){
		FaceData[i].GrSurf = GrpCreateSurface(96*FACE_NUMX,96,SURFACE_ID_FACE0+i);
		if(FaceData[i].GrSurf==NULL) return FALSE;
	}

	GrBomber = GrpCreateSurface(640, 480, SURFACE_ID_BOMBER);
	if(GrBomber == NULL) return FALSE;

	GrMTitle = GrpCreateSurface(640,20,SURFACE_ID_MTITLE);
	if(GrMTitle==NULL) return FALSE;


	// レンズをすでに作成しているのなら、破棄する //
	if(Lens) GrpReleaseLensBall(Lens);

	GrLens = GrpCreateSurface(140,140,SURFACE_ID_LENS);
	if(GrLens==NULL) return FALSE;

	Lens = GrpCreateLensBall(70, 36, GrLens);
	if(Lens == NULL) return FALSE;

	return TRUE;
}


//  レンズを破棄する
void CleanupLens(void)
{
	if(Lens) GrpReleaseLensBall(Lens);
}


// あるステージのグラフィックをロードする //
BOOL LoadGraph(int stage)
{
	BIT_DEVICE *in;
	int			i, x, y;
	HRESULT		ddrval;
	RECT		src;

//	bIsBombPalette = FALSE;

	// 音楽室用 //
	if(stage==GRAPH_ID_MUSICROOM){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;
		if(!GrpBMPLoadP(in,0,SURFACE_ID_SYSTEM))		return FALSE;
		if(!GrpBMPLoadP(in,19+4,SURFACE_ID_MUSIC))		return FALSE;
		FilEnd(in);

		return TRUE;
	}
	// タイトル画面用 //
	if(stage==GRAPH_ID_TITLE){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;
		if(!GrpBMPLoadP(in,0,SURFACE_ID_SYSTEM))		return FALSE;
		if(!GrpBMPLoadP(in,20+4,SURFACE_ID_MUSIC))		return FALSE;
		//LoadPaletteFromMusic();
		FilEnd(in);

		return TRUE;
	}
	// お名前登録画面用 //
	if(stage==GRAPH_ID_NAMEREGIST){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;
		if(!GrpBMPLoadP(in,0,SURFACE_ID_SYSTEM))		return FALSE;
		if(!GrpBMPLoadP(in,21+4,SURFACE_ID_NAMEREG))	return FALSE;
		FilEnd(in);

		return TRUE;
	}
	// 西方Ｐｒｏｊｅｃｔ表示用 //
	if(stage==GRAPH_ID_SPROJECT){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;
		if(!GrpBMPLoadP(in,31,SURFACE_ID_SPROJECT))		return FALSE;
		GrpGetPalette(SProjectPalette);

		//if(!GrpBMPLoadP(in,21+4,SURFACE_ID_NAMEREG))	return FALSE;
		FilEnd(in);

		return TRUE;
	}
	// エンディング全画像ロード(パレット含む) //
	if(stage==GRAPH_ID_ENDING){
		in = FilStartR("GRAPH2.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;

		SetRect(&src, 0, 0, 320, 240);
		for(i=0; i<6; i++){
			if(!GrpBMPLoadP(in,1+i,SURFACE_ID_MAPCHIP))		return FALSE;
			GrpGetPalette(EndingGrp[i].pal);
			if(i>=4) BltSetRect(&EndingGrp[i].rcTarget, (i%4)*320, 0, 320, 240);
			else     BltSetRect(&EndingGrp[i].rcTarget, (i%2)*320, (i/2)*240, 320, 240);

			if(i<4) EndingGrp[i].GrSurf = GrEnemy;
			else    EndingGrp[i].GrSurf = GrBomber;

			x = EndingGrp[i].rcTarget.left;
			y = EndingGrp[i].rcTarget.top;
			for(;;){
				ddrval = EndingGrp[i].GrSurf->BltFast(x,y,GrMap,&src,DDBLTFAST_NOCOLORKEY);
				if(ddrval == DD_OK){
					break;
				}
				if(ddrval == DDERR_SURFACELOST){
					if(!GrpRestore())
					return FALSE;
				}
				if(ddrval != DDERR_WASSTILLDRAWING){
					return FALSE;
				}
			}
		}

		if(!GrpBMPLoadP(in, 0, SURFACE_ID_MAPCHIP))		return FALSE;

		FilEnd(in);

		return TRUE;
	}


	// エキストラステージシステム用 //
	if(stage == GRAPH_ID_EXSTAGE){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;
		if(!GrpBMPLoadP(in,    0, SURFACE_ID_SYSTEM))	return FALSE;
		if(!GrpBMPLoadP(in, 27+1, SURFACE_ID_ENEMY))	return FALSE;
		GrpGetPalette(EnemyPalette);

		if(!GrpBMPLoadP(in,   27, SURFACE_ID_MAPCHIP))	return FALSE;

		// 諸事情により、ここにいるのです //
		if(!GrpBMPLoadP(in,      26,SURFACE_ID_BOMBER))	return FALSE;

		FilEnd(in);
		return TRUE;
	}

	// エキストラステージボス用(1) //
	if(stage == GRAPH_ID_EXBOSS1){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;

		if(!GrpBMPLoadP(in, 29, SURFACE_ID_ENEMY))		return FALSE;
		GrpGetPalette(EnemyPalette);

		FilEnd(in);
		return TRUE;
	}

	// エキストラステージボス用(2) //
	if(stage == GRAPH_ID_EXBOSS2){
		in = FilStartR("GRAPH.DAT",BD_FILE_READ);
		if(in==NULL) return FALSE;

		if(!GrpBMPLoadP(in, 30, SURFACE_ID_ENEMY))		return FALSE;
		GrpGetPalette(EnemyPalette);

		FilEnd(in);
		return TRUE;
	}

	if(stage<0 || stage>STAGE_MAX) return FALSE;

	// マップチップのロードは後で変換すること //
	in = FilStartR("GRAPH.DAT",BD_FILE_READ);
	if(in==NULL) return FALSE;
	if(!GrpBMPLoadP(in,       0,SURFACE_ID_SYSTEM))		return FALSE;
	if(!GrpBMPLoadP(in,stage+ 0,SURFACE_ID_ENEMY))		return FALSE;
	GrpGetPalette(EnemyPalette);

	//DWORD MapChipID[6] = {7,7,8,9,10,11};	// 本当は STAGE_MAX とすべき
	DWORD MapChipID[STAGE_MAX] = {7,8,9,10,11,12};
	if(!GrpBMPLoadP(in,MapChipID[stage-1],SURFACE_ID_MAPCHIP))	return FALSE;

	// 諸事情により、ここにいるのです //
	if(!GrpBMPLoadP(in,      26,SURFACE_ID_BOMBER))		return FALSE;

	FilEnd(in);

	return TRUE;
}

// 顔グラフィックをロードする //
BOOL LoadFace(BYTE SurfaceID,BYTE FileNo)
{
	BIT_DEVICE *in;

	if(SurfaceID<0 || SurfaceID>=SURFACE_MAX) return FALSE;

	in = FilStartR("GRAPH.DAT",BD_FILE_READ);
	if(in==NULL) return FALSE;
	if(!GrpBMPLoadP(in,13+FileNo,SURFACE_ID_FACE0+SurfaceID))		return FALSE;
	FilEnd(in);

	// パレットを保存する //
	GrpGetPalette(FaceData[SurfaceID].pal);

	return TRUE;
}

void LoadPaletteFromMusic(void)
{
	LPDIRECTDRAWPALETTE		lpdp;

	if(DxObj.Bpp != 8) return;
	if(!GrEnemy) return;
	if(GrEnemy->GetPalette(&lpdp) != DD_OK) return;

	lpdp->GetEntries(0,0,256,DxObj.pe);
	GrpSetPalette(DxObj.pe);

	lpdp->Release();
}

// 前後にある４０色をマップパーツのパレットにする //
void LoadPaletteFromMap(void)
{
	LPDIRECTDRAWPALETTE		lpdp;

	if(DxObj.Bpp != 8) return;
	if(!GrMap) return;
	if(GrMap->GetPalette(&lpdp) != DD_OK) return;
/*
	if(bIsBombPalette){
		lpdp->GetEntries(0, 0, 256, tempPalette);
	}
	else{
*/
		lpdp->GetEntries(0,0,256,DxObj.pe);
		GrpSetPalette(DxObj.pe);
//	}

	lpdp->Release();
}

// 敵のパレットにする
extern void LoadPaletteFromEnemy(void)
{
	if(DxObj.Bpp != 8) return;
	GrpSetPalette(EnemyPalette);
	return;
/*
	LPDIRECTDRAWPALETTE		lpdp;

	if(DxObj.Bpp != 8) return;
	if(!GrMap) return;
	if(GrEnemy->GetPalette(&lpdp) != DD_OK) return;
/*
	if(bIsBombPalette){
		lpdp->GetEntries(0, 0, 256, tempPalette);
	}
	else{
*/
/*
		lpdp->GetEntries(0,0,256,DxObj.pe);
		GrpSetPalette(DxObj.pe);
//	}

	lpdp->Release();
*/
}

// ＥＣＬ&ＳＣＬデータ列をメモリ上にロードする //
BOOL LoadStageData(BYTE stage)
{
	BIT_DEVICE		*in;
	int				i;

	// メモリを解放だ！ //
	if(ECL_Head!=NULL)				LocalFree(ECL_Head);
	if(SCL_Head!=NULL)				LocalFree(SCL_Head);
	if(ScrollInfo.DataHead!=NULL)	LocalFree(ScrollInfo.DataHead);
	SCL_Now = ECL_Head = SCL_Head = NULL;
	ScrollInfo.DataHead = NULL;

	if((in=FilStartR("ENEMY.DAT",BD_FILE_READ))==NULL) return FALSE;

	// エキストラステージシステム用 //
	if(stage == GRAPH_ID_EXSTAGE){
		if((ECL_Head=MemExpand(in, 24))==NULL)				return FALSE;	// ECL Load
		if((SCL_Head=MemExpand(in, 25))==NULL)				return FALSE;	// SCL Load
		if((ScrollInfo.DataHead=MemExpand(in, 26))==NULL)	return FALSE;	// MapData Load
	}
	else if(stage == GRAPH_ID_ENDING){
		if((SCL_Head=MemExpand(in, 47))==NULL) return FALSE;	// SCL Load

		SCL_Now   = SCL_Head;
		GameCount = 0;
		FilEnd(in);
		return TRUE;
	}
	else{
		// 各データをロードする //
		if(stage<1 || stage>STAGE_MAX)								return FALSE;
		if((ECL_Head=MemExpand(in,stage+0-1))==NULL)				return FALSE;	// ECL Load
		if((SCL_Head=MemExpand(in,stage+6-1))==NULL)				return FALSE;	// SCL Load
		if((ScrollInfo.DataHead=MemExpand(in,stage+12-1))==NULL)	return FALSE;	// MapData Load
	}

	// ファイルを閉じる //
	FilEnd(in);

	// スクロール用変数の初期化 //
	if(!ScrollInit()) return FALSE;

	// 各変数の初期化 //
	SCL_Now   = SCL_Head;
	GameCount = 0;

	// アニメーションの準備 //
	switch(stage){
		case(GRAPH_ID_EXSTAGE):		// エキストラステージのグラフィック矩形
			// Extra Boss I //
			// 00 : ■Ａ　0 ～ 3   :  翼無し通常　（10fpp)
			SetAnimeTH(Anime+0, 0, 0, 4);

			// 01 : ■Ｂ　4 ～ 7   :  翼有り通常　（10fpp)
			SetAnimeTH(Anime+1, 320, 0, 4);

			// 02 : ■Ｃ　8 ～ 13  :  翼装着　（翼無し->有り）　（6fpp)
			SetAnimeTH(Anime+2, 0, 80, 6);
			Anime[2].mode = ANM_STOP;

			// 03 : ■Ｄ　14 ～ 15 :  翼有り時攻撃（移動無し）　（6fpp)
			SetAnimeTH(Anime+3, 480, 80, 2);

			// 04 : ■Ｅ　16 ～ 17 :  翼装着時移動（もしくは移動攻撃）左　（6fpp)
			SetAnimeTH(Anime+4, 0, 160, 2);

			// 05 : ■Ｆ　18 ～ 19 :  翼装着時移動（もしくは移動攻撃）右　（6fpp)
			SetAnimeTH(Anime+5, 160, 160, 2);

			// 06 : ■Ｇ　24 ～ 30 :  段階変化　（翼有り->無し）　（6fpp)
			SetAnimeTH(Anime+6, 0, 240, 6);
			Anime[6].mode = ANM_STOP;

			// 07 : ■20 : 通常時ダメージ用マスク　（翼有り、無し兼用）
			SetAnimeTH(Anime+7, 320, 160, 1);

			// 08 : ■21 : 停止攻撃時ダメージ用マスク　
			SetAnimeTH(Anime+8, 400, 160, 1);

			// 09 : ■22 : 移動時（左）ダメージ用マスク　
			SetAnimeTH(Anime+9, 480, 160, 1);

			// 10 : ■23 : 移動時（右）ダメージ用マスク　
			SetAnimeTH(Anime+10, 560, 160, 1);

			SetAnime32(0   , 320+32*0, 11, 4, ANM_NORM);
			SetAnime32(0   , 320+32*1, 12, 4, ANM_NORM);
			SetAnime32(0   , 320+32*2, 13, 4, ANM_NORM);
			SetAnime32(0   , 320+32*3, 14, 4, ANM_NORM);
			SetAnime32(0   , 320+32*4, 15, 4, ANM_NORM);
			SetAnime32(32*4, 320     , 16, 4, ANM_NORM);
			SetAnime32(32*4, 320+32*1, 17, 1, ANM_NORM);

			// Extra Boss II //
			// 18 : ■Ａ : 停止アニメ　（10～12fpp)
			SetAnimeTH(Anime+18, 0, 0, 4);

			// 19 : ■Ｂ : 通常段階攻撃１　（?fpp)
			SetAnimeTH(Anime+19, 320, 0, 4);
			Anime[19].mode = ANM_STOP;

			// 20 : ■Ｃ :  通常段階攻撃２　および、高速移動前溜めポーズ　（6fpp)
			SetAnimeTH(Anime+20, 0, 80, 2);

			// 21 : ■Ｄ : 魂状態（ショットに当たらない無敵） (1 ～ 2 fpp）
			// (160,80), (200,80), (240,80), (280,80)
			Anime[21].width  = 40;
			Anime[21].height = 40;
			Anime[21].n      = 4;
			Anime[21].mode   = ANM_NORM;
			for(i=0; i<4; i++)  BltSetRect(Anime[21].ptn+i, 160+i*40, 80, 40, 40);

			// 22 : ■Ｅ : ダメージマスク(A)
			SetAnimeTH(Anime+22, 320, 80, 1);

			// 23 : ■Ｅ : ダメージマスク(B)
			SetAnimeTH(Anime+23, 400, 80, 1);

			// 24 : ■Ｅ : ダメージマスク(G)
			SetAnimeTH(Anime+24, 480, 80, 1);

			// 25 : ■Ｅ : ダメージマスク(C)
			SetAnimeTH(Anime+25, 560, 80, 1);

			// 26 : ■Ｆ : 高速移動アニメ
			Anime[26].width  = 80;
			Anime[26].height = 80;
			Anime[26].n      = 16;
			Anime[26].mode   = ANM_DEG;		// 16 パターンで助かりましたな...
			for(i=0; i<16; i++)
				BltSetRect(Anime[26].ptn+i, (i*80)%640, 160, 80, 80);

			// 27 : ■Ｇ : 通常段階攻撃２溜めポーズ　および、ワープ前後、
			SetAnimeTH(Anime+27, 560, 320, 1);

			// 28-32 : 陰陽玉ｘ５
			SetAnime32(  0, 384   , 28, 8, ANM_NORM);
			SetAnime32(  0, 384+32, 29, 8, ANM_NORM);
			SetAnime32(  0, 384+64, 30, 8, ANM_NORM);
			SetAnime32(256, 384+32, 31, 8, ANM_NORM);
			SetAnime32(256, 384+64, 32, 8, ANM_NORM);

			SetAnime32(0,  0, 33, 16, ANM_DEG);
			SetAnime32(0, 32, 34, 16, ANM_DEG);
			SetAnime32(0, 64, 35, 16, ANM_DEG);
			SetAnime32(0, 96, 36, 16, ANM_DEG);
			SetAnime32(0,128, 37, 16, ANM_DEG);

			// レーザー発射物 //
			Anime[38].width  = 40;
			Anime[38].height = 56;
			Anime[38].n      = 1;
			Anime[38].mode   = ANM_NORM;
			BltSetRect(Anime[38].ptn+0, 512, 0, 40, 56);

			// 中ボス //
			Anime[39].width  = 72;
			Anime[39].height = 56;
			Anime[39].n      = 2;
			Anime[39].mode   = ANM_NORM;
			SetRect(Anime[39].ptn+0, 0,424,72  ,480);
			SetRect(Anime[39].ptn+1,72,424,72*2,480);

			// 中ボスヒット //
			Anime[40].width  = 72;
			Anime[40].height = 56;
			Anime[40].n      = 1;
			Anime[40].mode   = ANM_NORM;
			SetRect(Anime[40].ptn+0,72*2,424,72*3,480);

			// レーザー発射物ヒット //
			Anime[41].width  = 40;
			Anime[41].height = 64;
			Anime[41].n      = 1;
			Anime[41].mode   = ANM_NORM;
			BltSetRect(Anime[41].ptn+0, 512, 56, 40, 56);

			// 謎の光弾 //
			Anime[42].width  = 24;
			Anime[42].height = 24;
			Anime[42].n      = 4;
			Anime[42].mode   = ANM_NORM;
			BltSetRect(Anime[42].ptn+0, 552,  0, 24, 24);
			BltSetRect(Anime[42].ptn+1, 552, 24, 24, 24);
			BltSetRect(Anime[42].ptn+2, 552,  0, 24, 24);
			BltSetRect(Anime[42].ptn+3, 552, 48, 24, 24);
		break;

		case(1):		// Ｓｔａｇｅ１のグラフィック矩形
			// 中ボス //
			Anime[0].width  = 72;
			Anime[0].height = 56;
			Anime[0].n      = 2;
			Anime[0].mode   = ANM_NORM;
			SetRect(Anime[0].ptn+0, 0,0,72  ,56);
			SetRect(Anime[0].ptn+1,72,0,72*2,56);

			SetAnime32(0,56+ 0,1,16,ANM_DEG);
			SetAnime32(0,56+32,2,16,ANM_DEG);
			SetAnime32(0,56+64,3,16,ANM_DEG);
			SetAnime32(0,56+96,4,16,ANM_DEG);

			// ボス //
			Anime[5].width  = 72;
			Anime[5].height = 64;
			Anime[5].n      = 1;
			Anime[5].mode   = ANM_NORM;
			SetRect(Anime[5].ptn+0,0,184,72,248);

			// 中ボスフラッシュ用 //
			Anime[6].width  = 72;
			Anime[6].height = 56;
			Anime[6].n      = 2;
			Anime[6].mode   = ANM_NORM;
			SetRect(Anime[6].ptn+0,72*2,0,72*3,56);
			SetRect(Anime[6].ptn+1,72*3,0,72*4,56);

			// ボスフラッシュ //
			Anime[7].width  = 72;
			Anime[7].height = 64;
			Anime[7].n      = 1;
			Anime[7].mode   = ANM_NORM;
			SetRect(Anime[7].ptn+0,72,184,72*2,248);
		break;

		case(2):		// Ｓｔａｇｅ２のグラフィック矩形
			SetAnime32(0,  0,0,16,ANM_DEG);
			SetAnime32(0, 32,1,16,ANM_DEG);
			SetAnime32(0, 64,2,16,ANM_DEG);
			SetAnime32(0, 96,3,16,ANM_DEG);
			SetAnime32(0,128,4,16,ANM_DEG);

			Anime[5].width  = 112;
			Anime[5].height = 48;
			Anime[5].n      = 1;
			Anime[5].mode   = ANM_NORM;
			SetRect(Anime[5].ptn+0,0,160,112,208);

			Anime[6].width  = 64;
			Anime[6].height = 48;
			Anime[6].n      = 1;
			Anime[6].mode   = ANM_NORM;
			SetRect(Anime[6].ptn+0,112,160,176,208);

			// 中ボス //
			Anime[7].width  = 64;
			Anime[7].height = 64;
			Anime[7].n      = 1;
			Anime[7].mode   = ANM_NORM;
			SetRect(Anime[7].ptn+0,0,208,64,272);

			// ボス羽 //
			Anime[8].width  = 112;
			Anime[8].height = 48;
			Anime[8].n      = 1;
			Anime[8].mode   = ANM_NORM;
			SetRect(Anime[8].ptn+0,176,160,288,208);

			// ボス丸 //
			Anime[9].width  = 64;
			Anime[9].height = 48;
			Anime[9].n      = 1;
			Anime[9].mode   = ANM_NORM;
			SetRect(Anime[9].ptn+0,288,160,352,208);

			// ボスフラッシュ１ //
			Anime[10].width  = 112;
			Anime[10].height = 48;
			Anime[10].n      = 1;
			Anime[10].mode   = ANM_NORM;
			SetRect(Anime[10].ptn+0,176,160+48,288,208+48);

			// ボスフラッシュ２ //
			Anime[11].width  = 64;
			Anime[11].height = 48;
			Anime[11].n      = 1;
			Anime[11].mode   = ANM_NORM;
			SetRect(Anime[11].ptn+0,288,160+48,352,208+48);

			// 中ボスフラッシュ //
			Anime[12].width  = 64;
			Anime[12].height = 64;
			Anime[12].n      = 1;
			Anime[12].mode   = ANM_NORM;
			SetRect(Anime[12].ptn+0,0+64,208,64+64,272);

/*
			// 羽モノ Left-I //
			Anime[10].width  = 104;
			Anime[10].height = 72;
			Anime[10].n      = 1;
			Anime[10].mode   = ANM_NORM;
			SetRect(Anime[10].ptn+0,184,208,288,280);

			// 羽モノ Right-I //
			Anime[11].width  = 104;
			Anime[11].height = 72;
			Anime[11].n      = 1;
			Anime[11].mode   = ANM_NORM;
			SetRect(Anime[11].ptn+0,288,208,392,280);

			// 羽モノ Left-0 //
			Anime[12].width  = 88;
			Anime[12].height = 80;
			Anime[12].n      = 1;
			Anime[12].mode   = ANM_NORM;
			SetRect(Anime[12].ptn+0,200,280,288,360);

			// 羽モノ Right-0 //
			Anime[13].width  = 88;
			Anime[13].height = 80;
			Anime[13].n      = 1;
			Anime[13].mode   = ANM_NORM;
			SetRect(Anime[13].ptn+0,288,280,376,360);
*/
			SetAnimeRect2(Anime+ 14,  0, 288, 159, 479);	// 雲
			SetAnimeRect2(Anime+ 15,160, 384, 271, 479);	//
			SetAnimeRect2(Anime+ 16,272, 368, 390, 478);	//
			SetAnimeRect2(Anime+ 17,400, 368, 496, 431);	//
			SetAnimeRect2(Anime+ 18,400, 160, 558, 359);	//
			SetAnimeRect2(Anime+ 19,528,  48, 639, 160);	//
			SetAnimeRect2(Anime+ 20,560, 160, 639, 270);	//
			SetAnimeRect2(Anime+ 21,576, 320, 639, 399);	//
		break;

		case(3):		// ゲイツ殿のステージ
			Anime[0].width  = 56;
			Anime[0].height = 56;
			Anime[0].n      = 16;
			Anime[0].mode   = ANM_DEG;
			for(i=0;i<8;i++) BltSetRect(Anime[0].ptn+i  ,i*56, 0,56,56);
			for(i=0;i<8;i++) BltSetRect(Anime[0].ptn+i+8,i*56,56,56,56);

			SetAnime32(0,112,1,16,ANM_DEG);
			SetAnime32(0,144,2,16,ANM_DEG);
			SetAnime32(0,176,3,16,ANM_DEG);

			Anime[4].width  = 48;
			Anime[4].height = 16;
			Anime[4].n      = 2;
			Anime[4].mode   = ANM_NORM;
			BltSetRect(Anime[4].ptn+0,592, 0,48,16);
			BltSetRect(Anime[4].ptn+1,592,16,48,16);

			Anime[5].width  = 48;
			Anime[5].height = 16;
			Anime[5].n      = 2;
			Anime[5].mode   = ANM_NORM;
			BltSetRect(Anime[5].ptn+0,592,32,48,16);
			BltSetRect(Anime[5].ptn+1,592,48,48,16);

			// ボス (464,384)
			Anime[6].width  = 11*16;
			Anime[6].height =  5*16 + 8;
			Anime[6].n      = 1;
			Anime[6].mode   = ANM_NORM;
			BltSetRect(Anime[6].ptn+0,464,392,11*16,5*16+8);

			SetAnime32(0,208,7,16,ANM_DEG);
			SetAnime40(0,240,8);

			// ボスの影 //
			Anime[10].width  = 196;
			Anime[10].height = 100;
			Anime[10].n      = 1;
			Anime[10].mode   = ANM_NORM;
			SetRect(Anime[10].ptn+0, 444, 292, 640, 392);

			// ボスフラッシュ
			Anime[9].width  = 128;
			Anime[9].height = 76;
			Anime[9].n      = 1;
			Anime[9].mode   = ANM_NORM;
			SetRect(Anime[9].ptn+0, 512, 164, 640, 240);
		/*	Anime[9].width  = 11*16;
			Anime[9].height =  5*16 + 8;
			Anime[9].n      = 1;
			Anime[9].mode   = ANM_NORM;
			BltSetRect(Anime[9].ptn+0, 464, 392-88, 11*16, 5*16+8);
		*/
		break;

		case(4):	// マリーさんのステージ
			SetAnime32(0,  0,0,16,ANM_DEG);
			SetAnime32(0, 32,1,16,ANM_DEG);
			SetAnime32(0, 64,2,16,ANM_DEG);
			SetAnime32(0, 96,3, 2,ANM_NORM);
			SetAnime24(64,96,4);
			SetAnime32(0,128,5,16,ANM_DEG);

			//(304,296)-(640,480)
			Anime[6].width  = (640-304);
			Anime[6].height = (480-296);
			Anime[6].n      = 1;
			Anime[6].mode   = ANM_NORM;
			SetRect(Anime[6].ptn+0,304,296,640,480);

			// ボスのフラッシュ //
			Anime[7].width  = (640-304-32);		// ここに注意
			Anime[7].height = (480-296);
			Anime[7].n      = 1;
			Anime[7].mode   = ANM_NORM;
			SetRect(Anime[7].ptn+0,0,296,304,480);
		break;

		case(5):	// ご主人様のステージ
			SetAnime32(  0,   0, 0, 16, ANM_DEG);		// 赤いヤツ
			SetAnime32(  0,  32, 1, 16, ANM_DEG);		// 赤いヤツの出現用演出
			SetAnime32(  0,  64, 2, 16, ANM_DEG);		// 蒼いヤツ
			SetAnime32(  0,  96, 3, 16, ANM_DEG);		// 緑のヤツ
			SetAnime32(  0, 128, 4, 16, ANM_DEG);		// オレンジなやつ
			SetAnime32(512,   0, 5,  4, ANM_NORM);		// 原子炉搭載ビット
			SetAnime32(512,  64, 6,  4, ANM_NORM);		// オレンジなヤツの出現用演出

			// 中ボス用のオプション //
			Anime[7].width  = 24;
			Anime[7].height = 24;
			Anime[7].n      = 4;
			Anime[7].mode   = ANM_NORM;
			BltSetRect(Anime[7].ptn+0, 592, 96+ 0, 24, 24);
			BltSetRect(Anime[7].ptn+1, 592, 96+24, 24, 24);
			BltSetRect(Anime[7].ptn+2, 592, 96+ 0, 24, 24);
			BltSetRect(Anime[7].ptn+3, 592, 96+48, 24, 24);

			SetAnimeRect(Anime+8, 512, 96, 80, 9*8);	// 屈強なる中ボス

			// メタリックご主人 //
			Anime[9].width  = (640-304);
			Anime[9].height = (480-256);
			Anime[9].n      = 1;
			Anime[9].mode   = ANM_NORM;
			SetRect(Anime[9].ptn+0,304,256,640,480);
		break;

		case(6):
			// ラスボス(座り->立ち) //
			Anime[0].width  = 56;
			Anime[0].height = 72;
			Anime[0].n      = 6;
			Anime[0].mode   = ANM_STOP;
			BltSetRect(Anime[0].ptn+0, 56*0,72,56,72);
			BltSetRect(Anime[0].ptn+1, 56*1,72,56,72);
			BltSetRect(Anime[0].ptn+2, 56*2,72,56,72);
			BltSetRect(Anime[0].ptn+3, 56*3,72,56,72);
			BltSetRect(Anime[0].ptn+4, 56*4,72,56,72);
			BltSetRect(Anime[0].ptn+5, 56*5,72,56,72);

			// ラスボス(立ち->座り) //
			Anime[1].width  = 56;
			Anime[1].height = 72;
			Anime[1].n      = 6;
			Anime[1].mode   = ANM_STOP;
			BltSetRect(Anime[1].ptn+0, 56*5,72,56,72);
			BltSetRect(Anime[1].ptn+1, 56*4,72,56,72);
			BltSetRect(Anime[1].ptn+2, 56*3,72,56,72);
			BltSetRect(Anime[1].ptn+3, 56*2,72,56,72);
			BltSetRect(Anime[1].ptn+4, 56*1,72,56,72);
			BltSetRect(Anime[1].ptn+5, 56*0,72,56,72);

			// ラスボス(ガード) //
			Anime[2].width  = 56;
			Anime[2].height = 72;
			Anime[2].n      = 4;
			Anime[2].mode   = ANM_NORM;
			BltSetRect(Anime[2].ptn+0, 56*6,72,56,72);
			BltSetRect(Anime[2].ptn+1, 56*7,72,56,72);
			BltSetRect(Anime[2].ptn+2, 56*6,72,56,72);
			BltSetRect(Anime[2].ptn+3, 56*8,72,56,72);

			// ラスボス(攻撃-壱) //
			Anime[3].width  = 56;
			Anime[3].height = 72;
			Anime[3].n      = 9+1;
			Anime[3].mode   = ANM_STOP;
			BltSetRect(Anime[3].ptn+0, 56*0,0,56,72);
			BltSetRect(Anime[3].ptn+1, 56*1,0,56,72);
			BltSetRect(Anime[3].ptn+2, 56*2,0,56,72);
			BltSetRect(Anime[3].ptn+3, 56*3,0,56,72);
			BltSetRect(Anime[3].ptn+4, 56*4,0,56,72);
			BltSetRect(Anime[3].ptn+5, 56*5,0,56,72);
			BltSetRect(Anime[3].ptn+6, 56*6,0,56,72);
			BltSetRect(Anime[3].ptn+7, 56*7,0,56,72);
			BltSetRect(Anime[3].ptn+8, 56*8,0,56,72);
			BltSetRect(Anime[3].ptn+9, 56*5,72,56,72);	// ちょっと追加

			// 幼虫期(//
			SetAnimeRect2(Anime+4, 432,272,632,464);

			// ラスボス(ジャンプに見えるといいなぁ) //
			Anime[5].width  = 56;
			Anime[5].height = 72;
			Anime[5].n      = 11;
			Anime[5].mode   = ANM_STOP;
			BltSetRect(Anime[5].ptn+0,  56*0,72,56,72);
			BltSetRect(Anime[5].ptn+1,  56*1,72,56,72);
			BltSetRect(Anime[5].ptn+2,  56*2,72,56,72);
			BltSetRect(Anime[5].ptn+3,  56*3,72,56,72);
			BltSetRect(Anime[5].ptn+4,  56*4,72,56,72);
			BltSetRect(Anime[5].ptn+5,  56*5,72,56,72);
			BltSetRect(Anime[5].ptn+6,  56*4,72,56,72);
			BltSetRect(Anime[5].ptn+7,  56*3,72,56,72);
			BltSetRect(Anime[5].ptn+8,  56*2,72,56,72);
			BltSetRect(Anime[5].ptn+9,  56*1,72,56,72);
			BltSetRect(Anime[5].ptn+10, 56*0,72,56,72);


			// 蝶状態で放つビット？(Open) //
			Anime[6].width  = 33;
			Anime[6].height = 32;
			Anime[6].n      = 10;
			Anime[6].mode   = ANM_STOP;
			BltSetRect(Anime[6].ptn+0, 32*0,416,32,32);
			BltSetRect(Anime[6].ptn+1, 32*1,416,32,32);
			BltSetRect(Anime[6].ptn+2, 32*2,416,32,32);
			BltSetRect(Anime[6].ptn+3, 32*3,416,32,32);
			BltSetRect(Anime[6].ptn+4, 32*4,416,32,32);
			BltSetRect(Anime[6].ptn+5, 32*5,416,32,32);
			BltSetRect(Anime[6].ptn+6, 32*0,448,32,32);
			BltSetRect(Anime[6].ptn+7, 32*1,448,32,32);
			BltSetRect(Anime[6].ptn+8, 32*2,448,32,32);
			BltSetRect(Anime[6].ptn+9, 32*3,448,32,32);

			// 蝶状態で放つビット？(Close) //
			Anime[7].width  = 33;
			Anime[7].height = 32;
			Anime[7].n      = 10;
			Anime[7].mode   = ANM_STOP;
			BltSetRect(Anime[7].ptn+0, 32*3,448,32,32);
			BltSetRect(Anime[7].ptn+1, 32*2,448,32,32);
			BltSetRect(Anime[7].ptn+2, 32*1,448,32,32);
			BltSetRect(Anime[7].ptn+3, 32*0,448,32,32);
			BltSetRect(Anime[7].ptn+4, 32*5,416,32,32);
			BltSetRect(Anime[7].ptn+5, 32*4,416,32,32);
			BltSetRect(Anime[7].ptn+6, 32*3,416,32,32);
			BltSetRect(Anime[7].ptn+7, 32*2,416,32,32);
			BltSetRect(Anime[7].ptn+8, 32*1,416,32,32);
			BltSetRect(Anime[7].ptn+9, 32*0,416,32,32);

			SetAnimeRect(Anime+8, 0, 368, 48, 48);	// 屈強なる中ボス
		break;
	}

	return TRUE;
}

// １パターンのグラフィックをアニメとして定義する //
static FVOID SetAnimeRect(ANIME_DATA *anm,int x, int y, int w, int h)
{
	anm->width  = w;
	anm->height = h;
	anm->n      = 1;
	anm->mode   = ANM_NORM;

	BltSetRect(anm->ptn+0, x, y, w, h);
}

// １パターンのグラフィックをアニメとして定義する //
static FVOID SetAnimeRect2(ANIME_DATA *anm,int x1, int y1, int x2, int y2)
{
	anm->width  = x2-x1;
	anm->height = y2-y1;
	anm->n      = 1;
	anm->mode   = ANM_NORM;

	SetRect(anm->ptn+0, x1, y1, x2, y2);
}

static FVOID SetAnimeTH(ANIME_DATA *anm, int x, int y, int n)
{
	int		i;

	anm->width  = 80;
	anm->height = 80;
	anm->n      = n;
	anm->mode   = ANM_NORM;

	for(i=0; i<n; i++){
		BltSetRect(anm->ptn+i, x+i*80, y, 80, 80);
	}
}


// ｎ番目の曲をロードする //
extern BOOL LoadMusic(int no)
{
	BIT_DEVICE *in;
	BOOL flag;

	in = FilStartR("MUSIC.DAT",BD_FILE_READ);
	if(in==NULL) return FALSE;
	flag = PMid_Load(in,no);
	MusicNum = FileHead.n;
	FilEnd(in);

	return flag;
}

// 全てのSoundデータをロードする //
BOOL LoadSound(void)
{
	BIT_DEVICE *in;

	in = FilStartR("SOUND.DAT",BD_FILE_READ);
	if(in==NULL) return FALSE;

	while(1){
		if(!SndWAVLoadP(in,SOUND_ID_KEBARI  ,SOUND_ID_KEBARI  ,SNDMAX_KEBARI  )) break;
		if(!SndWAVLoadP(in,SOUND_ID_TAME    ,SOUND_ID_TAME    ,SNDMAX_TAME    )) break;
		if(!SndWAVLoadP(in,SOUND_ID_LASER   ,SOUND_ID_LASER   ,SNDMAX_LASER   )) break;
		if(!SndWAVLoadP(in,SOUND_ID_LASER2  ,SOUND_ID_LASER2  ,SNDMAX_LASER2  )) break;
		if(!SndWAVLoadP(in,SOUND_ID_BOMB    ,SOUND_ID_BOMB    ,SNDMAX_BOMB    )) break;
		if(!SndWAVLoadP(in,SOUND_ID_SELECT  ,SOUND_ID_SELECT  ,SNDMAX_SELECT  )) break;
		if(!SndWAVLoadP(in,SOUND_ID_HIT     ,SOUND_ID_HIT     ,SNDMAX_HIT     )) break;
		if(!SndWAVLoadP(in,SOUND_ID_CANCEL  ,SOUND_ID_CANCEL  ,SNDMAX_CANCEL  )) break;
		if(!SndWAVLoadP(in,SOUND_ID_WARNING ,SOUND_ID_WARNING ,SNDMAX_WARNING )) break;
		if(!SndWAVLoadP(in,SOUND_ID_SBLASER ,SOUND_ID_SBLASER ,SNDMAX_SBLASER )) break;
		if(!SndWAVLoadP(in,SOUND_ID_BUZZ    ,SOUND_ID_BUZZ    ,SNDMAX_BUZZ))     break;
		if(!SndWAVLoadP(in,SOUND_ID_MISSILE ,SOUND_ID_MISSILE ,SNDMAX_MISSILE )) break;
		if(!SndWAVLoadP(in,SOUND_ID_JOINT   ,SOUND_ID_JOINT   ,SNDMAX_JOINT   )) break;
		if(!SndWAVLoadP(in,SOUND_ID_DEAD    ,SOUND_ID_DEAD    ,SNDMAX_DEAD    )) break;
		if(!SndWAVLoadP(in,SOUND_ID_SBBOMB  ,SOUND_ID_SBBOMB  ,SNDMAX_SBBOMB  )) break;
		if(!SndWAVLoadP(in,SOUND_ID_BOSSBOMB,SOUND_ID_BOSSBOMB,SNDMAX_BOSSBOMB)) break;
		if(!SndWAVLoadP(in,SOUND_ID_ENEMYSHOT,SOUND_ID_ENEMYSHOT,SNDMAX_ENEMYSHOT)) break;
		if(!SndWAVLoadP(in,SOUND_ID_HLASER, SOUND_ID_HLASER, SNDMAX_HLASER))	break;
		if(!SndWAVLoadP(in,SOUND_ID_TAMEFAST, SOUND_ID_TAMEFAST, SNDMAX_TAMEFAST))	break;
		if(!SndWAVLoadP(in,SOUND_ID_WARP, SOUND_ID_WARP, SNDMAX_WARP))	break;

		FilEnd(in);
		return TRUE;
	}

	FilEnd(in);
	return FALSE;
}


// MusicRoom のコメントをロードする //
extern BOOL LoadMusicRoomComment(int no)
{
	BIT_DEVICE		*in;
	int				i, j;
	BYTE			*temp;

	if(no < 0 || no > 19) return FALSE;

	if((in=FilStartR("ENEMY.DAT",BD_FILE_READ))==NULL) return FALSE;
	if((temp=MemExpand(in, 27+no))==NULL) return FALSE;
	FilEnd(in);

	for(i=0; i<19*2; i++){
		MTitleBuffer[i] = (BYTE)temp[i];
	}
	MTitleBuffer[19*2] = '\0';

	for(i=0; i<20; i++){
		for(j=0; j<19*2; j++){
			MCommentBuffer[i][j] = (BYTE)temp[(i+1)*19*2 + j];
		}
		MCommentBuffer[i][19*2] = '\0';
	}

	LocalFree(temp);

	return TRUE;
}


/*
// ボム用パレット属性に変更する //
extern void EnterBombPalette(void)
{
	LPDIRECTDRAWPALETTE		lpdp;

	bIsBombPalette = TRUE;

	if(DxObj.Bpp != 8) return;
	if(!GrBomber) return;
	if(GrBomber->GetPalette(&lpdp) != DD_OK) return;

	// 現在のパレットを記憶する //
	GrpGetPalette(tempPalette);

	lpdp->GetEntries(0,0,256,DxObj.pe);
	GrpSetPalette(DxObj.pe);

	lpdp->Release();
}


// ボム用パレット属性を外す
extern void LeaveBombPalette(void)
{
	if(!bIsBombPalette) return;

	bIsBombPalette = FALSE;

	GrpSetPalette(tempPalette);
}
*/

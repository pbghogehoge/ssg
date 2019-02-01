/*
 *   Ending.cpp   : エンディングの処理
 *
 */

#include "Ending.h"


typedef struct tagEndingGrpInfo {
	DWORD		timer;		// 表示用タイマー
	DWORD		fadein;		// フェードイン時刻
	DWORD		fadeout;	// フェードアウト時刻
	ENDING_GRP	*target;	// 描画対象
	short		alpha;		// パレットの状態

	int			x, y;		// 表示する左上
	BOOL		bWantDisp;	// 表示するかどうか
} EndingGrpInfo;

// スタッフ描画タスク //
typedef struct tagEndingStTask{
	DWORD		timer;		// 表示用タイマー
	DWORD		fadein;		// フェードイン時刻
	DWORD		fadeout;	// フェードアウト時刻

	DWORD		NumStf;		// 全スタッフ数
	DWORD		StfID[10];	// スタッフＩＤ
	DWORD		TitleID;	// タイトル、すなわち役職ＩＤ

	short		alpha;		// パレットの状態

	int			ox, oy;		// 表示基準座標

	BOOL		bWantDisp;	// 表示するのかな
} EndingStTask;

typedef struct tagEndingText {
	char	*Text[10];		// 表示するテキストへのポインタ
	int		NumText;		// 現在格納されているテキストの数
} EndingText;


EndingGrpInfo	EGrpInfo;
EndingStTask	EStfTask;
EndingText		EText;

// フォント情報格納用ね //
ExtraFontInfo	*pFontInfo[6];
ExtraFontInfo	*pMember[7];

// フラッシュの状態 //
DWORD FlashState = 0;


// グラフィックのフェードアウト用関数 //
void FadeoutPaletteGrp(PALETTEENTRY *Dest, PALETTEENTRY *Src, BYTE a);

// スタッフ名のフェードアウト用関数 //
void FadeoutPaletteStf(PALETTEENTRY *Dest, PALETTEENTRY *Src, BYTE a);

void UpdateGrpInfo();		// グラフィックの更新(内部データ)
void UpdateStfInfo();		// スタッフの更新(内部データ)

void DrawGrpInfo();		// グラフィックの描画
void DrawStfInfo();		// スタッフの描画
void DrawTextInfo();	// テキストの描画

void DrawFadeInfo();	// フェードＩＯ情報の反映

FVOID EndingCleanup();	// おしまい処理

FVOID EndingSCLDecode();	// エンディング用 SCL のデコード



// エンディングまわりの初期化 //
FBOOL EndingInit(void)
{
	PALETTEENTRY	pal[256];

	RECT	src[7] = {
		{0, 0,160, 24}, {0, 24,104, 48}, {0, 48,160, 72},
		{0,72,232, 96}, {0, 96,168,120}, {0,144,104,168},
		{0, 480-32, 9*32, 480},
	};

	RECT	src2[7] = {
		{0, 168, 72, 192}, {96, 168, 168, 192}, {192, 168, 264, 192}, {288, 168, 360, 192},
		{0, 192, 144, 216}, {168, 192, 320, 216},
		{0, 216, 336, 264}
	};

	int		i;

	GrpSetClip(0, 0, 640, 480);
	GrpCls();
	GrpFlip();
	GrpCls();

	if(!LoadGraph(GRAPH_ID_ENDING)) return FALSE;
	if(!LoadStageData(GRAPH_ID_ENDING)) return FALSE;
	Mid_Stop();


	GrpGetPalette(pal);
	pal[255].peRed   = 0;
	pal[255].peGreen = 0;
	pal[255].peBlue  = 0;
	pal[199].peRed   = 255;
	pal[199].peGreen = 255;
	pal[199].peBlue  = 255;
	pal[198].peRed   = 128;
	pal[198].peGreen = 128;
	pal[198].peBlue  = 128;
	GrpSetPalette(pal);


	GameMain = EndingProc;

	FlashState = 0;

	EGrpInfo.bWantDisp = FALSE;
	EStfTask.bWantDisp = FALSE;

	EText.NumText = 0;

	for(i=0; i<7; i++){
		pFontInfo[i] = CreateExtraFont(GrMap, &src[i]);
		if(pFontInfo[i] == NULL) return FALSE;
	}

	for(i=0; i<7; i++){
		pMember[i] = CreateExtraFont(GrMap, &src2[i]);
		if(pMember[i] == NULL) return FALSE;
	}

	return TRUE;
}


// おしまい処理 //
FVOID EndingCleanup()
{
	int		i;

	for(i=0; i<6; i++){
		DeleteExtraFont(pFontInfo[i]);
	}

	for(i=0; i<7; i++){
		DeleteExtraFont(pMember[i]);
	}
}


// エンディング状態推移用プロシージャ //
FVOID EndingProc(void)
{/*
	Key_Read();

	if(Key_Data){
		EndingCleanup();
		GameExit();
		return;
	}
*/
	extern FBOOL IsDraw();

	if(FlashState) FlashState -= 32;

	EndingSCLDecode();
	if(GameMain != EndingProc) return;

	if(IsDraw()){
		UpdateGrpInfo();
		UpdateStfInfo();
		EndingDraw();
	}
}


// エンディング時の描画処理 //
FVOID EndingDraw(void)
{
	// 画面消去 //
	if(DxObj.Bpp == 8) GrpClsEx(NULL, 255);
	else               GrpClsEx(NULL, RGB(0,0,0));

	// それぞれのグラフィックを描画するで //
	DrawGrpInfo();
	DrawStfInfo();
	DrawTextInfo();

	// フェード情報の反映ぢゃ //
	DrawFadeInfo();

	GrpFlip();
}


// グラフィックのフェードアウト用関数 //
void FadeoutPaletteGrp(PALETTEENTRY *Dest, PALETTEENTRY *Src, BYTE a)
{
	int				i;

	for(i=0; i<200; i++){
		Dest[i].peRed   = ((WORD)Src[i].peRed   * a) / 255;
		Dest[i].peGreen = ((WORD)Src[i].peGreen * a) / 255;
		Dest[i].peBlue  = ((WORD)Src[i].peBlue  * a) / 255;
	}
	for(i=200; i<255; i++){
		Dest[i] = Src[i];
	}

	Dest[255].peRed   = 0;
	Dest[255].peGreen = 0;
	Dest[255].peBlue  = 0;

	Dest[199].peRed   = 255;
	Dest[199].peGreen = 255;
	Dest[199].peBlue  = 255;

	Dest[198].peRed   = 128;
	Dest[198].peGreen = 128;
	Dest[198].peBlue  = 128;
}


// スタッフ名のフェードアウト用関数 //
void FadeoutPaletteStf(PALETTEENTRY *Dest, PALETTEENTRY *Src, BYTE a)
{
	int				i;

	// 注意：最終パレットは密かに？へんこうしないど //
	for(i=0; i<200; i++){
		Dest[i] = Src[i];
	}
	for(i=200; i<255; i++){
		Dest[i].peRed   = ((WORD)Src[i].peRed   * a) / 255;
		Dest[i].peGreen = ((WORD)Src[i].peGreen * a) / 255;
		Dest[i].peBlue  = ((WORD)Src[i].peBlue  * a) / 255;
	}

	Dest[255].peRed   = 0;
	Dest[255].peGreen = 0;
	Dest[255].peBlue  = 0;

	Dest[199].peRed   = 255;
	Dest[199].peGreen = 255;
	Dest[199].peBlue  = 255;

	Dest[198].peRed   = 128;
	Dest[198].peGreen = 128;
	Dest[198].peBlue  = 128;
}


// グラフィックの更新(内部データ) //
void UpdateGrpInfo()
{
	EGrpInfo.timer++;
	if(EGrpInfo.timer > EGrpInfo.fadeout){
		if(EGrpInfo.alpha-3 > 0) EGrpInfo.alpha-= 3;
		else                     EGrpInfo.alpha = 0;
	}
	else if(EGrpInfo.timer > EGrpInfo.fadein){
		if(EGrpInfo.alpha+3 < 255) EGrpInfo.alpha+= 3;
		else                       EGrpInfo.alpha = 255;
	}

	if(EGrpInfo.bWantDisp && EGrpInfo.alpha==0)
		EGrpInfo.bWantDisp = FALSE;
}


// スタッフの更新(内部データ)
void UpdateStfInfo()
{
	EStfTask.timer++;
	if(EStfTask.timer > EStfTask.fadeout){
		if(EStfTask.alpha-3 > 0) EStfTask.alpha-= 3;
		else                     EStfTask.alpha = 0;
	}
	else if(EStfTask.timer > EStfTask.fadein){
		if(EStfTask.alpha+3 < 255) EStfTask.alpha+= 3;
		else                       EStfTask.alpha = 255;
	}

	if(EStfTask.bWantDisp && EStfTask.alpha==0)
		EStfTask.bWantDisp = FALSE;
}


// グラフィックの描画 //
void DrawGrpInfo()
{
	RECT			src;

	if(!EGrpInfo.bWantDisp) return;

	// 驚異の画像表示 //
	src = EGrpInfo.target->rcTarget;
	GrpBltXC(&src, EGrpInfo.x, EGrpInfo.y, EGrpInfo.target->GrSurf);
}


// スタッフの描画 //
void DrawStfInfo()
{
	int		i;

	if(!EStfTask.bWantDisp) return;

	DrawExtraFont(pFontInfo[EStfTask.TitleID],
		EStfTask.ox, EStfTask.oy, 0);//255-EStfTask.alpha);

	for(i=0; i<EStfTask.NumStf; i++){
		DrawExtraFont(pMember[EStfTask.StfID[i]],
			EStfTask.ox, EStfTask.oy+i*30+50, 0);//255-EStfTask.alpha);
	}
}


// テキストの描画 //
void DrawTextInfo()
{
	int			i, max = 0;
	int			dx;
	HDC			hdc;

	for(i=0; i<EText.NumText; i++){
		if(max < strlen(EText.Text[i])) max = strlen(EText.Text[i]);
	}

	dx = 8 * (39-max/2);

	if(DxObj.Back->GetDC(&hdc) == DD_OK){
		SetBkMode(hdc, TRANSPARENT);

		SetTextColor(hdc, RGB(128, 128, 128));
		for(i=0; i<EText.NumText; i++){
			TextOut(hdc, dx+21, 360-10+i*25, EText.Text[i], strlen(EText.Text[i]));
			TextOut(hdc, dx+19, 360-10+i*25, EText.Text[i], strlen(EText.Text[i]));
			TextOut(hdc, dx+20, 359-10+i*25, EText.Text[i], strlen(EText.Text[i]));
			TextOut(hdc, dx+20, 361-10+i*25, EText.Text[i], strlen(EText.Text[i]));
		}

		SetTextColor(hdc, RGB(255, 255, 255));
		for(i=0; i<EText.NumText; i++){
			TextOut(hdc, dx+20, 360-10+i*25, EText.Text[i], strlen(EText.Text[i]));
		}

		DxObj.Back->ReleaseDC(hdc);
	}
}


void FlashPaletteGrp(PALETTEENTRY *dest, PALETTEENTRY *pal, DWORD a)
{
	int		i;

	if(a > 256) a = a - 256;
	for(i=0; i<256; i++){
		dest[i].peRed   = min(256, (256*(256-a) + (WORD)pal[i].peRed   * a) / 256);
		dest[i].peGreen = min(256, (256*(256-a) + (WORD)pal[i].peGreen * a) / 256);
		dest[i].peBlue  = min(256, (256*(256-a) + (WORD)pal[i].peBlue  * a) / 256);
	}
}


// フェードＩＯ情報の反映 //
void DrawFadeInfo()
{
	PALETTEENTRY	temp_pal[256];
	WORD			a;

	// フェードアウト関連
	if(DxObj.Bpp == 8){
		if(FlashState){
			FlashPaletteGrp(temp_pal, EGrpInfo.target->pal, FlashState);
			GrpSetPalette(temp_pal);
		}
		else if(EGrpInfo.target){
			FadeoutPaletteGrp(temp_pal, EGrpInfo.target->pal, (BYTE)EGrpInfo.alpha);
			FadeoutPaletteStf(temp_pal, temp_pal, (BYTE)EStfTask.alpha);
			GrpSetPalette(temp_pal);
		}
		else{
			memset(temp_pal, 0, sizeof(temp_pal));
			temp_pal[199].peRed   = 255;
			temp_pal[199].peGreen = 255;
			temp_pal[199].peBlue  = 255;
			temp_pal[198].peRed   = 128;
			temp_pal[198].peGreen = 128;
			temp_pal[198].peBlue  = 128;
			GrpSetPalette(temp_pal);
		}
	}
	else{
		GrpLock();

		if(EGrpInfo.bWantDisp){
			GrpSetAlpha(255-EGrpInfo.alpha, ALPHA_NORM);
			GrpSetColor(0, 0, 0);
			GrpBoxA(EGrpInfo.x, EGrpInfo.y, EGrpInfo.x+320, EGrpInfo.y+240);
		}
		if(EStfTask.bWantDisp){
			GrpSetAlpha(255-EStfTask.alpha, ALPHA_NORM);
			GrpSetColor(0, 0, 0);
			if(EStfTask.ox == 320)     GrpBoxA(0, 0, 640,480);
			else if(EStfTask.ox > 320) GrpBoxA(320,0,640,300);
			else                       GrpBoxA(0,0, 320-50,300);
		}
		if(FlashState){
			if(FlashState > 256) a = FlashState - 256;
			else                 a = FlashState;
			GrpSetAlpha(255-a, ALPHA_NORM);
			GrpSetColor(5, 5, 5);
			GrpBoxA(0,0,640,480);
		}

		GrpUnlock();
	}
}



// エンディング用 SCL のデコード //
FVOID EndingSCLDecode()
{
	BOOL		bFlag = TRUE;
	BYTE		*cmd;
	DWORD		temp;

	while(bFlag){
		cmd = SCL_Now;
		switch(cmd[0]){
			case(SCL_TIME):
				temp = (*(DWORD *)(&cmd[1]));
				if(temp>GameCount) bFlag    = FALSE;
				else               SCL_Now += 5;		// cmd(1)+time(4)
			break;

			case(SCL_MSG):		// メッセージを出力する
				EText.Text[EText.NumText++] = (char *)cmd+1;
				SCL_Now += (strlen((char *)cmd+1)+2);
			break;

			case(SCL_FACE):		// 顔を表示する
				switch(cmd[1]){
				case 0:
					EGrpInfo.fadein  = 0;
					EGrpInfo.fadeout = 128+64+64+512;
					EGrpInfo.x       = 640-40-320;
					EGrpInfo.y       = 40;
				break;

				case 1:case 2: case 3:
					EGrpInfo.fadein  = 0;
					EGrpInfo.fadeout = 128+64;
					EGrpInfo.x       = 320-160;
					EGrpInfo.y       = 40;
				break;

				case 5:
					EGrpInfo.fadein  = 0;
					EGrpInfo.fadeout = 128+64+64+(512+512)*2;
					EGrpInfo.x       = 40;
					EGrpInfo.y       = 40;
				break;

				default:
					EGrpInfo.fadein  = 0;
					EGrpInfo.fadeout = 128+64+64+512;
					EGrpInfo.x       = 40;
					EGrpInfo.y       = 40;
				break;
				}
				EGrpInfo.alpha   = 0;
				EGrpInfo.target  = EndingGrp + cmd[1];
				EGrpInfo.timer   = 0;
				EGrpInfo.bWantDisp = TRUE;
				SCL_Now += 2;
			break;

			case(SCL_STAFF):	// わかりにくいが、１２８を加えると、役割名指定ね
				if(cmd[1] >= 128){
					switch(cmd[1] - 128){
						case 0:case 4:
							EStfTask.fadein   = 0;
							EStfTask.fadeout  = 128+64+64+128;
							EStfTask.ox       = 320+130;
							EStfTask.oy       = 80+50;
						break;
						case 2:case 5:
							EStfTask.fadein   = 0;
							EStfTask.fadeout  = 128+64+64+128;
							EStfTask.ox       = 320+130;
							EStfTask.oy       = 80;
						break;
						case 1:case 3:
							EStfTask.fadein   = 0;
							EStfTask.fadeout  = 128+64+64+128;
							EStfTask.ox       = 130;
							EStfTask.oy       = 80+50;
						break;
						case 6:
							EStfTask.fadein   = 0;
							EStfTask.fadeout  = 128+64+64;//+64;
							EStfTask.ox       = 320;
							EStfTask.oy       = 80+80;
						break;
					}
					EStfTask.alpha    = 0;
					EStfTask.timer    = 0;
					EStfTask.timer    = 0;
					EStfTask.NumStf   = 0;
					EStfTask.TitleID  = cmd[1]-128;
					EStfTask.bWantDisp = TRUE;
				}
				else{
					EStfTask.StfID[EStfTask.NumStf++] = cmd[1];
				}
				SCL_Now+=2;
			break;

			case(SCL_NPG):		// 新しいページに変更する
				EText.NumText = 0;
				SCL_Now++;
			break;

			case(SCL_END):		// カウントも変更させずにリターンするのだ
				EndingCleanup();
				EGrpInfo.bWantDisp = FALSE;
				EStfTask.bWantDisp = FALSE;
				NameRegistInit(FALSE);
			return;

			case(SCL_MUSIC):
				if(LoadMusic(cmd[1])) Mid_Play();
				SCL_Now += 2;
			break;

			case(SCL_EFC):
				switch(cmd[1]){
					case 0:		FlashState = 256*2;		break;
				}

				SCL_Now += 2;
			break;

			case(SCL_STAGECLEAR):	// ステージクリア
				// ステージクリア処理をここに記述 //
				//GameNextStage();	// 本当はエラーチェックが必要!!
			return;

			case(SCL_GAMECLEAR):
				//if(GameStage == 6) GameStage = 7;
				//NameRegistInit();
			return;

			default:	// 未実装 or ばぐ
			return;
		}
	}

	GameCount++;
}

/*                                                                           */
/*   WINDOWSYS.cpp   コマンドウィンドウ処理                                  */
/*                                                                           */
/*                                                                           */

#include "WindowSys.h"



///// [非公開関数] /////

static WINDOW_INFO *CWinSearchActive(WINDOW_SYSTEM *ws);	// アクティブなウィンドウを探す
static void CWinKeyEvent(WINDOW_SYSTEM *ws);				// キーボード入力を処理する

static void DrawWindowFrame(int x,int y,int w,int h);		// ウィンドウ枠を描画する
static HFONT SetFont(HDC hdc,BYTE FontID);					// フォントをセットする
static void GrpBoxA2(int x1,int y1,int x2,int y2);			// 平行四辺形ＢＯＸ描画



///// [グローバル変数] /////

WIN_GRPINFO		WinGrpInfo;		// ウィンドウに使用するフォント、Surface など
MSG_WINDOW		MsgWindow;		// メッセージウィンドウ



// コマンドウィンドウの初期化 //
void CWinInit(WINDOW_SYSTEM *ws,int x,int y,int select)
{
	ws->x = x;
	ws->y = y;

	ws->Count       = 0;
	ws->Select[0]   = select;
	ws->SelectDepth = 0;
	ws->State       = CWIN_INIT;

	ws->OldKey   = Key_Data;
	ws->KeyCount = CWIN_KEYWAIT;

	ws->FirstWait = TRUE;
}

// コマンドウィンドウを１フレーム動作させる //
void CWinMove(WINDOW_SYSTEM *ws)
{
	switch(ws->State){
		case(CWIN_DEAD):	// 使用されていない
		return;

		case(CWIN_FREE):	// 入力待ち状態
			CWinKeyEvent(ws);
			ws->Count = 0;
		return;

		case(CWIN_OPEN):	// 項目移動中(進む)
		break;

		case(CWIN_CLOSE):	// 項目移動中(戻る)
		break;

		case(CWIN_NEXT):	// 次のウィンドウに移行中
		break;

		case(CWIN_BEFORE):	// 前のウィンドウに移行中
		break;

		case(CWIN_INIT):	// 初期化処理中
			ws->State = CWIN_FREE;
		break;
	}

	ws->Count++;
}

// コマンドウィンドウの描画 //
void CWinDraw(WINDOW_SYSTEM *ws)
{
	WINDOW_INFO		*p;
	int				i;
	HDC				hdc;
	HFONT			oldfont;
	BYTE			alpha;

	// アクティブな項目を検索する //
	p = CWinSearchActive(ws);

	// 半透明ＢＯＸの描画 //
	alpha = (DxObj.Bpp==8) ? 64+32 : 128;

	GrpLock();
	GrpSetAlpha(alpha,ALPHA_NORM);

	GrpSetColor(0,0,0);
	GrpBoxA(ws->x,ws->y,ws->x+140,ws->y+16);

	GrpSetColor(0,0,2);
	for(i=0;i<p->NumItems;i++){
		if(i==ws->Select[ws->SelectDepth]){
			GrpSetAlpha(128,ALPHA_NORM);
			GrpSetColor(5,0,0);
		}
		GrpBoxA(ws->x,ws->y+(i+1)*16,ws->x+140,ws->y+(i+2)*16);
		if(i==ws->Select[ws->SelectDepth]){
			GrpSetAlpha(alpha,ALPHA_NORM);
			GrpSetColor(0,0,2);
		}
	}
	GrpUnlock();

	// 文字列の描画 //
	if(DxObj.Back->GetDC(&hdc)==DD_OK){
		SetBkMode(hdc,TRANSPARENT);
		oldfont = (HFONT)SelectObject(hdc,WinGrpInfo.SmallFont);
		SetTextColor(hdc,RGB(128,128,128));
		TextOut(hdc,ws->x+1,ws->y,p->Title,strlen(p->Title));
		SetTextColor(hdc,RGB(255,255,255));
		TextOut(hdc,ws->x+0,ws->y,p->Title,strlen(p->Title));

		for(i=0;i<p->NumItems;i++){
			SetTextColor(hdc,RGB(128,128,128));
			TextOut(hdc,ws->x+1+8,ws->y+(i+1)*16+1,p->ItemPtr[i]->Title,strlen(p->ItemPtr[i]->Title));
			SetTextColor(hdc,RGB(255,255,255));
			TextOut(hdc,ws->x+0+8,ws->y+(i+1)*16+1,p->ItemPtr[i]->Title,strlen(p->ItemPtr[i]->Title));
		}
		SelectObject(hdc,oldfont);
		DxObj.Back->ReleaseDC(hdc);
	}
}

// コマンド [Exit] のデフォルト処理関数 //
BOOL CWinExitFn(WORD key)
{
	switch(key){
		case(KEY_RETURN):case(KEY_TAMA):case(KEY_BOMB):case(KEY_ESC):
		return FALSE;

		default:
		return TRUE;
	}
}

// メッセージウィンドウをオープンする //
void MWinOpen(RECT *rc)
{
	int			i,y_mid;

	if(MsgWindow.State != MWIN_DEAD) return;

	// 状態および、最終値のセット //
	MsgWindow.State     = MWIN_OPEN;
	MsgWindow.FaceState = MFACE_NONE;			// 何も表示しない
	MsgWindow.FaceID    = 0;
	MsgWindow.FaceTime  = 0;
	MsgWindow.MaxSize = (*rc);

	// 矩形の初期値をセットする //
	y_mid = (MsgWindow.MaxSize.bottom + MsgWindow.MaxSize.top)/2;
	MsgWindow.NowSize.left   = MsgWindow.MaxSize.left;
	MsgWindow.NowSize.right  = MsgWindow.MaxSize.right;
	MsgWindow.NowSize.top    = y_mid - 4;
	MsgWindow.NowSize.bottom = y_mid + 4;

	MWinCmd(MWCMD_NORMALFONT);					// ノーマルフォント

	// ウィンドウ内に表示するものの初期化 //
	MsgWindow.Line = 0;
	for(i=0;i<MSG_HEIGHT;i++) MsgWindow.Msg[i] = NULL;
}

// メッセージウィンドウをクローズする //
void MWinClose(void)
{
//	if(MsgWindow.State != MWIN_FREE) return;

	MsgWindow.FaceState = MFACE_CLOSE;
	MsgWindow.State     = MWIN_CLOSE;
}

// メッセージウィンドウを強制クローズする //
void MWinForceClose(void)
{
	MsgWindow.FaceState = MFACE_NONE;
	MsgWindow.State     = MWIN_DEAD;
}

// メッセージウィンドウを動作させる //
void MWinMove(void)
{
	switch(MsgWindow.FaceState){
		case(MFACE_OPEN):		// 顔を表示しようとしている
			MsgWindow.FaceTime+=16;
			if(MsgWindow.FaceTime==0) MsgWindow.FaceState = MFACE_WAIT;
		break;

		case(MFACE_NEXT):
			MsgWindow.FaceTime+=16;
			if(MsgWindow.FaceTime==0){
				MsgWindow.FaceState = MFACE_OPEN;
				MsgWindow.FaceID    = MsgWindow.NextFace;
				GrpSetPalette(FaceData[MsgWindow.FaceID/FACE_NUMX].pal);
			}
		break;

		case(MFACE_CLOSE):		// 顔を消そうとしている
			MsgWindow.FaceTime+=16;
			if(MsgWindow.FaceTime==0) MsgWindow.FaceState = MFACE_NONE;
		break;

		default:
		break;
	}

	switch(MsgWindow.State){
		case(MWIN_OPEN):
			MsgWindow.NowSize.top    -= 2;
			MsgWindow.NowSize.bottom += 2;

			// 完全にオープンできた場合 //
			if(MsgWindow.NowSize.top <= MsgWindow.MaxSize.top){
				MsgWindow.NowSize = MsgWindow.MaxSize;
				MsgWindow.State   = MWIN_FREE;
			}
		break;

		case(MWIN_CLOSE):
			MsgWindow.NowSize.top    += 3;
			MsgWindow.NowSize.bottom -= 3;
			MsgWindow.NowSize.right  += 6;
			MsgWindow.NowSize.left   -= 6;

			// 完全にクローズできた場合 //
			if(MsgWindow.NowSize.top >= MsgWindow.NowSize.bottom){
				MsgWindow.State   = MWIN_DEAD;
			}
		break;

		case(MWIN_DEAD):	case(MWIN_FREE):
		break;
	}
}

// メッセージウィンドウを描画する(上に同じ) //
void MWinDraw(void)
{
	HFONT	oldfont;
	HDC		hdc;
	BYTE	alpha;
	RECT	src;
	GRP		GrFace;

	int		x = MsgWindow.NowSize.left;		// ウィンドウ左上Ｘ
	int		y = MsgWindow.NowSize.top;		// ウィンドウ左上Ｙ
	int		w = (MsgWindow.NowSize.right  - x);		// ウィンドウ幅
	int		h = (MsgWindow.NowSize.bottom - y);		// ウィンドウ高さ
	int		i,TextX,TextY;
	int		len,time,oy;

	// メッセージウィンドウが死んでいたら何もしない //
	if(MsgWindow.State == MWIN_DEAD) return;

	// 半透明部の描画 //
	GrpLock();
	alpha = (DxObj.Bpp==8) ? 64+32 : 110;
	GrpSetAlpha(alpha,ALPHA_NORM);
	GrpSetColor(0,0,3);//0,0,3
	GrpBoxA(x+4,y+4,x+w-4,y+h-4);
	GrpUnlock();

	// 文字列を表示するのはウィンドウが[FREE]である場合だけ        //
	// -> こうしないと文字列用 Surface を作成することになるので... //
	if(MsgWindow.State == MWIN_FREE){
		if(DxObj.Back->GetDC(&hdc)==DD_OK){
			oldfont = SetFont(hdc,MsgWindow.FontID);	// セットされたフォントで描画
			SetBkMode(hdc,TRANSPARENT);

			for(i=0;i<MsgWindow.Line;i++){
				if(MsgWindow.Msg[i]==NULL) continue;	// 一応安全対策
				TextX = x + 96;
				TextY = y +  8 + i*MsgWindow.FontDy;
				SetTextColor(hdc,RGB(128,128,128));		// 灰色で１どっとずらして描画
				TextOut(hdc,TextX+1,TextY,MsgWindow.Msg[i],strlen(MsgWindow.Msg[i]));
				SetTextColor(hdc,RGB(255,255,255));		// 白で表示すべき位置に表示
				TextOut(hdc,TextX+0,TextY,MsgWindow.Msg[i],strlen(MsgWindow.Msg[i]));
			}

			SelectObject(hdc,oldfont);
			DxObj.Back->ReleaseDC(hdc);
		}
	}

	DrawWindowFrame(x,y,w,h);

	// お顔をかきましょう(表示を要請されている場合にだけ) //
	GrFace = FaceData[MsgWindow.FaceID/FACE_NUMX].GrSurf;
	switch(MsgWindow.FaceState){
		case(MFACE_WAIT):
			oy = MsgWindow.MaxSize.bottom - 100;
			BltSetRect(&src,(MsgWindow.FaceID%FACE_NUMX)*96,0,96,96);
			GrpBlt(&src,x+2,oy,GrFace);
		break;

		case(MFACE_OPEN):
			time = MsgWindow.FaceTime>>2;
			oy = MsgWindow.MaxSize.bottom - 100;
			for(i=0;i<96;i++){
				len = cosl(time+i*153,(64-time)/2);
				//len = cosl(time+i*4,64-time);
				BltSetRect(&src,(MsgWindow.FaceID%FACE_NUMX)*96,i,96,1);
				GrpBlt(&src,x+len+2,oy+i,GrFace);
				//if(i&1)	GrpBlt(&src,x+len+2,oy+i,GrFace);
				//else	GrpBlt(&src,x-len+2,oy+i,GrFace);
			}
		break;

		case(MFACE_NEXT):
			time = (255-MsgWindow.FaceTime)>>2;
			oy = MsgWindow.MaxSize.bottom - 100;
			for(i=0;i<96;i++){
				len = cosl(time+i*153,(64-time)/2);
				//len = cosl(time+i*4,64-time);
				BltSetRect(&src,(MsgWindow.FaceID%FACE_NUMX)*96,i,96,1);
				GrpBlt(&src,x+len+2,oy+i,GrFace);
				//if(i&1)	GrpBlt(&src,x+len+2,oy+i,GrFace);
				//else	GrpBlt(&src,x-len+2,oy+i,GrFace);
			}
		break;

		case(MFACE_CLOSE):
			time = MsgWindow.FaceTime>>1;
			oy = MsgWindow.MaxSize.bottom - 100;
			for(i=0;i<96;i++){
				len = cosl(time+i*4,time);
				BltSetRect(&src,(MsgWindow.FaceID%FACE_NUMX)*96,i,96,1);
				if(i&1)	GrpBlt(&src,x-len+2,oy+i,GrFace);
				else	GrpBlt(&src,x+len+2,oy+i,GrFace);
			}
		break;
	}
}

// メッセージ文字列を送る //
void MWinMsg(const char *s)
{
	int			Line,i;

	Line = MsgWindow.Line;

	if(Line==0){
		// 新規挿入の場合 //
		for(i=0;i<MSG_HEIGHT;i++) MsgWindow.Msg[i] = NULL;
	}
	else if(Line==MsgWindow.MaxLine){
		// すでに表示最大行数を超えていた場合 //
		for(i=1;i<MsgWindow.MaxLine-1;i++) MsgWindow.Msg[i] = MsgWindow.Msg[i+1];
		MsgWindow.Msg[Line-1] = s;
		return;
	}

	// ポインタセット＆行数更新 //
	MsgWindow.Msg[Line] = s;
	MsgWindow.Line = Line+1;
}

// 顔をセットする //
void MWinFace(BYTE faceID)
{
	if(MsgWindow.State==MWIN_DEAD) return;		// 表示不可
	if(faceID/FACE_NUMX>=FACE_MAX) return;		// あり得ない数字

	if(MsgWindow.FaceState==MFACE_NONE){
		MsgWindow.FaceState = MFACE_OPEN;
		MsgWindow.FaceID = faceID;
		GrpSetPalette(FaceData[faceID/FACE_NUMX].pal);
	}
	else{
		MsgWindow.FaceState = MFACE_NEXT;
		MsgWindow.NextFace  = faceID;
	}

	MsgWindow.FaceTime  = 0;
}

// コマンドを送る //
void MWinCmd(BYTE cmd)
{
	int		temp,i;
	int		Ysize = 0;

	switch(cmd){
		case(MWCMD_LARGEFONT):		// ラージフォントを使用する
			Ysize +=  8;
		case(MWCMD_NORMALFONT):		// ノーマルフォントを使用する
			Ysize +=  2;
		case(MWCMD_SMALLFONT):		// スモールフォントを使用する
			Ysize += 14;
			temp = MsgWindow.MaxSize.bottom - MsgWindow.MaxSize.top - 16;
			for(i=0;i<MSG_HEIGHT;i++) MsgWindow.Msg[i] = NULL;			// 文字列無効化
			MsgWindow.MaxLine = temp / Ysize;							// 表示可能最大行数
			MsgWindow.FontDy  =(temp % Ysize)/(temp/Ysize)+Ysize + 1;	// Ｙ増量
			MsgWindow.FontID  = cmd;									// 使用フォント

		case(MWCMD_NEWPAGE):		// 改ページする
			for(i=0;i<MSG_HEIGHT;i++) MsgWindow.Msg[i] = NULL;			// 文字列無効化
			MsgWindow.Line = 0;											// 最初の行へ
		break;

		default:		// ここに来たらバグね...
		break;
	}
}

// ウィンドウ枠を描画する //
static void DrawWindowFrame(int x,int y,int w,int h)
{
	RECT	src;

	w = w >> 1;
	h = h >> 1;

	// 左上 //
	SetRect(&src,0,0,w,h);
	GrpBlt(&src,x,y,GrTama);

	// 右上 //
	SetRect(&src,384-w,0,384,h);
	GrpBlt(&src,x+w,y,GrTama);

	// 左下 //
	SetRect(&src,0,80-h,w,80);
	GrpBlt(&src,x,y+h,GrTama);

	// 右下 //
	SetRect(&src,384-w,80-h,384,80);
	GrpBlt(&src,x+w,y+h,GrTama);
}

// フォントをセットする //
static HFONT SetFont(HDC hdc,BYTE FontID)
{
	// ID に対応するフォントをセットする //
	switch(FontID){
		case(MWCMD_SMALLFONT):		return (HFONT)SelectObject(hdc,WinGrpInfo.SmallFont);
		case(MWCMD_NORMALFONT):		return (HFONT)SelectObject(hdc,WinGrpInfo.NormalFont);
		case(MWCMD_LARGEFONT):		return (HFONT)SelectObject(hdc,WinGrpInfo.LargeFont);
		default:					return NULL;
	}
}

// ヘルプ文字列を送る //
void MWinHelp(WINDOW_SYSTEM *ws)
{
	WINDOW_INFO		*p;
	int				i;

	// アクティブなウィンドウを検索し、メッセージ領域をクリアする //
	p = CWinSearchActive(ws);
	for(i=1;i<MSG_HEIGHT;i++) MsgWindow.Msg[i] = NULL;

	// 一列だけ文字列を割り当てる //
	MsgWindow.Msg[0] = p->ItemPtr[ws->Select[ws->SelectDepth]]->Help;
	MsgWindow.Line   = 1;
}

// アクティブなウィンドウを探す //
static WINDOW_INFO *CWinSearchActive(WINDOW_SYSTEM *ws)
{
	WINDOW_INFO		*p;
	int				i;

	// 現在アクティブな項目を探す //
	p = &(ws->Parent);
	for(i=0;i<ws->SelectDepth;i++){
		p = p->ItemPtr[ws->Select[i]];
	}

	return p;
}

// キーボード入力を処理する //
static void CWinKeyEvent(WINDOW_SYSTEM *ws)
{
	WINDOW_INFO		*p,*p2;
	int				Depth;

	if(ws->FirstWait){
		if(Key_Data) return;
		else         ws->FirstWait = FALSE;
	}

	// 操作性が悪かった点を修正 //
	if(Key_Data==0 && ws->KeyCount){
		ws->OldKey   = 0;
		ws->KeyCount = 0;
		return;
	}

	// キーボードの過剰なリピート防止 //
	switch(ws->OldKey){
		// 一定間隔でリピートを許可するキー //
		case(KEY_UP):case(KEY_DOWN):case(KEY_LEFT):case(KEY_RIGHT):
			if(ws->KeyCount){
				ws->KeyCount--;
				if(ws->KeyCount==0) ws->OldKey=0;
			}
			else ws->KeyCount = CWIN_KEYWAIT;
		return;

		// いかなる場合もリピートを許可しないキー //
		case(KEY_TAMA):case(KEY_BOMB):case(KEY_RETURN):case(KEY_ESC):
			if(Key_Data == ws->OldKey) return;

		default:
			ws->KeyCount = 0;
		break;
	}

	ws->OldKey = Key_Data;

	// アクティブなウィンドウを検索する //
	p     = CWinSearchActive(ws);
	Depth = ws->SelectDepth;

	// 一部のキーボード入力を処理する(KEY_UP/KEY_DOWN) //
	switch(Key_Data){
		case(KEY_UP):		// 一つ上の項目へ
			ws->Select[Depth] = (ws->Select[Depth]+p->NumItems-1)%(p->NumItems);
			SndPlay(SOUND_ID_SELECT);
		return;

		case(KEY_DOWN):		// 一つ下の項目へ
			ws->Select[Depth] = (ws->Select[Depth]+1)%(p->NumItems);
			SndPlay(SOUND_ID_SELECT);
		return;

		case(KEY_ESC):case(KEY_BOMB):
			SndPlay(SOUND_ID_CANCEL);
		break;

		case(KEY_TAMA):case(KEY_RETURN):case(KEY_LEFT):case(KEY_RIGHT):
			SndPlay(SOUND_ID_SELECT);
		break;
	}

	// アクティブな項目をセットする //
	p2 = p->ItemPtr[ws->Select[Depth]];

	if(p2->CallBackFn != NULL){
		// コールバック動作時の処理 //
		if(p2->CallBackFn(Key_Data)==FALSE){
			if(Depth==0){
				if(Key_Data!=KEY_ESC && Key_Data!=KEY_BOMB){
					// 後で (CWIN_CLOSE) に変更すること//
					ws->State  = CWIN_DEAD;
					ws->OldKey = 0;			// ここは結構重要
				}
			}
			else{
				if(ws->SelectDepth != 0){
					ws->SelectDepth--;
				}
			}
		}
	}
	else{
		// デフォルトのキーボード動作 //
		switch(Key_Data){
			case(KEY_TAMA):case(KEY_RETURN):	// 決定・選択
				if(p2->NumItems!=0){
					ws->Select[Depth+1] = 0;
					ws->SelectDepth++;
				}
			break;

			case(KEY_BOMB):case(KEY_ESC):		// キャンセル
				if(ws->SelectDepth != 0){
					ws->SelectDepth--;
				}
			break;
		}
	}
}

// 平行四辺形ＢＯＸ描画 //
static void GrpBoxA2(int x1,int y1,int x2,int y2)
{
	int i;

	for(i=0;i<=y2-y1;i++)
		GrpBoxA(x1,y1+i,x2+i*2,y1+i+1);
}

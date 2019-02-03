/*
 *   MapEdit.cpp   簡易マップエディタ
 *
 */

#include "MapEdit.h"
#include "GrpUty.h"
#include <time.h>


UndoStack	Undo;
MapEditor	Map;
CopyData	Copy;
GRP			GrMapChip = NULL;
GRP			GrTama    = NULL;


static void EditorSub(void);						// エディタのサブ
static void EditorUndo(void);						// Ｕｎｄｏ用のサブ
static void EditorScroll(void);						// 画面のスクロール処理
static void EditorMapChip(void);					// マップチップ領域変更
static void EditorLayer(void);						// レイヤー変更
static void EditorInputNum(void);					// 数値入力処理
static void DrawMapChip(int x,int y,PBGMAP no);		// マップチップの描画
static void DrawCursor(void);						// カーソルの描画
static void DrawHelp(void);							// ヘルプの描画

static void ViewDraw(void);							// ビューモードの描画処理

static void ConvertEditArea();						// 作業領域内に変換する
static void ConvertChipArea();						// マップチップ領域内に変換する

static char *CreateTempFileName(void);				// テンポラリファイルの名前を決める

// エディタの初期化 //
BOOL EditorInit(void)
{
	int		i,j;

	GrMapChip = GrpCreateSurface(BMP_CHIP_WIDTH,BMP_CHIP_HEIGHT,ID_MAP_CHIP);
	if(!GrMapChip) return FALSE;
	GrTama    = GrpCreateSurface(640,480,ID_TAMA);
	if(!GrTama) return FALSE;

	if(!GrpBMPLoad("MAP.BMP",ID_MAP_CHIP))	return FALSE;
	if(!GrpBMPLoad("TAMA.BMP",ID_TAMA))		return FALSE;


	// 構造体の初期化 //
	for(i=0;i<LAYER_MAX;i++){
		Map.Data.ScrollWait[i] = (LAYER_MAX-i)*10;//SCROLLWAIT_DEFAULT;
		for(j=0;j<MAP_SIZE;j++){
			Map.Data.Data[i][j].DataID = MAPDATA_NONE;
			Map.Data.Data[i][j].bMask  = FALSE;
		}
	}

	Map.ChipX    = 0;
	Map.Line     = 0;
	Map.State    = ME_NONE;
	Map.KeyCount = 0;
	Map.ViewTime = 0;

	Undo.UndoLeft = 0;
	Undo.RedoLeft = 0;
	Undo.StackPtr = 0;

	LoadMapData("MAPEDIT2.DAT");

	return TRUE;
}

// エディタのメイン //
void EditorMain(void)
{
	BOOL bMouseMove;

	GetKeyboardState(Map.KeyInfo);
	GetCursorPos(&Map.MouseInfo);
	Map.MouseInfo.x &= (~0xf);
	Map.MouseInfo.y &= (~0xf);
	Map.MouseL = (Map.KeyInfo[VK_LBUTTON]&0x80) ? TRUE : FALSE;
	Map.MouseR = (Map.KeyInfo[VK_RBUTTON]&0x80) ? TRUE : FALSE;

	// マウスが動いたかどうか(ME_PUTRECTで使用する) //
	if(Map.MouseInfo.x==Map.MouseTemp.x && Map.MouseInfo.y==Map.MouseTemp.y)
		bMouseMove = FALSE;
	else
		bMouseMove = TRUE;

	// キーリピート回避用(マウスにも使用する) //
	if(Map.KeyCount) Map.KeyCount--;

	if(Map.State != ME_VIEW) EditorDraw();
	else                     ViewDraw();

	switch(Map.State){
		case(ME_MASK_ON):		// マスク(描き込み不可)領域選択中
			if(!Map.MouseR){
				StkPushUndo();
				SetMask();
				Map.State = ME_NONE;
			}
			else{
				ConvertEditArea();
				Map.Select.right  = Map.MouseInfo.x;
				Map.Select.bottom = Map.MouseInfo.y;
			}
		break;

		case(ME_MASK_OFF):		// マスク(描き込み 可 )領域選択中
			if(!Map.MouseL){
				StkPushUndo();
				ResetMask();
				Map.State = ME_NONE;
			}
			else{
				ConvertEditArea();
				Map.Select.right  = Map.MouseInfo.x;
				Map.Select.bottom = Map.MouseInfo.y;
			}
		break;

		case(ME_GETRECTM):		// パーツ選択中(Mapﾊﾟｰﾂから)
			if(!Map.MouseR){
			// 下で代入しているのはＰＵＴを行う際に移動したかどうかを判別するための
			// 変数なのだが、１回目は移動していなくても実行できるようにしなければならない。
			// 従ってダミーの値である -1 を代入している。
			// なお、この処理は、マウスのボタンの押し続けによる Push の多発を防ぐことにある。
				Map.MouseTemp.x = -1;
				Map.MouseTemp.y = -1;
				ConvertRect(&Map.Select);
				SetCursorPos(Map.Select.left,Map.Select.top);
				GetMapDataM();
				Map.State = ME_PUTRECT;
			}
			else{
				ConvertChipArea();
				Map.Select.right  = Map.MouseInfo.x;
				Map.Select.bottom = Map.MouseInfo.y;
			}
		break;

		case(ME_GETRECTE):		// パーツ選択中(作業領域から)
			if(!Map.MouseR){
			// 下で代入しているのはＰＵＴを行う際に移動したかどうかを判別するための
			// 変数なのだが、１回目は移動していなくても実行できるようにしなければならない。
			// 従ってダミーの値である -1 を代入している。
			// なお、この処理は、マウスのボタンの押し続けによる Push の多発を防ぐことにある。
				Map.MouseTemp.x = -1;
				Map.MouseTemp.y = -1;
				ConvertRect(&Map.Select);
				SetCursorPos(Map.Select.left,Map.Select.top);
				GetMapDataE();
				Map.State = ME_PUTRECT;
			}
			else{
				ConvertEditArea();
				Map.Select.right  = Map.MouseInfo.x;
				Map.Select.bottom = Map.MouseInfo.y;
			}
		break;

		case(ME_PUTRECT):		// パーツ貼り付け中
			if(Map.MouseL && bMouseMove){
				StkPushUndo();		// 状態をＰＵＳＨしておく
				PutMapData();
				Map.MouseTemp = Map.MouseInfo;	// 判別用変数の更新
			}
			else if(Map.KeyInfo['P']&0x80){
				// 画面全体を塗りつぶし //
			}
			else if(Map.MouseR){
				FreeMapData();
				Map.State = ME_NONE;
				Map.KeyCount=KEY_WAITTIME;
			}

			EditorScroll();			// スクロールを許可する
			EditorLayer();			// レイヤー変更を許可
		break;

		case(ME_VIEW):			// ビューモード
			// 安全のため、やや手抜き //
			if(Map.ViewTime*TIME_PER_FRAME/16/Map.Data.ScrollWait[0]<MAP_LEN-MAP_HEIGHT*2){
				if(Map.KeyInfo[VK_SHIFT]&0x80) Map.ViewTime += 8;
				else                           Map.ViewTime++;
			}

			if(Map.KeyInfo['V']&0x80){
				if(Map.KeyCount) break;
				Map.State    = ME_NONE;
				Map.KeyCount = KEY_WAITTIME;
				Map.ViewTime = 0;
			}
		break;

		case(ME_HELP):			// ヘルプ表示
			if(Map.MouseR || Map.MouseL || (Map.KeyInfo[VK_F1]&0x80)){
				if(Map.KeyCount) break;
				Map.State    = ME_NONE;
				Map.KeyCount = KEY_WAITTIME;
			}
			EditorScroll();			// スクロールを許可する
			EditorMapChip();		// マップパーツ領域変更を許可
			EditorLayer();			// レイヤー変更
		break;

		case(ME_NONE):			// 何もしない
			EditorScroll();			// スクロールを許可する
			EditorMapChip();		// マップパーツ領域変更を許可
			EditorLayer();			// レイヤー変更
			EditorSub();			// 通常時の処理...
		break;

		default:				// エラー
		break;
	}

	EditorUndo();		// Ｕｎｄｏ／Ｒｅｄｏの処理
}

// エディタのサブ //
static void EditorSub(void)
{
	MapData	*p;
	BOOL	MaskFlag;

	BOOL bCtrl  = (Map.KeyInfo[VK_CONTROL]&0x80) ? TRUE : FALSE;
	int i,cx,cy;

	// 右クリック //
	if(Map.MouseR && Map.KeyCount==0){
		cx = Map.MouseInfo.x>>4;
		cy = Map.MouseInfo.y>>4;

		if(cx>=RESERVED_WIDTH && cx<RESERVED_WIDTH+MAP_WIDTH && cy>=RESERVED_HEIGHT){
			if(bCtrl) Map.State = ME_MASK_ON;
			else      Map.State = ME_GETRECTE;
		}
		else if(cx>=RESERVED_WIDTH+MAP_WIDTH){
			Map.State = ME_GETRECTM;
		}
		else return;

		Map.Select.left = Map.Select.right  = Map.MouseInfo.x;
		Map.Select.top  = Map.Select.bottom = Map.MouseInfo.y;
		return;
	}

	// 左クリック //
	if(Map.MouseL && Map.KeyCount==0){
		cx = Map.MouseInfo.x>>4;
		cy = Map.MouseInfo.y>>4;

		if(cx>=RESERVED_WIDTH && cx<RESERVED_WIDTH+MAP_WIDTH && cy>=RESERVED_HEIGHT){
			if(bCtrl) Map.State = ME_MASK_OFF;
			else      return;//Map.State = ME_GETRECTE;
		}
		//else if(cx>=RESERVED_WIDTH+MAP_WIDTH){
		//	Map.State = ME_GETRECTM;
		//}
		else return;

		Map.Select.left = Map.Select.right  = Map.MouseInfo.x;
		Map.Select.top  = Map.Select.bottom = Map.MouseInfo.y;
		return;
	}

	// HELP 表示モードに移行 //
	if((Map.KeyInfo[VK_F1]&0x80) && Map.KeyCount==0){
		Map.State = ME_HELP;
		Map.KeyCount = KEY_WAITTIME;
		return;
	}

	// VIEW モードに移行 //
	if((Map.KeyInfo['V']&0x80) && Map.KeyCount==0){
		Map.State = ME_VIEW;
		Map.KeyCount = KEY_WAITTIME;
		return;
	}

	// マスクを全体にかける(or 全解除) //
	if((Map.KeyInfo['M']&0x80) && Map.KeyCount==0){
		// Ｕｎｄｏが効くようにする //
		StkPushUndo();

		// 現在表示されているものをチェックする //
		p = &Map.Data.Data[Map.Layer][Map.Line*MAP_WIDTH];
		MaskFlag = FALSE;
		for(i=0;i<MAP_WIDTH*MAP_HEIGHT;i++,p++){
			if(!p->bMask){
				MaskFlag = TRUE;
				break;
			}
		}

		p = &Map.Data.Data[Map.Layer][Map.Line*MAP_WIDTH];
		for(i=0;i<MAP_WIDTH*MAP_HEIGHT;i++,p++) p->bMask=MaskFlag;

		Map.KeyCount=KEY_WAITTIME;

		return;
	}

	// 画面全体をコピーする(範囲指定)
	if(Map.KeyInfo['A']&0x80){
		Map.MouseTemp.x = -1;
		Map.MouseTemp.y = -1;

		Map.Select.left   = (RESERVED_WIDTH)*16;
		Map.Select.top    = (RESERVED_HEIGHT)*16;
		Map.Select.right  = (RESERVED_WIDTH+MAP_WIDTH-1)*16;
		Map.Select.bottom = 480-16;

		SetCursorPos(Map.Select.left,Map.Select.top);
		GetMapDataE();
		Map.State = ME_PUTRECT;
		return;
	}

	// 簡易テンポラリセーブ //
	if((Map.KeyInfo['S']&0x80) && Map.KeyCount==0){
		SaveMapData(CreateTempFileName());
		Map.KeyCount=KEY_WAITTIME;
	}
}

// Ｕｎｄｏ用のサブ //
static void EditorUndo(void)
{
	// Undo //
	if((Map.KeyInfo['U']&0x80) && Map.KeyCount==0){
		StkPopUndo();
		Map.KeyCount = KEY_WAITTIME;
		return;
	}

	// Redo //
	if((Map.KeyInfo['R']&0x80) && Map.KeyCount==0){
		StkRedo();
		Map.KeyCount = KEY_WAITTIME;
		return;
	}
}

// 画面のスクロール処理 //
static void EditorScroll(void)
{
	int		temp,x,y;
	BOOL	bShift;

	// シフトキーのチェック //
	bShift = (Map.KeyInfo[VK_SHIFT]&0x80) ? TRUE : FALSE;

	// マウス→キーボードへ //
	x = Map.MouseInfo.x>>4;
	y = Map.MouseInfo.y>>4;
	if(Map.MouseL && Map.State==ME_NONE){
		if(x<RESERVED_WIDTH && y>=RESERVED_HEIGHT){
			if(y<15) Map.KeyInfo[VK_UP]   |= 0x80;
			else     Map.KeyInfo[VK_DOWN] |= 0x80;
		}
	}

	// スクロール(SHIFT を考慮に入れる) //
	if(Map.KeyInfo[VK_DOWN]&0x80){
		temp = (bShift) ? 10 : 1;
		Map.Line = max(Map.Line-temp,0);
		if(Map.State==ME_PUTRECT){
			Map.MouseTemp.x = -1;
			Map.MouseTemp.y = -1;
		}
		return;
	}
	if(Map.KeyInfo[VK_UP]&0x80){
		temp = (bShift) ? 10 : 1;
		Map.Line = min(Map.Line+temp,MAP_LEN-MAP_HEIGHT);
		if(Map.State==ME_PUTRECT){
			Map.MouseTemp.x = -1;
			Map.MouseTemp.y = -1;
		}
		return;
	}
}

// マップチップ領域変更
static void EditorMapChip(void)
{
	int x,y,temp;

	// マウス→キーボードへ //
	x    = (Map.MouseInfo.x>>4)-(RESERVED_WIDTH+MAP_WIDTH);
	y    = Map.MouseInfo.y>>4;
	temp = (40-(RESERVED_WIDTH+MAP_WIDTH))/2;

	if(Map.MouseL){
		if(x>=0){
			if(x>temp) Map.KeyInfo[VK_RIGHT] |= 0x80;
			else       Map.KeyInfo[VK_LEFT]  |= 0x80;
		}
	}

	if(Map.KeyInfo[VK_LEFT]&0x80){
		if(Map.ChipX>0) Map.ChipX--;
		return;
	}
	if(Map.KeyInfo[VK_RIGHT]&0x80){
		if(Map.ChipX<40-CHIP_WIDTH) Map.ChipX++;
		return;
	}
}

// レイヤー変更 //
static void EditorLayer(void)
{
	int		i,j;

	// レイヤー変更 //
	if(Map.KeyInfo[VK_TAB]&0x80 && Map.KeyCount==0){
		Map.Layer = (Map.Layer+1)%LAYER_MAX;
		Map.KeyCount=KEY_WAITTIME;
		if(Map.State==ME_PUTRECT){
			Map.MouseTemp.x = -1;
			Map.MouseTemp.y = -1;
		}
		return;
	}

	// レイヤーの重みを増減する //
	if((Map.KeyInfo[VK_ADD]&0x80) && Map.KeyCount==0){
		j = (Map.KeyInfo[VK_SHIFT]&0x80) ? 10 : 1;
		if(Map.Data.ScrollWait[Map.Layer]+j<999) Map.Data.ScrollWait[Map.Layer]+=j;
		else                                     Map.Data.ScrollWait[Map.Layer] =999;
		Map.KeyCount=KEY_WAITTIME;
		for(i=Map.Layer-1;i>=0;i--){
			if(Map.Data.ScrollWait[Map.Layer]>Map.Data.ScrollWait[i])
				Map.Data.ScrollWait[i] = Map.Data.ScrollWait[Map.Layer];
		}
	}
	if((Map.KeyInfo[VK_SUBTRACT]&0x80) && Map.KeyCount==0){
		j = (Map.KeyInfo[VK_SHIFT]&0x80) ? 10 : 1;
		if(Map.Data.ScrollWait[Map.Layer]-j>0) Map.Data.ScrollWait[Map.Layer]-=j;
		else                                   Map.Data.ScrollWait[Map.Layer] =1;
		Map.KeyCount=KEY_WAITTIME;
		for(i=Map.Layer+1;i<LAYER_MAX;i++){
			if(Map.Data.ScrollWait[Map.Layer]<Map.Data.ScrollWait[i])
				Map.Data.ScrollWait[i] = Map.Data.ScrollWait[Map.Layer];
		}
	}
}

// 描画関連 //
void EditorDraw(void)
{
	MapData	*p;
	int		i,j,k,x,y,dy,wait,temp;
	RECT	src;
	char	buf[100],timebuf[20];

	GrpCls();

	// 情報の表示 //
	_strtime(timebuf);
	wait = Map.Data.ScrollWait[Map.Layer];
	sprintf(buf,"L:%1d SW:%03d Count:%08d  <%s>",
		Map.Layer,wait,wait*Map.Line*16,timebuf);
	x = 10*8;//RESERVED_WIDTH<<4;
	y = 0;
	GrpPuts(buf,x,y);
	switch(Map.State){
		case(ME_NONE):		strcpy(buf,"        ");	break;
		case(ME_MASK_ON):	strcpy(buf,"MSK(ON) ");	break;
		case(ME_MASK_OFF):	strcpy(buf,"MSK(OFF)");	break;
		case(ME_GETRECTM):	strcpy(buf,"GET(M)  ");	break;
		case(ME_GETRECTE):	strcpy(buf,"GET(E)  ");	break;
		case(ME_PUTRECT):	strcpy(buf,"PUT     ");	break;
		case(ME_VIEW):		strcpy(buf,"VIEW    ");	break;
		case(ME_HELP):		strcpy(buf,"HELP    ");	break;
	}
	GrpPuts(buf,0,0);

	// マップパーツパレットの描画 //
	x = (RESERVED_WIDTH+MAP_WIDTH)<<4;
	y = 0;
	SetRect(&src,Map.ChipX<<4,0,640,480);
	GrpBlt(&src,x,y,GrMapChip);

	// ラインの表示 //
	GrpLock();
	GrpSetColor(0,0,1+Map.Layer);
	GrpBox(0,RESERVED_HEIGHT<<4,RESERVED_WIDTH<<4,480);
	GrpUnlock();
	for(i=0;i<MAP_HEIGHT;i++){
		sprintf(buf,"%04d",Map.Line+i);
		x = 0;
		y = (480-16+4)-(i<<4);
		GrpPut8(buf,x,y);
	}

	// 下のレイヤーの表示 //
	for(k=0;k<Map.Layer;k++){
		temp = (Map.Line*Map.Data.ScrollWait[Map.Layer])<<4;
		p = &Map.Data.Data[k][((temp/(Map.Data.ScrollWait[k]))>>4)*MAP_WIDTH];
		dy = (temp/(Map.Data.ScrollWait[k]))%16;
		for(i=0;i<MAP_HEIGHT;i++){
			for(j=0;j<MAP_WIDTH;j++){
				// マスクに関係なく描画を行う //
				x = (j+RESERVED_WIDTH)<<4;
				y = (480-16)-(i<<4)+dy;
				if(p->DataID != MAPDATA_NONE)
					DrawMapChip(x,y,p->DataID);
				p++;
			}
		}
	}

	// 下のレイヤーを暗くする //
	if(Map.Layer){
		GrpSetAlpha(180,ALPHA_NORM);
		GrpSetColor(1,1,1);//2,1,0);
		GrpLock();
		GrpBoxA(RESERVED_WIDTH*16,RESERVED_HEIGHT*16,(RESERVED_WIDTH+MAP_WIDTH)*16,480);
		GrpUnlock();
	}

	// マップパーツ作業領域の描画(レイヤーを考慮に入れる) //
	p = &Map.Data.Data[Map.Layer][Map.Line*MAP_WIDTH];
	for(i=0;i<MAP_HEIGHT;i++){
		for(j=0;j<MAP_WIDTH;j++){
			// マスクに関係なく描画を行う //
			x = (j+RESERVED_WIDTH)<<4;
			y = (480-16)-(i<<4);
			if(p->DataID != MAPDATA_NONE)
				DrawMapChip(x,y,p->DataID);
			p++;
		}
	}

	// マスクの描画(Lockを考慮に入れると、上と別処理にした方がはやい) //
	p = &Map.Data.Data[Map.Layer][Map.Line*MAP_WIDTH];
	GrpSetAlpha(180,ALPHA_NORM);
	GrpSetColor(0,0,3);
	GrpLock();
	for(i=0;i<MAP_HEIGHT;i++){
		for(j=0;j<MAP_WIDTH;j++){
			if(p->bMask){
				x = (j+RESERVED_WIDTH)<<4;
				y = (480-16)-(i<<4);
				GrpBoxA(x,y,x+16,y+16);
			}
			p++;
		}
	}
	GrpUnlock();

	// ヘルプの描画 //
	if(Map.State == ME_HELP)
		DrawHelp();

	// カーソルの描画 //
	DrawCursor();

	GrpFlip();
}

// ビューモードの描画処理 //
static void ViewDraw(void)
{
	MapData	*p;
	RECT	src;
	DWORD	Time = Map.ViewTime*TIME_PER_FRAME;
	DWORD	Line;
	int		k,x,y,dy,i,j;

	GrpCls();

	// 全てのレイヤーの表示 //
	for(k=0;k<LAYER_MAX;k++){
		Line = (Time/Map.Data.ScrollWait[k]);
		p    = &Map.Data.Data[k][(Line/16)*MAP_WIDTH];
		dy   = Line%16;
		for(i=0;i<MAP_HEIGHT;i++){
			for(j=0;j<MAP_WIDTH;j++){
				// マスクに関係なく描画を行う //
				x = (j+RESERVED_WIDTH)<<4;
				y = (480-16)-(i<<4)+dy;
				if(p->DataID != MAPDATA_NONE)
					DrawMapChip(x,y,p->DataID);
				p++;
			}
		}
	}

	GrpFlip();
}

// マップチップの描画 //
static void DrawMapChip(int x,int y,PBGMAP no)
{
	RECT	src;

	// テーブル化するかね... //
	src.left   = (no%(BMP_CHIP_WIDTH/16))<<4;
	src.top    = (no/(BMP_CHIP_WIDTH/16))<<4;

	src.right  = src.left + 16;
	src.bottom = src.top  + 16;

	GrpBlt(&src,x,y,GrMapChip);
}

// カーソルの描画 //
static void DrawCursor(void)
{
	int		x,y,width,height;
	RECT	select;

	static BYTE counter;

	//counter+=4;
	//temp=sinl(counter,50)+180;

	switch(Map.State){
		case(ME_MASK_ON):		// マスク(描き込み不可)領域選択中
		case(ME_MASK_OFF):		// マスク(描き込み 可 )領域選択中
		case(ME_GETRECTM):		// パーツ選択中(Mapﾊﾟｰﾂから)
		case(ME_GETRECTE):		// パーツ選択中(作業領域から)
			ConvertRect(&select);
			x = select.left;
			y = select.top;
			width  = (select.right-select.left)+16;
			height = (select.bottom-select.top)+16;
			GrpSetAlpha(180,ALPHA_NORM);
			GrpSetColor(0,3,0);
		break;

		case(ME_PUTRECT):		// パーツ貼り付け中
			ConvertRect(&select);
			x = Map.MouseInfo.x;
			y = Map.MouseInfo.y;
			width  = (select.right-select.left)+16;
			height = (select.bottom-select.top)+16;
			GrpSetAlpha(180,ALPHA_NORM);
			GrpSetColor(3,0,0);
		break;

		case(ME_HELP):			// ヘルプ表示
		default:
			//ME_NONE			// 何もしない
			//ME_VIEW,			// ビューモード
			x = Map.MouseInfo.x;
			y = Map.MouseInfo.y;
			width = height = 16;
			GrpSetAlpha(200,ALPHA_NORM);
			GrpSetColor(4,4,4);
		break;
	}

	GrpLock();
	GrpBoxA(x,y,x+width,y+height);
	GrpUnlock();
}

// ヘルプの描画
static void DrawHelp(void)
{
	HDC		hdc;
	int		i;

#define HELP_ITEM	23
#define HELP_SX		(16*4)
#define HELP_SY		(16*4)

	static char *HelpData[HELP_ITEM] ={
		" MapEditor2  for Neo GIAN ..秋霜玉(仮    "MAPEDIT2_VERSION,
		" ",
		" ",
		"貼り付け                      : 選択後,左クリック",
		"範囲選択(コピー)              : 右ドラッグ       ",
		"範囲選択(マスクON)            : Ctrl + 左ドラッグ",
		"範囲選択(マスクOFF)           : Ctrl + 右ドラッグ",
		"スクロール                    : ↑↓ + (Shift)   ",
		"マップパーツの範囲変更        : ←→             ",
		"    (Line/Mapﾊﾟｰﾂ上で右クリックでスクロール可能) ",
		" ",
		"レイヤー変更                  : TAB              ",
		"レイヤーの重み変更            : ＋－ +  (Shift)  ",
		"ビューモード(実際の早さで)    : Ｖ               ",
		"画面全体をコピー              : Ａ               ",
		"画面内のマスクセット/リセット : Ｍ               ",
		"元に戻す(Undo)                : Ｕ               ",
		"やり直し(Redo)                : Ｒ               ",
		"一時セーブ                    : Ｓ               ",
		"強制終了(変更する予定)        : ESC              ",
		" ",
		" ",
		" ",
	};

	HFONT	hfont;

	GrpLock();
	GrpSetAlpha(180,ALPHA_NORM);
	for(i=0;i<HELP_ITEM;i++){
		if(i&0x01) GrpSetColor(0,0,4);
		else       GrpSetColor(0,0,3);
		GrpBoxA(HELP_SX,HELP_SY+i*16,HELP_SX+16*19,HELP_SY+16+i*16);
	}
	GrpUnlock();

	hfont = CreateFont(12,0,0,0,0,0,0,0,SHIFTJIS_CHARSET,OUT_TT_ONLY_PRECIS,
		CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH,NULL);

	if(DxObj.Back->GetDC(&hdc) != DD_OK) return;
	SelectObject(hdc,hfont);
	SetBkMode(hdc,TRANSPARENT);

	SetTextColor(hdc,RGB(128,128,128));
	for(i=0;i<HELP_ITEM;i++){
		TextOut(hdc,HELP_SX+6,HELP_SY+3+i*16,HelpData[i],lstrlen(HelpData[i]));
	}
	SetTextColor(hdc,RGB(255,255,255));
	for(i=0;i<HELP_ITEM;i++){
		TextOut(hdc,HELP_SX+5,HELP_SY+3+i*16,HelpData[i],lstrlen(HelpData[i]));
	}

	DxObj.Back->ReleaseDC(hdc);

	DeleteObject(hfont);
}

// マップチップの配置データをLoadする //
BOOL LoadMapData(char *filename)
{
	ScrollSaveHeader	Header[LAYER_MAX];
	FILE				*fp;
	int					i,j,k;
	DWORD				NumLayer;
	PBGMAP				Data,Count;

	fp = fopen(filename,"rb");
	if(fp==NULL) return FALSE;

	// レイヤ数 //
	fread(&NumLayer,sizeof(DWORD),1,fp);

	// ヘッダ×ｎ 読み込み //
	fread(Header,sizeof(ScrollSaveHeader),NumLayer,fp);

	// ちと、深いかも... //
	for(i=0;i<NumLayer;i++){
		Map.Data.ScrollWait[i] = Header[i].ScrollWait;
		fseek(fp,Header[i].Address,SEEK_SET);
		for(j=0;j<Header[i].Length;j++){
			for(k=0;k<MAP_WIDTH;){
				fread(&Data,sizeof(PBGMAP),1,fp);
				if(Data==MAPDATA_NONE){
					// ゼロ圧縮がかかっている場合 //
					fread(&Count,sizeof(PBGMAP),1,fp);
					for(;Count>0;Count--){
						Map.Data.Data[i][k+j*MAP_WIDTH].DataID = MAPDATA_NONE;
						Map.Data.Data[i][k+j*MAP_WIDTH].bMask  = FALSE;
						k = k+1;
					}
				}
				else{
					// 通常のデータ //
					Map.Data.Data[i][k+j*MAP_WIDTH].DataID = Data;
					Map.Data.Data[i][k+j*MAP_WIDTH].bMask  = FALSE;
					k = k+1;
				}
			}
		}
	}

	fclose(fp);

	return TRUE;
}

// マップチップの配置データをSaveする //
BOOL SaveMapData(char *filename)
{
	ScrollSaveHeader	Header[LAYER_MAX];
	FILE				*fp;
	int					i,j,k;
	DWORD				NumLayer;
	DWORD				Length;
	DWORD				StartAddr;
	PBGMAP				Data,Count;

	// この部分は後で変更する事 //
	ZeroMemory(Header,sizeof(Header));
	NumLayer = LAYER_MAX;

	// 書き込みの準備＆ヘッダの書き込み(領域確保) //
	fp = fopen(filename,"wb");
	if(fp==NULL) return FALSE;
	fwrite(&NumLayer,sizeof(DWORD),1,fp);
	for(i=0;i<NumLayer;i++) fwrite(&Header,sizeof(ScrollSaveHeader),1,fp);

	// 開始アドレスの初期値 //
	StartAddr = ftell(fp);

	for(i=0;i<NumLayer;i++){
		Length = MAP_LEN;	// 後で変更すること

		// データをゼロ圧縮しながら書き込み(Length の変動に注意) //
		for(j=0;j<Length;j++){
			Count = 0;
			for(k=0;k<MAP_WIDTH;k++){
				Data = Map.Data.Data[i][k+j*MAP_WIDTH].DataID;
				if(Data==MAPDATA_NONE){
					if(Count==0) fwrite(&Data,sizeof(Data),1,fp);
					Count++;
				}
				else if(Count){
					fwrite(&Count,sizeof(Count),1,fp);		// 繰り返し回数
					fwrite(&Data ,sizeof(Data) ,1,fp);		// 次のデータ
					Count = 0;
				}
				else fwrite(&Data,sizeof(Data),1,fp);		// 通常データの書き込み
			}
			if(Count) fwrite(&Count,sizeof(Count),1,fp);	// 繰り返し回数
		}

		// ヘッダに書き込むべき値を保存する //
		Header[i].Address    = StartAddr;					// 書き込み開始アドレス
		Header[i].ScrollWait = Map.Data.ScrollWait[i];		// スクロールウェイト
		Header[i].Length     = Length;						// このレイヤーの長さ(縦方向)

		StartAddr = ftell(fp);								// 開始アドレスの更新
	}

	// ヘッダを書き直す //
	fseek(fp,sizeof(DWORD),SEEK_SET);
	fwrite(&Header,sizeof(ScrollSaveHeader),NumLayer,fp);
	fclose(fp);

	return TRUE;
}

// 作業領域内に変換する //
static void ConvertEditArea(void)
{
	BOOL flag = FALSE;

	if(Map.MouseInfo.x < RESERVED_WIDTH*16)
		Map.MouseInfo.x = RESERVED_WIDTH*16,flag=TRUE;

	if(Map.MouseInfo.x > (RESERVED_WIDTH+MAP_WIDTH-1)*16)
		Map.MouseInfo.x = (RESERVED_WIDTH+MAP_WIDTH-1)*16,flag=TRUE;

	if(Map.MouseInfo.y < RESERVED_HEIGHT*16)
		Map.MouseInfo.y = RESERVED_HEIGHT*16,flag=TRUE;

	if(flag) SetCursorPos(Map.MouseInfo.x,Map.MouseInfo.y);
}

// マップチップ領域内に変換する
static void ConvertChipArea(void)
{
	BOOL flag = FALSE;

	if(Map.MouseInfo.x < (RESERVED_WIDTH+MAP_WIDTH)*16)
		Map.MouseInfo.x = (RESERVED_WIDTH+MAP_WIDTH)*16,flag=TRUE;

	if(flag) SetCursorPos(Map.MouseInfo.x,Map.MouseInfo.y);
}

// 小→大となる矩形を得る //
void ConvertRect(RECT *rc)
{
	// 自分自身に対しても変更できるように //
	volatile RECT temp = Map.Select;

	// 横方向 //
	if(Map.Select.right > Map.Select.left){
		rc->left  = temp.left;
		rc->right = temp.right;
	}
	else{
		rc->left  = temp.right;
		rc->right = temp.left;
	}

	// 縦方向 //
	if(Map.Select.bottom > Map.Select.top){
		rc->bottom = temp.bottom;
		rc->top    = temp.top;
	}
	else{
		rc->bottom = temp.top;
		rc->top    = temp.bottom;
	}
}

// Undo スタックを用いて Redo を行う //
void StkRedo(void)
{
	// Redo できるか //
	if(Undo.RedoLeft==0) return;

	// ポインタを移動してからスタックから取りだす //
	Undo.StackPtr = (Undo.StackPtr+1)%UNDO_MAX;
	Map.Data = Undo.Data[Undo.StackPtr];

	// Undo/Redo の回数調整 //
	Undo.RedoLeft--;
	Undo.UndoLeft++;
}

// Undo スタックに挿入 //
void StkPushUndo(void)
{
	// 残り回数を増やす //
	if(Undo.UndoLeft<UNDO_MAX-1) Undo.UndoLeft++;

	// Redo できないようにする //
	Undo.RedoLeft = 0;

	// スタックに挿入し、ポインタを移動する //
	Undo.Data[Undo.StackPtr] = Map.Data;
	Undo.StackPtr = (Undo.StackPtr+1)%UNDO_MAX;
}

// Undo スタックから取り出し //
void StkPopUndo(void)
{
	// Undo できるか //
	if(Undo.UndoLeft==0) return;

	// １回目のＵｎｄｏの場合現在の状態を詰めておく //
	if(Undo.RedoLeft==0) Undo.Data[Undo.StackPtr] = Map.Data;

	// ポインタを移動してからスタックから取り出し //
	Undo.StackPtr = (Undo.StackPtr+UNDO_MAX-1)%UNDO_MAX;
	Map.Data = Undo.Data[Undo.StackPtr];

	// Undo/Redo の回数調整 //
	Undo.RedoLeft++;
	Undo.UndoLeft--;
}

// マスクをセットする //
void SetMask(void)
{
	int width,height,i,j,x,y;
	MapData	*p;

	// 前準備 //
	ConvertRect(&Map.Select);
	width  = ((Map.Select.right - Map.Select.left)>>4)+1;
	height = ((Map.Select.bottom - Map.Select.top)>>4)+1;

	// マスクをセットする //
	x = (Map.Select.left>>4)-RESERVED_WIDTH;
	y = 30-RESERVED_HEIGHT-(Map.Select.bottom>>4);
	p = &Map.Data.Data[Map.Layer][(Map.Line+y)*MAP_WIDTH+x];
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			(p+(i*MAP_WIDTH)+j)->bMask = TRUE;
		}
	}
}

// マスクをリセットする //
void ResetMask(void)
{
	int width,height,i,j,x,y;
	MapData	*p;

	// 前準備 //
	ConvertRect(&Map.Select);
	width  = ((Map.Select.right - Map.Select.left)>>4)+1;
	height = ((Map.Select.bottom - Map.Select.top)>>4)+1;

	// マスクをセットする //
	x = (Map.Select.left>>4)-RESERVED_WIDTH;
	y = 30-RESERVED_HEIGHT-(Map.Select.bottom>>4);
	p = &Map.Data.Data[Map.Layer][(Map.Line+y)*MAP_WIDTH+x];
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			(p+(i*MAP_WIDTH)+j)->bMask = FALSE;
		}
	}
}

// マップデータの取得(マップパーツエリア) //
void GetMapDataM(void)
{
	int		width,height,i,j,sx,sy;
	PBGMAP	*p;

	// 前準備 //
	width  = (Map.Select.right>>4)-(Map.Select.left>>4) + 1;
	height = (Map.Select.bottom>>4)-(Map.Select.top>>4) + 1;

	// 構造体の初期化 //
	Copy.Width  = width;
	Copy.Height = height;
	Copy.Data   = (PBGMAP *)LocalAlloc(LPTR,sizeof(PBGMAP)*width*height);

	// 実際にデータを代入する //
	p  = Copy.Data;
	sx = Map.ChipX + (Map.Select.left>>4) - (RESERVED_WIDTH+MAP_WIDTH);
	sy = Map.Select.top>>4;
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			*(p++) = (PBGMAP)(sx+j+(sy+i)*(BMP_CHIP_WIDTH>>4));
		}
	}
}

// マップデータの取得(作業領域) //
void GetMapDataE(void)
{
	int		width,height,i,j,sx,sy;
	PBGMAP	*dest;
	MapData	*src;

	// 前準備 //
	width  = (Map.Select.right>>4)-(Map.Select.left>>4) + 1;
	height = (Map.Select.bottom>>4)-(Map.Select.top>>4) + 1;

	// 構造体の初期化 //
	Copy.Width  = width;
	Copy.Height = height;
	Copy.Data   = (PBGMAP *)LocalAlloc(LPTR,sizeof(PBGMAP)*width*height);

	// 実際にデータを代入する //
	sx   = (Map.Select.left>>4)-RESERVED_WIDTH;
	sy   = 29-(Map.Select.top>>4);
	src  = &Map.Data.Data[Map.Layer][(Map.Line+sy)*MAP_WIDTH+sx];
	dest = Copy.Data;
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			*(dest++) = (src-(i*MAP_WIDTH)+j)->DataID;
		}
	}
}

// マップデータの解放 //
void FreeMapData(void)
{
	// ダミー値を代入(０の時はコピーできないので安全か？) //
	Copy.Width  = 0;
	Copy.Height = 0;

	// メモリ解放(SAFE_RELEASE) //
	if(Copy.Data) LocalFree(Copy.Data);
	Copy.Data = NULL;
}

// マップデータの貼り付け //
void PutMapData(void)
{
	int			i,j,x,y;
	MapData		*p;
	PBGMAP		*src;

	x = (Map.MouseInfo.x>>4)-RESERVED_WIDTH;
	y = 29-(Map.MouseInfo.y>>4);
	p = &Map.Data.Data[Map.Layer][(Map.Line+y)*MAP_WIDTH+x];

	// 実際にコピーを行う //
	src = Copy.Data;
	for(i=0;i<Copy.Height;i++){
		for(j=0;j<Copy.Width;j++){
			// クリッピングをかける //
			if(x+j>=0 && x+j<MAP_WIDTH && y-i>=0 && y-i<MAP_HEIGHT){
				// マスクチェックをする //
				if((p-(i*MAP_WIDTH)+j)->bMask == FALSE){
					(p-(i*MAP_WIDTH)+j)->DataID = *src;
				}
			}
			src++;
		}
	}

	/* ここでメモリを解放してもよいが、複数回コピーを行うことを許可するため、
	 * あえてメモリ解放をしない
	 */
}

// テンポラリファイルの名前を決める //
static char *CreateTempFileName(void)
{
	int			i;
	static char buffer[1000];
	FILE		*fp;

	// まだ存在しないファイルを探す //
	for(i=0;i<1000;i++){
		sprintf(buffer,"MAPE%03d.TMP",i);
		fp = fopen(buffer,"rb");
		if(fp==NULL) break;
		fclose(fp);
	}

	return buffer;
}

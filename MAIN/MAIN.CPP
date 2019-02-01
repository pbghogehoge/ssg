#include "..\\Gian07SrcFiles\\GIAN.h"
#include <direct.h>

extern const char *APP_CLASS = "GIAN07";
extern const char *APP_NAME  = "秋霜玉";


// プロトタイプ宣言 //
void XMain(void);
BOOL XInit(void);
void XCleanup(void);

long FAR PASCAL WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static BOOL AppInit(HINSTANCE hInstance,int nCmdShow);
static void AppCleanup(void);

//void SaveSnapshot(HWND hWnd);


// グローバル変数 //
HWND	hWndMain;
BOOL	bIsActive;
BOOL	bMouseVisible;
DWORD	WaitTime;


int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	MSG			msg;
	HWND		old_gian;
	DWORD		tempdw;

	// 他のところで起動していたらそいつをRestoreする //
	if(old_gian=FindWindow(APP_CLASS,NULL)){
		SetForegroundWindow(old_gian);
		SendMessage(old_gian,WM_SYSCOMMAND,SC_RESTORE,0);
		return FALSE;
	}

	// 引数があった場合、カレントディレクトリが変わってしまうので... //
	if(*lpCmdLine){
		char temp[1000],*p;
		strcpy(temp,GetCommandLine()+1);
		if((p=strchr(temp,'\"')) && (strchr(temp,'\\'))){
			while(*p!='\\') p--;
			*(p+1)='\0';
			_chdir(temp);
		}
		else return FALSE;
	}

	if(!AppInit(hInstance,nShowCmd)){
		return FALSE;
	}

	while(1){
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)){
			if(!GetMessage(&msg,NULL,0,0)){
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(bIsActive){
			// １フレーム進める //
			tempdw = timeGetTime();
			if(tempdw-WaitTime>=16 || ConfigDat.DrawMode==0){
				GameMain();
				WaitTime = tempdw;
			}
			//else Sleep(1);
		}
		else{
			WaitMessage();
		}
	}
}


long FAR PASCAL WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT		ps;
	HDC				hdc;
	static	BOOL	ExitFlag = FALSE;

	switch(message){
		case WM_ACTIVATEAPP:
			bIsActive = (BOOL)wParam;
			if(bIsActive){
				bMouseVisible = FALSE;
				Mid_Play();
			}
			else{
				bMouseVisible = TRUE;
				Mid_Stop();
			}
		break;

		case WM_SETCURSOR:
			if(bMouseVisible) SetCursor(LoadCursor(NULL,IDC_ARROW));
			else              SetCursor(NULL),ShowCursor(TRUE);
		return 1;

		case WM_ERASEBKGND:
		return 1;

		case WM_PAINT:
			hdc = BeginPaint(hWnd,&ps);
			EndPaint(hWnd,&ps);
		return 1;

		// IME 関連のメッセージは無視 //
		case WM_IME_CHAR:		case WM_IME_COMPOSITION:		case WM_IME_COMPOSITIONFULL:
		case WM_IME_CONTROL:	case WM_IME_ENDCOMPOSITION:		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:		case WM_IME_NOTIFY:				case WM_IME_SELECT:
		case WM_IME_SETCONTEXT:	case WM_IME_STARTCOMPOSITION:

		#if(WINVER >= 0x0500)
			case WM_IME_REQUEST:
		#endif

		return 1;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE){
				ExitFlag = TRUE;
				AppCleanup();
				MoveWindow(hWndMain,0,0,0,0,TRUE);
				ShowWindow(hWndMain,SW_HIDE);
				DestroyWindow(hWndMain);
				break;
			}
			if(wParam == SC_RESTORE)	break;
		return 1;

		case WM_SYSKEYDOWN:
			if(ExitFlag) return 0;
		break;

		case WM_KEYDOWN:
		/*	switch(wParam){
				case(VK_ESCAPE):
					DestroyWindow(hWnd);
				break;
			}*/
		return 0;

		default:
		break;
	}

	return DefWindowProc(hWnd,message,wParam,lParam);
}

static BOOL AppInit(HINSTANCE hInstance,int nCmdShow)
{
	WNDCLASS	wc;
	HMENU		hMenu;

	wc.style			= CS_DBLCLKS;							//
	wc.lpfnWndProc		= WndProc;								//
	wc.cbClsExtra		= 0;									//
	wc.cbWndExtra		= 0;									//
	wc.hInstance		= hInstance;							//
	wc.hIcon			= (HICON)LoadIcon(hInstance,"NEO_TAMA_ICON");		//
	wc.hCursor			= 0;									// 後で変更
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);			//
	wc.lpszMenuName		= 0;									// 後で変更
	wc.lpszClassName	= APP_CLASS;							//

	if(!RegisterClass(&wc)){
		return FALSE;
	}

	hWndMain = CreateWindowEx(
		0,												//
		APP_CLASS,										//
		APP_NAME,										//
		(WS_VISIBLE|WS_SYSMENU|WS_EX_TOPMOST|WS_POPUP),	//
		0,												//
		0,												//
		GetSystemMetrics(SM_CXSCREEN),					//
		GetSystemMetrics(SM_CYSCREEN),					//
		NULL,											//
		NULL,											//
		hInstance,										//
		NULL);											//

	if(!hWndMain) return FALSE;

	//ShowWindow(hWndMain,nCmdShow);
	//UpdateWindow(hWndMain);
	ShowCursor(FALSE);

	hMenu = GetSystemMenu(hWndMain,FALSE);
	DeleteMenu(hMenu,SC_MAXIMIZE,MF_BYCOMMAND);
	DeleteMenu(hMenu,SC_MINIMIZE,MF_BYCOMMAND);
	DeleteMenu(hMenu,SC_MOVE    ,MF_BYCOMMAND);
	DeleteMenu(hMenu,SC_SIZE    ,MF_BYCOMMAND);


	DebugSetup("ErrLOG2.txt");
	ErrSetup();

	// キーボード(JoyPad)入力を受け付ける //
	Key_Start(hInstance,hWndMain);

	try{
		if(!GrpEnum(hWndMain))	throw(FALSE);
		if(!XInit())			throw(FALSE);		// コンフィグ依存の初期化処理
	}
	catch(BOOL flag){
		DestroyWindow(hWndMain);
		return flag;
	}

	return TRUE;
}

static void AppCleanup(void)
{
	XCleanup();

	GrpCleanup();
	SndCleanup();
	Mid_Stop();
	Mid_End();
	Key_End();

	ShowCursor(TRUE);
	ErrCleanup();
}

BOOL XInit(void)
{
	XDDRAWENUM		*pXDD;

	// コンフィグをロードする //
	ConfigLoad();

	Key_PadConfig2(ConfigDat.PadTama,   KEY_TAMA);
	Key_PadConfig2(ConfigDat.PadBomb,   KEY_BOMB);
	Key_PadConfig2(ConfigDat.PadShift,  KEY_SHIFT);
	Key_PadConfig2(ConfigDat.PadCancel, KEY_ESC);

	// グラフィックの初期化 //
	pXDD = &DxEnum[ConfigDat.DeviceID];
	if(!GrpInit(pXDD->lpDDGuid,pXDD->D3D,ConfigDat.BitDepth)) return FALSE;
	InitSurface();
	InitFonts();
	GrpSetRestoreHook(SEffectRestore);

	// サウンドの初期化 //
	if(ConfigDat.SoundFlags&SNDF_WAVE_ENABLE){
		// 何らかの理由で使用できなければ、Disable とする //
		if(!SndInit(hWndMain)) ConfigDat.SoundFlags &= (~SNDF_WAVE_ENABLE);
		else if(!LoadSound()){
			ConfigDat.SoundFlags &= (~SNDF_WAVE_ENABLE);
			SndCleanup();
		}
	}

	// ＭＩＤＩの初期化 //
	// MIDFN_MIDLOOP を使用すべきか //
	if(ConfigDat.SoundFlags&SNDF_MIDI_ENABLE){
		if(!Mid_Start(MIDFN_CALLBACK,MIDPL_NORM))
			ConfigDat.SoundFlags &= (~SNDF_MIDI_ENABLE);
	}

	if(!GameInit()) return FALSE;		// 各種初期化
	//if(!GameExit()) return FALSE;		// タイトルへ
	//if(!EndingInit()) return FALSE;
	if(!SProjectInit()) return FALSE;

	GrpSetCaptureFilename("秋霜");

	return TRUE;
}

void XCleanup(void)
{
	CleanupFont();
	CleanupLens();
	ConfigSave();
}

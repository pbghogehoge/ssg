#include "MapEdit.h"
#include <direct.h>

#define APP_CLASS	"MAP_EDITOR"
#define APP_NAME	"MapEditor2"


// プロトタイプ宣言 //
long FAR PASCAL WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static BOOL AppInit(HINSTANCE hInstance,int nCmdShow);
static void AppCleanup(void);


// グローバル変数 //
HWND	hWndMain;
BOOL	bIsActive;
BOOL	bMouseVisible;


int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	MSG			msg;
	HWND		old_gian;


	if(old_gian=FindWindow(APP_CLASS,NULL)){
		SetForegroundWindow(old_gian);
		SendMessage(old_gian,WM_SYSCOMMAND,SC_RESTORE,0);
		return FALSE;
	}

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
			EditorMain();
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

	switch(message){
		case WM_ACTIVATEAPP:
			bIsActive = (BOOL)wParam;
			if(bIsActive) bMouseVisible = FALSE;
			else{
				bMouseVisible = TRUE;
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

		case WM_IME_CHAR:		case WM_IME_COMPOSITION:		case WM_IME_COMPOSITIONFULL:
		case WM_IME_CONTROL:	case WM_IME_ENDCOMPOSITION:		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:		case WM_IME_NOTIFY:				case WM_IME_SELECT:
		case WM_IME_SETCONTEXT:	case WM_IME_STARTCOMPOSITION:

		#if(WINVER >= 0x0500)
			case WM_IME_REQUEST:
		#endif

		return 1;

		case WM_DESTROY:
			AppCleanup();
		break;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE)		break;
			if(wParam == SC_RESTORE){
				break;
			}
		return 1;

		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE){
				DestroyWindow(hWnd);
			}
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
	wc.hIcon			= (HICON)LoadIcon(hInstance,"ICON_MAPEDIT2");		//
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


	ErrSetup();

	try{
		if(!GrpEnum(hWndMain))								throw(FALSE);

		if(!GrpInit(DxEnum->lpDDGuid,DxEnum->D3D,16))
			if(!GrpInit(DxEnum->lpDDGuid,DxEnum->D3D,8))	throw(FALSE);

		if(!EditorInit())									throw(FALSE);
	}
	catch(BOOL flag){
		DestroyWindow(hWndMain);
		return flag;
	}

	return TRUE;
}

static void AppCleanup(void)
{
	SaveMapData("MAPEDIT2.DAT");

	GrpCleanup();

	ShowCursor(TRUE);
	ErrCleanup();

	PostQuitMessage(0);
}

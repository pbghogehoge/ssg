/*                                                                           */
/*   DI_UTY.cpp   Direct Input 関連の関数                                    */
/*                                                                           */
/*                                                                           */

#include "DI_UTY.H"
//#include "..\GIAN_MAIN\GIAN.H"
#pragma message(PBGWIN_DI_UTY_H)

//#pragma optimize("",off)


// グローバル変数(Private)の実体 //
INPUT_OBJ			InputObj;
InputConfig			IConfig;


// グローバル変数(Public)の実体
WORD Key_Data = 0;
WORD Pad_Data = 0;
void (*Key_Read)(void) = Key_Read_STD;


// 秘密の関数 //
WORD Pad_Read(void);



BOOL Key_Start(HINSTANCE hInst,HWND hWnd)
{
	HRESULT		dirval;

	// 初期値の準備 //
	memset(IConfig.bIsUsed,FALSE,256);
	IConfig.bIsUsed[DIK_ESCAPE] = TRUE;
	IConfig.bIsUsed[DIK_RETURN] = TRUE;
	IConfig.bIsUsed[DIK_UP]     = TRUE;
	IConfig.bIsUsed[DIK_DOWN]   = TRUE;
	IConfig.bIsUsed[DIK_LEFT]   = TRUE;
	IConfig.bIsUsed[DIK_RIGHT]  = TRUE;

	IConfig.KeyUp    = DIK_NUMPAD8;	IConfig.bIsUsed[DIK_NUMPAD8] = TRUE;
	IConfig.KeyDown  = DIK_NUMPAD2;	IConfig.bIsUsed[DIK_NUMPAD2] = TRUE;
	IConfig.KeyLeft  = DIK_NUMPAD4;	IConfig.bIsUsed[DIK_NUMPAD4] = TRUE;
	IConfig.KeyRight = DIK_NUMPAD6;	IConfig.bIsUsed[DIK_NUMPAD6] = TRUE;

	IConfig.KeyULeft  = DIK_NUMPAD7;	IConfig.bIsUsed[DIK_NUMPAD7] = TRUE;
	IConfig.KeyURight = DIK_NUMPAD9;	IConfig.bIsUsed[DIK_NUMPAD9] = TRUE;
	IConfig.KeyDLeft  = DIK_NUMPAD1;	IConfig.bIsUsed[DIK_NUMPAD1] = TRUE;
	IConfig.KeyDRight = DIK_NUMPAD3;	IConfig.bIsUsed[DIK_NUMPAD3] = TRUE;

	IConfig.KeyTama  = DIK_Z;		IConfig.bIsUsed[DIK_Z]       = TRUE;
	IConfig.KeyBomb  = DIK_X;		IConfig.bIsUsed[DIK_X]       = TRUE;
	IConfig.KeyShift = DIK_LSHIFT;	IConfig.bIsUsed[DIK_LSHIFT]  = TRUE;

	// DirectInput オブジェクトの作成 //
	dirval = DirectInputCreate(hInst,DIRECTINPUT_VERSION,&(InputObj.pdi),NULL);
	if(dirval != DI_OK){
		dirval = DirectInputCreate(hInst,0x0300,&(InputObj.pdi),NULL);
		if(dirval != DI_OK) return FALSE;
	}

	// DirectInput デバイス (Keyboard) の作成 //
	dirval = InputObj.pdi->CreateDevice(GUID_SysKeyboard,&(InputObj.pdev),NULL);
	if(dirval != DI_OK){
		Key_End();
		return FALSE;
	}

	// 協調レベルのセット //
	dirval = InputObj.pdev->SetCooperativeLevel(hWnd,DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
	if(dirval != DI_OK){
		Key_End();
		return FALSE;
	}

	// キーボードのデータフォーマットをセットする //
	dirval = InputObj.pdev->SetDataFormat(&c_dfDIKeyboard);
	if(dirval != DI_OK){
		Key_End();
		return FALSE;
	}

	dirval = InputObj.pdev->Acquire();
	if(dirval != DI_OK){
		Key_End();
		return FALSE;
	}

	// ＤＩを使用した読み込み //
	Key_Read = Key_Read_DI;

	return TRUE;
}

void Key_End(void)
{
	/*
	char	buf[100];

	sprintf(buf,"Dev:%x  DI:%x",InputObj.pdev,InputObj.pdi);
	ErrInsert(buf);
*/
	if(InputObj.pdev){
		InputObj.pdev->Unacquire();
		InputObj.pdev->Release();
		InputObj.pdev = NULL;
	}

	if(InputObj.pdi){
		InputObj.pdi->Release();
		InputObj.pdi = NULL;
	}

	Key_Read = Key_Read_STD;
}

extern void Key_Read_STD(void)
{
	WORD		val = 0;
//	JOYINFOEX	joy;
//	UINT		x,y;
	BYTE		buf[256];

	GetKeyboardState(buf);

	if(buf[VK_UP]&0x80)			val |= KEY_UP;
	if(buf[VK_NUMPAD8]&0x80)	val |= KEY_UP;
	if(buf[VK_DOWN]&0x80)		val |= KEY_DOWN;
	if(buf[VK_NUMPAD2]&0x80)	val |= KEY_DOWN;
	if(buf[VK_LEFT]&0x80)		val |= KEY_LEFT;
	if(buf[VK_NUMPAD4]&0x80)	val |= KEY_LEFT;
	if(buf[VK_RIGHT]&0x80)		val |= KEY_RIGHT;
	if(buf[VK_NUMPAD6]&0x80)	val |= KEY_RIGHT;

	if(buf[VK_NUMPAD7]&0x80)	val |= (KEY_UP|KEY_LEFT);
	if(buf[VK_NUMPAD9]&0x80)	val |= (KEY_UP|KEY_RIGHT);
	if(buf[VK_NUMPAD1]&0x80)	val |= (KEY_DOWN|KEY_LEFT);
	if(buf[VK_NUMPAD3]&0x80)	val |= (KEY_DOWN|KEY_RIGHT);

	if(buf['Z']&0x80)			val |= KEY_TAMA;
	if(buf[VK_SPACE]&0x80)		val |= KEY_TAMA;
	if(buf['X']&0x80)			val |= KEY_BOMB;

	if(buf[VK_SHIFT]&0x80)		val |= KEY_SHIFT;

	if(buf[VK_RETURN]&0x80)		val |= KEY_RETURN;
	if(buf[VK_ESCAPE]&0x80)		val |= KEY_ESC;

	if(val == 0){
		Pad_Data = Pad_Read();
	}

	Key_Data = val | Pad_Data;
}

extern void Key_Read_DI(void)
{
	static BYTE		buf[300];

	HRESULT		dirval;
	WORD		val = 0;

	dirval = InputObj.pdev->GetDeviceState(256,(LPVOID)buf);
	if(dirval != DI_OK){
		if(dirval == DIERR_INPUTLOST){
			dirval = InputObj.pdev->Acquire();
			if(dirval==DI_OK) return;
		}

		// キー入力ＡＰＩを通常に戻す //
		Key_End();
		return;
	}

	if(buf[DIK_UP]&0x80)			val |= KEY_UP;
	if(buf[IConfig.KeyUp]&0x80)		val |= KEY_UP;

	if(buf[DIK_DOWN]&0x80)			val |= KEY_DOWN;
	if(buf[IConfig.KeyDown]&0x80)	val |= KEY_DOWN;

	if(buf[DIK_LEFT]&0x80)			val |= KEY_LEFT;
	if(buf[IConfig.KeyLeft]&0x80)	val |= KEY_LEFT;
	if(buf[DIK_RIGHT]&0x80)			val |= KEY_RIGHT;
	if(buf[IConfig.KeyRight]&0x80)	val |= KEY_RIGHT;

	if(buf[IConfig.KeyULeft]&0x80)	val |= KEY_ULEFT;
	if(buf[IConfig.KeyURight]&0x80)	val |= KEY_URIGHT;
	if(buf[IConfig.KeyDLeft]&0x80)	val |= KEY_DLEFT;
	if(buf[IConfig.KeyDRight]&0x80)	val |= KEY_DRIGHT;

	if(buf[IConfig.KeyTama]&0x80)	val |= KEY_TAMA;
	//if(buf[DIK_SPACE]&0x80)		val |= KEY_TAMA;
	if(buf[IConfig.KeyBomb]&0x80)	val |= KEY_BOMB;

	if(buf[IConfig.KeyShift]&0x80)	val |= KEY_SHIFT;

	if(buf[DIK_RETURN]&0x80)	val |= KEY_RETURN;
	if(buf[DIK_ESCAPE]&0x80)	val |= KEY_ESC;

	if(val == 0){
		Pad_Data = Pad_Read();
	}

	Key_Data = val | Pad_Data;
}


WORD Pad_Read(void)
{
	JOYINFOEX	joy;
	int			x,y;
	WORD		val = 0;

	joy.dwSize  = sizeof(joy);
	joy.dwFlags = JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY;
	if(joyGetPosEx(JOYSTICKID1, &joy) == JOYERR_NOERROR){
		x = joy.dwXpos>>11;
		y = joy.dwYpos>>11;

		if(joy.dwButtons&IConfig.PadMask_Tama)		val |= KEY_TAMA;	// JOY_BUTTON1?
		if(joy.dwButtons&IConfig.PadMask_Bomb)		val |= KEY_BOMB;	// JOY_BUTTON2?
		if(joy.dwButtons&IConfig.PadMask_Shift)		val |= KEY_SHIFT;	// JOY_BUTTON5?
		if(joy.dwButtons&IConfig.PadMask_Cancel)	val |= KEY_ESC;		// JOY_BUTTONXX

		if(x<=12)	val |= KEY_LEFT;
		if(x>=20)	val |= KEY_RIGHT;
		if(y<=12)	val |= KEY_UP;
		if(y>=20)	val |= KEY_DOWN;

		return val;
	}

	return 0;
}


extern int Key_PadConfig(WORD Key)
{
	JOYINFOEX	joy;
	DWORD		mask;
	int			ret;

	joy.dwSize  = sizeof(joy);
	joy.dwFlags = JOY_RETURNBUTTONS|JOY_RETURNX|JOY_RETURNY;
	if(joyGetPosEx(JOYSTICKID1, &joy) != JOYERR_NOERROR) return 0;

	mask = joy.dwButtons;

	switch(mask){
		case JOY_BUTTON1:		ret = 1;	break;
		case JOY_BUTTON2:		ret = 2;	break;
		case JOY_BUTTON3:		ret = 3;	break;
		case JOY_BUTTON4:		ret = 4;	break;
		case JOY_BUTTON5:		ret = 5;	break;
		case JOY_BUTTON6:		ret = 6;	break;
		case JOY_BUTTON7:		ret = 7;	break;
		case JOY_BUTTON8:		ret = 8;	break;
		case JOY_BUTTON9:		ret = 9;	break;
		case JOY_BUTTON10:		ret = 10;	break;
		case JOY_BUTTON11:		ret = 11;	break;
		case JOY_BUTTON12:		ret = 12;	break;
		case JOY_BUTTON13:		ret = 13;	break;
		case JOY_BUTTON14:		ret = 14;	break;
		case JOY_BUTTON15:		ret = 15;	break;
		case JOY_BUTTON16:		ret = 16;	break;
		case JOY_BUTTON17:		ret = 17;	break;
		case JOY_BUTTON18:		ret = 18;	break;
		case JOY_BUTTON19:		ret = 19;	break;
		case JOY_BUTTON20:		ret = 20;	break;
		case JOY_BUTTON21:		ret = 21;	break;
		case JOY_BUTTON22:		ret = 22;	break;
		case JOY_BUTTON23:		ret = 23;	break;
		case JOY_BUTTON24:		ret = 24;	break;
		case JOY_BUTTON25:		ret = 25;	break;
		case JOY_BUTTON26:		ret = 26;	break;
		case JOY_BUTTON27:		ret = 27;	break;
		case JOY_BUTTON28:		ret = 28;	break;
		case JOY_BUTTON29:		ret = 29;	break;
		case JOY_BUTTON30:		ret = 30;	break;
		case JOY_BUTTON31:		ret = 31;	break;
		case JOY_BUTTON32:		ret = 32;	break;

		case 0:		return -1;

		default:	// 同時押しで、無効化できる //
			ret = 0;	mask = 0;
		break;
	}
/*
	for(ret=1; ret<=32; ret++){
		val = 1 << (ret-1);
		if(val == joy.dwButtons) break;
	}

	if(ret == 33){
		val = 0;
		ret = 0;
	}
*/
	switch(Key){
		case(KEY_TAMA):		IConfig.PadMask_Tama   = mask;	break;
		case(KEY_BOMB):		IConfig.PadMask_Bomb   = mask;	break;
		case(KEY_SHIFT):	IConfig.PadMask_Shift  = mask;	break;
		case(KEY_ESC):		IConfig.PadMask_Cancel = mask;	break;

		default:	return 0;
	}

	return ret;
}

extern void Key_PadConfig2(BYTE PadID, WORD KeyData)
{
	DWORD		temp;

	switch(PadID){
		case 0:		temp = 0;			break;

		case 1:		temp = JOY_BUTTON1;		break;
		case 2:		temp = JOY_BUTTON2;		break;
		case 3:		temp = JOY_BUTTON3;		break;
		case 4:		temp = JOY_BUTTON4;		break;
		case 5:		temp = JOY_BUTTON5;		break;
		case 6:		temp = JOY_BUTTON6;		break;
		case 7:		temp = JOY_BUTTON7;		break;
		case 8:		temp = JOY_BUTTON8;		break;
		case 9:		temp = JOY_BUTTON9;		break;
		case 10:	temp = JOY_BUTTON10;	break;
		case 11:	temp = JOY_BUTTON11;	break;
		case 12:	temp = JOY_BUTTON12;	break;
		case 13:	temp = JOY_BUTTON13;	break;
		case 14:	temp = JOY_BUTTON14;	break;
		case 15:	temp = JOY_BUTTON15;	break;
		case 16:	temp = JOY_BUTTON16;	break;
		case 17:	temp = JOY_BUTTON17;	break;
		case 18:	temp = JOY_BUTTON18;	break;
		case 19:	temp = JOY_BUTTON19;	break;
		case 21:	temp = JOY_BUTTON20;	break;
		case 22:	temp = JOY_BUTTON21;	break;
		case 23:	temp = JOY_BUTTON22;	break;
		case 24:	temp = JOY_BUTTON23;	break;
		case 25:	temp = JOY_BUTTON24;	break;
		case 26:	temp = JOY_BUTTON26;	break;
		case 27:	temp = JOY_BUTTON27;	break;
		case 28:	temp = JOY_BUTTON28;	break;
		case 29:	temp = JOY_BUTTON29;	break;
		case 30:	temp = JOY_BUTTON30;	break;
		case 31:	temp = JOY_BUTTON31;	break;
		case 32:	temp = JOY_BUTTON32;	break;

		default:	return;
	}

//	if(PadID == 0) temp = 0;
//	else           temp = 1 << (PadID-1);

	switch(KeyData){
		case(KEY_TAMA):		IConfig.PadMask_Tama   = temp;	break;
		case(KEY_BOMB):		IConfig.PadMask_Bomb   = temp;	break;
		case(KEY_SHIFT):	IConfig.PadMask_Shift  = temp;	break;
		case(KEY_ESC):		IConfig.PadMask_Cancel = temp;	break;
	}

	return;

	//IConfig.PadMask_Tama   = JOY_BUTTON1;
	//IConfig.PadMask_Bomb   = JOY_BUTTON2;
	//IConfig.PadMask_Shift  = JOY_BUTTON5;
	//IConfig.PadMask_Cancel = 0;
}


//#pragma optimize("",on)

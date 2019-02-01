/*                                                                           */
/*   DX_TYPE.h   ＤＸ関連の定数とか                                          */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DX_TYPE_H
#define PBGWIN_DX_TYPE_H		"DX_TYPE : Version 0.02"


// 更新履歴 //

// 2000/05/15 : 関数呼び出し規約 FINT 等を追加
// 2000/01/30 : バージョン文字列を出力するようにした
// 1999/12/02 : ＤＤ＿ＵＴＹから分離する


// ヘッダファイル //
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <dsound.h>
#include <dinput.h>
#include <stdio.h>


// 使用するライブラリ //
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"d3dim.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dinput.lib")
#pragma comment(lib,"winmm.lib")


// ヘッダファイルのバージョンを表示する //
#pragma message("-----["PBGWIN_DX_TYPE_H " / " __TIMESTAMP__ "]-----")

#if   DIRECTDRAW_VERSION == 0x0700
	#pragma message("      DirectDraw  : Version 7.00")
#elif DIRECTDRAW_VERSION == 0x0600
	#pragma message("      DirectDraw  : Version 6.00")
#elif DIRECTDRAW_VERSION == 0x0500
	#pragma message("      DirectDraw  : Version 5.00")
#elif DIRECTDRAW_VERSION == 0x0300
	#pragma message("      DirectDraw  : Version 3.00")
#endif

#if   DIRECTSOUND_VERSION == 0x0700
	#pragma message("      DirectSound : Version 7.00")
#elif DIRECTSOUND_VERSION == 0x0600
	#pragma message("      DirectSound : Version 6.00")
#elif DIRECTSOUND_VERSION == 0x0500
	#pragma message("      DirectSound : Version 5.00")
#elif DIRECTSOUND_VERSION == 0x0300
	#pragma message("      DirectSound : Version 3.00")
#else
	#pragma message("      DirectSound : Version <= 6.00???")
#endif

#if   DIRECTINPUT_VERSION == 0x0700
	#pragma message("      DirectInput : Version 7.00")
#elif DIRECTINPUT_VERSION == 0x0600
	#pragma message("      DirectInput : Version 6.00")
#elif DIRECTINPUT_VERSION == 0x0500
	#pragma message("      DirectInput : Version 5.00")
#elif DIRECTINPUT_VERSION == 0x0300
	#pragma message("      DirectInput : Version 3.00")
#endif

#pragma message("------------------------------")


// マクロ //
#define RELEASE(x)		if((x)!=NULL){(x)->Release();x=NULL;}
#define ZEROMEM(x)		memset(&x,0,sizeof(x))


// 型 //
typedef	LPDIRECTDRAWSURFACE	GRP;

#define PVOID	void pascal
#define PBOOL	BOOL pascal
#define PGRP	GRP  pascal
#define PINT	int  pascal

#define PVOIDFN(fn)		void (pascal fn)
#define PBOOLFN(fn)		BOOL (pascal fn)
#define PGRPFN(fn)		GRP  (pascal fn)

#define FVOID	void __fastcall
#define FBOOL	BOOL __fastcall
#define FGRP	GRP  __fastcall

#define FCHAR	char	__fastcall
#define FINT	int		__fastcall
#define FSHORT	short	__fastcall

#define FBYTE	BYTE	__fastcall
#define FWORD	WORD	__fastcall
#define FDWORD	DWORD	__fastcall

#define FVOIDFN(fn)		void (__fastcall fn)
#define FBOOLFN(fn)		BOOL (__fastcall fn)
#define FGRPFN(fn)		GRP  (__fastcall fn)

#define IVOID	__inline void
#define IBOOL	__inline BOOL
#define IGRP	__inline GRP
#define IINT	__inline INT
#define IBYTE	__inline BYTE
#define IWORD	__inline WORD
#define IDWORD	__inline DWORD

#define FIVOID	__forceinline void
#define FIBOOL	__forceinline BOOL
#define FIGRP	__forceinline GRP

#endif

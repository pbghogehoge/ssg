/*                                                                           */
/*   PBGUTY_X.h   各種汎用関数詰め合わせ                                     */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_PBGUTY_X_H
#define PBGWIN_PBGUTY_X_H
/*
#ifndef DIRECTDRAW_VERSION
	#define DIRECTDRAW_VERSION		0x0500
#endif

#ifndef DIRECTSOUND_VERSION
	#define DIRECTSOUND_VERSION		0x0500
#endif

#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION		0x0500
#endif
*/

#include "DX_TYPE.h"			// 型を定義
#include "DD_UTY.h"				// DDraw & D3D
#include "DS_UTY.h"				// DSound
#include "DI_UTY.h"				// DInput
#include "UT_MATH.h"			// 整数用数学関数
#include "PBGMIDI.h"			// MIDI 操作


#ifdef _DEBUG
	#pragma message("PBGUTY_X(DEBUG)\n")
#else
	#pragma message("PBGUTY_X(Release)\n")
#endif


#endif

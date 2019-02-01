/*                                                                           */
/*   PBGMIDI.h   ＭＩＤＩ管理用関数                                          */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_PBGMIDI_H
#define PBGWIN_PBGMIDI_H	"PBGMIDI : Version 0.31 : Update 2000/08/04"

// 更新履歴 //
// 2000/08/04 : Mid_Free() をグローバルにした。

// 2000/03/22 : MIDI フェードアウト関数をマスターボリュームからＣＣのボリュームに変更
//            : 処理の追いつかない MIDI があった場合は、メッセージ送出に工夫が必要かも




// C++ 対策 //
#ifdef __cplusplus
extern "C" {
#endif


#include <windows.h>
#include <mmsystem.h>
//#pragma comment(lib,"winmm.lib")
#include "LZ_UTY.h"


#define MIDFN_NOPLAY	0x00	// 再生しない
#define MIDFN_CALLBACK	0x01	// CALLBACK_FN を使用して再生
#define MIDFN_MIDLOOP	0x02	// ゲーム等の一定スピードのループで再生
#define MIDFN_STREAM	0x04	// MIDI_STREAM を使用して再生(予約のみ)

#define MIDPL_NORM		0x00	// 曲の最後にきたら最初に戻って再生
#define MIDPL_STOP		0x01	// 曲の最後にきたら停止

#define MIDST_STOP		0x00	// 停止している
#define MIDST_PLAY		0x01	// 再生中
#define MIDST_PAUSE		0x02	// 一時停止

#define MID_STDTEMPO	(1<<7)	// 標準のテンポ


//// みでぃ用構造体 ////
typedef struct{
	HMIDIOUT	mp;					// 出力デバイスのハンドル
	WORD		state;				// 現在の状態
	WORD		fnmode;				// MIDI再生に使用する関数
	WORD		plmode;				// 曲の最後に達したときどうするか
	int			nDevice;			// デバイス数
	int			NowID;				// 現在のデバイスのＩＤ
	char		**name;				// デバイス名

	// 以下は外部から変更＆参照しないこと //
	DWORD		FadeCount;			// フェードＩ／Ｏカウンタ
	char		FadeFlag;			// フェードＩ／Ｏフラグ(In or Out or 無し)
	int			FadeWait;			// フェードＩ／Ｏウェイト

	BYTE		MaxVolume;			// ボリュームの最大値(メッセージでも変化,0-127)
	BYTE		NowVolume;			// 現在のボリューム(0-127)
} MID_DEVICE;

typedef struct {
	DWORD	MThd;
	DWORD	size;
} MID_FILEST;

typedef struct {
	WORD	format;
	WORD	track;
	WORD	timebase;
} MID_MAINST;

typedef struct {
	DWORD	MTrk;
	DWORD	size;
} MID_TRACKST;

typedef struct {
	BOOL	play;
	int		count;
	int		size;
	BYTE	status;
	LPBYTE	data;
	LPBYTE	work;
} MID_TRACK;

typedef struct {
	int			format;
	int			track;
	int			timebase;
	int			tempo;
	DWORDLONG	playcount1;
	DWORDLONG	playcount2;
	DWORDLONG	fticks;
	DWORDLONG	nticks;
	MID_TRACK	*data;
} MID_DATA;

//// たいまー用構造体 ////
typedef struct {
	UINT		htimer;
	TIMECAPS	caps;
	UINT		delay;
} TIME_DATA;


//// 関数 ////
BOOL Mid_Start(WORD fnmode,WORD plmode);	// ＭＩＤＩ関連初期化
void Mid_End(void);							// ＭＩＤＩ関連おしまい
void Mid_Play(void);						// 再生する
void Mid_Stop(void);						// 停止する
BOOL Mid_Free(void);						// データ解放する
void Mid_Volume(BYTE volume);				// マスターボリュームを変更する
void Mid_Tempo(char tempo);					// テンポを変更する
void Mid_FadeOut(BYTE speed);				// フェードアウト(数字が大きいほど早い)
void Mid_Pan(char pan);						// 全てのチャンネルのパンを変更する(無駄関数)
BOOL Mid_ChgDev(char pos);					// 出力デバイスを変更する

BOOL Mid_Load(char *filename);				// 普通のＭＩＤＩファイルをロードする
BOOL PMid_Load(BIT_DEVICE *in,DWORD n);		// 圧縮ＭＩＤＩファイルをロードする

void Mid_LoopFunc(DWORD time);				// 一定間隔ループを自前で生成して再生するとき使用
char *Mid_GetTitle(void);					// この曲のお名前は？

void Mid_TableInit(void);					// 各種テーブルの初期化

DWORDLONG Mid_GetPlaycount1(void);		// playcount1 の取得
DWORDLONG Mid_GetPlaycount2(void);		// playcount2 の取得
DWORDLONG Mid_GetTicks(void);			// tick の取得

void Mid_GetData(MID_DATA *pData);		// 全情報を取得



//// グローバル変数 ////
extern MID_DEVICE	Mid_Dev;
extern WORD			Mid_PlayTable[16][128];
extern WORD			Mid_PlayTable2[16][128];		// レベルメーター用
extern BYTE			Mid_NoteTable[16][128];			// ノート表示用
extern BYTE			Mid_NoteWTable[16][128];		// ノート表示用(2)
extern BYTE			Mid_PanpodTable[16];			// パンポット
extern BYTE			Mid_ExpressionTable[16];		// エクスプレッション
extern BYTE			Mid_VolumeTable[16];			// ボリューム
extern DWORD		Mid_PlayTime;


// C++ 対策 //
#ifdef __cplusplus
}
#endif


#endif

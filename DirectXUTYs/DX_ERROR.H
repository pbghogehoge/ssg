/*                                                                           */
/*   DX_ERROR.h   DirectX のエラー出力用関数                                 */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_DX_ERROR_H
#define PBGWIN_DX_ERROR_H		"DX_ERROR : Version 0.01 : Update 1999/11/20"


// 更新履歴 //

// 1999/12/10 : ファイルにエラー出力をする関数を追加
// 1999/11/20 : 作成開始


// C++ 対策 //
#ifdef __cplusplus
extern "C" {
#endif


// ヘッダファイル //
#include "DX_TYPE.h"



// 構造体＆共用体 //
typedef struct _error{
	char			*s;			// エラー文字列
	struct _error	*next;		// 次のエラー
} ERROR_DATA;


// 関数プロトタイプ宣言 //
extern void ErrSetup(void);				// エラー用バッファの初期化
extern void ErrInsert(char *s);			// エラー文字列を追加
extern void ErrCleanup(void);			// エラー文字列出力＆メモリ解放
extern void DebugSetup(char *filename);	// エラー出力準備(->LogFile)
extern void DebugCleanup(void);			// エラー吐き出し用ファイルを閉じる
extern void DebugOut(char *s);			// デバッグメッセージ吐き出し

// C++ 対策 //
#ifdef __cplusplus
}
#endif


#endif

/*                                                                           */
/*   DD_CLIP2D.cpp   ２Ｄ図形描画用クリッピング                              */
/*                                                                           */
/*                                                                           */

#include "DD_CLIP2D.h"
#pragma message(PBGWIN_DD_CLIP2D_H)


#define OUT_LEFT	0x01	//0001B
#define OUT_RIGHT	0x02	//0010B
#define OUT_X		0x03	//0011B
#define OUT_TOP		0x08	//1000B
#define OUT_BOTTOM	0x04	//0100B
#define OUT_Y		0x0c	//1100B

/* Grp_PClip のｘクリップ用マクロ */
#define GETOUT_X(ans,x,x1,x2,label)		\
	_asm MOV ans,OUT_LEFT				\
	_asm CMP x1,x						\
	_asm JG  SHORT label				\
	_asm XOR ans,OUT_X					\
	_asm CMP x,x2						\
	_asm JG  SHORT label				\
	_asm XOR ans,ans					\
	_asm label:

/* Grp_PClip のｙクリップ用マクロ */
#define GETOUT_Y(ans,y,y1,y2,label)		\
	_asm MOV ans,OUT_BOTTOM				\
	_asm CMP y,y2						\
	_asm JG  SHORT label				\
	_asm XOR ans,OUT_Y					\
	_asm CMP y,y1						\
	_asm JL  SHORT label				\
	_asm XOR ans,ans					\
	_asm label:


extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;


#pragma warning(disable : 4035)
int pascal Grp_PClip(POINT *dest,POINT *src,int n)
{
	int d,oc,x,y,radr,lastout,i;

	_asm{
		PUSH EDI				;
		PUSH ESI				;

		MOV EBX,n				;
		DEC EBX					; 点の数からループ回数を計算する
		SHL EBX,3				;
		MOV ESI,src

		MOV EAX,[ESI+EBX]		; ｘ座標のロード(src[n-1].x)
		MOV EDX,EAX				; 最も左の座標の初期化
		MOV ECX,EAX				; 最も右の座標の初期化

		MOV EAX,[ESI+EBX+4]		; ｙ座標のロード(src[n-1].y)
		MOV EDI,EAX				; 最も上の座標の初期化
		MOV d,EAX				; 最も下の座標の初期化
		SUB EBX,8				; 次の点へ

LP1_0:							; 最も左にあるｘ座標をチェック
		MOV EAX,[ESI+EBX]
		CMP EAX,EDX
		JG  SHORT LP1_1
		MOV EDX,EAX
LP1_1:							; 最も右にあるｘ座標をチェック
		CMP EAX,ECX
		JL  SHORT LP1_2
		MOV ECX,EAX
LP1_2:							; 最も上にあるｙ座標をチェック
		MOV EAX,[ESI+EBX+4]
		CMP EAX,EDI
		JG  SHORT LP1_3
		MOV EDI,EAX
LP1_3:							; 最も下にあるｙ座標をチェック
		CMP EAX,d
		JLE SHORT LP1_4
		MOV d,EAX
LP1_4:							; ループの最後か？
		SUB EBX,8
		JGE SHORT LP1_0

		CMP CLIP_X_MIN,ECX		;
		JG  SHORT FIN_OUT		;
		CMP EDX,CLIP_X_MAX		;
		JG  SHORT FIN_OUT		; 完全に外側なら FIN_OUT へ
		CMP CLIP_Y_MAX,EDI		;
		JL  SHORT FIN_OUT		;
		MOV EAX,d				;
		CMP CLIP_Y_MIN,EAX		;
		JG  SHORT FIN_OUT		;

		CMP EDX,CLIP_X_MIN		;
		JL  SHORT LP2_0			;
		CMP ECX,CLIP_X_MAX		;
		JG  SHORT LP2_0			;
		CMP EDI,CLIP_Y_MIN		; 完全に内側でなければ LP2_0 へ
		JL  SHORT LP2_0			;
		MOV EAX,CLIP_Y_MAX		;
		CMP d,EAX				;
		JG  SHORT LP2_0			;

		; オフセット＆セグメントチェックは省略

		MOV EAX,-1				; 完全に内側の場合
		JMP POL_END
FIN_OUT:						; 完全に外側の場合
		XOR EAX,EAX
		JMP POL_END
LP2_0:
		MOV EAX,n
		MOV ESI,20; 結果の点の個数(ダミーです)
		MOV EBX,src
		MOV EAX,[EBX]
		MOV x,EAX
		MOV EAX,[EBX+4]
		MOV y,EAX
		GETOUT_Y(EBX,EAX,CLIP_Y_MIN,CLIP_Y_MAX,label_000)
		MOV lastout,EBX
		MOV EDI,n
		DEC EDI
		JNS SHORT LP2_1
		JMP LP3_0
LP2_1:
		MOV EAX,EDI
		SHL EAX,3
		ADD EAX,src
		MOV radr,EAX
		MOV i,EDI
		MOV EDI,y
LP2_2:
		MOV EDX,x
		MOV ECX,EDI
		MOV EBX,radr	// EBX は３２ビットポインタとして動作するのだ
		//MOV ES,[BP+src+2]
		MOV EAX,[EBX]
		MOV x,EAX
		MOV EDI,[EBX+4]	// 2->4
		CMP EDX,EAX
		JNE SHORT LP2_3
		CMP ECX,EDI
		JNE SHORT LP2_3
		JMP LP2_B
LP2_3:
		SUB EDX,EAX
		SUB ECX,EDI
		GETOUT_Y(EAX,EDI,CLIP_Y_MIN,CLIP_Y_MAX,label_001)
		MOV oc,EAX
		MOV EBX,lastout
		CMP EAX,EBX
		JE  SHORT LP2_9
		MOV d,EDX
		OR  EBX,EBX
		JE  SHORT LP2_6
		CMP EBX,OUT_TOP
		JNE SHORT LP2_4
		MOV EAX,DWORD PTR CLIP_Y_MIN
		JMP SHORT LP2_5
LP2_4:
		MOV EAX,DWORD PTR CLIP_Y_MAX
LP2_5:
		CMP  EAX,EDI
		JE   SHORT LP2_6
		DEC  ESI
		MOV  EBX,ESI
		SHL  EBX,3				// 2->3
		ADD  EBX,dest
		//MOV  ES,[BP+dest+2]
		MOV  [EBX+4],EAX		// 2->4
		SUB  EAX,EDI
		IMUL EDX
		IDIV ECX
		ADD  EAX,x
		MOV  [EBX],EAX
LP2_6:
		MOV EBX,oc
		OR  EBX,EBX
		JE  SHORT LP2_9
		CMP EBX,OUT_TOP
		JNE SHORT LP2_7
		MOV EAX,DWORD PTR CLIP_Y_MIN
		JMP SHORT LP2_8
		EVEN
LP2_7:
		MOV EAX,DWORD PTR CLIP_Y_MAX
LP2_8:
		MOV  EDX,ECX
		ADD  EDX,EDI
		CMP  EDX,EAX
		JE   SHORT LP2_9
		DEC  ESI
		MOV  EBX,ESI
		SHL  EBX,3			// 2->3
		ADD  EBX,dest
		//MOV  ES,[BP+dest+2]
		MOV  [EBX+4],EAX	// 2->4
		SUB  EAX,EDI
		IMUL DWORD PTR d	// WORD->DWORD
		IDIV ECX
		ADD  EAX,x
		MOV  [EBX],EAX
LP2_9:
		MOV ECX,oc
		OR  ECX,ECX
		JNE SHORT LP2_A
		DEC ESI
		MOV EBX,ESI
		SHL EBX,3			// 2->3
		ADD EBX,dest
		//MOV ES,[BP+dest+2]
		MOV EAX,x
		MOV [EBX],EAX
		MOV [EBX+4],EDI		// 2->4
LP2_A:
		MOV lastout,ECX
LP2_B:
		SUB DWORD PTR radr,8	// WORD->DWORD  4->8 前のアドレスに変更かな？
		DEC DWORD PTR i			// WORD->DWORD
		JS  SHORT LP3_0			// WORD->DWORD
		JMP LP2_2
LP3_0:
		MOV ECX,20
		MOV EBX,ECX
		SHL EBX,3				// 2->3
		ADD EBX,dest
		//MOV ES,[BP+dest+2]
		MOV EAX,[EBX-4]			// 2->4
		MOV y,EAX
		MOV EDI,[EBX-8]			// 4->8
		GETOUT_X(EAX,EDI,CLIP_X_MIN,CLIP_X_MAX,label_002)
		MOV lastout,EAX
		CMP ESI,ECX
		JL  SHORT LP3_1
		XOR EAX,EAX
		JMP POL_END
		EVEN
LP3_1:
		MOV EAX,ESI
		SHL EAX,3				// 2->3
		ADD EAX,dest
		MOV radr,EAX
		MOV EAX,ECX
		SUB EAX,ESI
		MOV i,EAX
		MOV ESI,0
LP3_2:
		MOV ECX,EDI
		MOV EAX,y
		MOV EDX,EAX
		MOV EBX,radr
		MOV EDI,[EBX]
		MOV EAX,[EBX+4]			// 2->4
		MOV y,EAX
		SUB ECX,EDI
		SUB EDX,EAX
		MOV EBX,EDI
		GETOUT_X(EAX,EBX,CLIP_X_MIN,CLIP_X_MAX,label_003)
		MOV oc,EAX
		MOV EBX,lastout
		CMP EAX,EBX
		JE  SHORT LP3_8
		MOV d,EDX
		OR  EBX,EBX
		JE  SHORT LP3_5
		CMP EBX,OUT_LEFT
		JNE SHORT LP3_3
		MOV EAX,DWORD PTR CLIP_X_MIN
		JMP SHORT LP3_4
		EVEN
LP3_3:
		MOV EAX,DWORD PTR CLIP_X_MAX
LP3_4:
		CMP  EAX,EDI
		JE   SHORT LP3_5
		MOV  EBX,ESI
		SHL  EBX,3				// 2->3
		ADD  EBX,dest
		MOV  [EBX],EAX
		SUB  EAX,EDI
		IMUL EDX
		IDIV ECX
		ADD  EAX,y
		MOV  [EBX+4],EAX		// 2->4
		INC  ESI
LP3_5:
		MOV EBX,oc
		OR  EBX,EBX
		JE  SHORT LP3_8
		CMP EBX,OUT_LEFT
		JNE SHORT LP3_6
		MOV EAX,DWORD PTR CLIP_X_MIN
		JMP SHORT LP3_7
LP3_6:
		MOV EAX,DWORD PTR CLIP_X_MAX
LP3_7:
		MOV  EBX,ECX
		ADD  EBX,EDI
		CMP  EBX,EAX
		JE   SHORT LP3_8
		MOV  EBX,ESI
		SHL  EBX,3				// 2->3
		ADD  EBX,dest
		MOV  [EBX],EAX
		SUB  EAX,EDI
		IMUL DWORD PTR d		// WORD->DWORD
		IDIV ECX
		ADD  EAX,y
		MOV  [EBX+4],EAX		// 2->4
		INC  ESI
LP3_8:
		MOV ECX,oc
		OR  ECX,ECX
		JNE SHORT LP3_9
		MOV EBX,ESI
		SHL EBX,3				// 2->3
		ADD EBX,dest
		MOV [EBX],EDI
		MOV EAX,y
		MOV [EBX+4],EAX			// 2->4
		INC ESI
LP3_9:
		MOV lastout,ECX
		ADD DWORD PTR radr,8	// WORD->DWORD 4->8
		DEC DWORD PTR i			// WORD->DWORD
		JE  SHORT LP3_A
		JMP LP3_2
LP3_A:
		MOV EAX,ESI
POL_END:
		POP ESI
		POP EDI
	}
}
#pragma warning(default : 4035)


/* 一応後始末する */
#undef OUT_LEFT
#undef OUT_RIGHT
#undef OUT_X
#undef OUT_TOP
#undef OUT_BOTTOM
#undef OUT_Y
#undef GETOUT_X
#undef GETOUT_Y

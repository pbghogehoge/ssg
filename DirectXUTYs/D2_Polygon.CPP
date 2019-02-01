/*                                                                           */
/*   D2_Polygon.cpp   ８ビット時のポリゴン描画                               */
/*                                                                           */
/*                                                                           */

#include "D2_Polygon.h"
#include "DD_CLIP2D.h"


static LINE_WORK TZOID_A,TZOID_B;

static FINT DrawTrapezoid(int y,int dy);		// 台形の描画



// ポリゴンの描画 //
FVOID _2DPolygon(POINT *p,int n)
{
	int		i,j,dy;
	int		nLeft,nRight;
	int		TopY,BottomY;
	POINT	temp[100];

	switch(i=Grp_PClip(temp,p,n)){
		case(0):			// 完全に外側
		return;

		case(-1):			// 完全に内側
		break;

		default:			// クリッピングがかかった
			n = i;
			p = temp;
		break;
	}

	if(n<=2) return;


	// 頂点サーチ //
	TopY = BottomY = p[n-1]. y;
	nLeft = nRight = n-1;
	for(i=n-2; i>=0; i--){
		if(p[i].y == TopY){
			nLeft = i;
		}
		else if(p[i].y > TopY){
			// 一番下にある点のＹ座標を更新
			if(BottomY < p[i].y) BottomY = p[i].y;
		}
		else{
			// 一番上にある点のＹ座標を更新
			nRight = i;
			nLeft  = i;
			TopY   = p[i].y;
		}
	}

	if(nRight==0 && nLeft==n-1){
		nLeft  = 0;
		nRight = n-1;
	}

	//if(nLeft==nRight) n2 = n-1;
	//else              n2 = n-2;

	i = (nLeft-1 < 0) ? (n-1) : nLeft-1;
	SetLineWork(&TZOID_A, p[nLeft ].x, p[i].x, p[i].y-p[nLeft ].y);
	j = (nRight+1 > n-1) ? 0 : nRight+1;
	SetLineWork(&TZOID_B, p[nRight].x, p[j].x, p[j].y-p[nRight].y);

	// 描画ループ //
	while(TopY<BottomY/*n2>0*/){
		//i = (nLeft-1 < 0) ? (n-1) : nLeft-1;
		//j = (nRight+1 > n-1) ? 0 : nRight+1;

		if(p[i].y < p[j].y)	dy = p[i].y-TopY;
		else				dy = p[j].y-TopY;

		//Key_Read();
		//if(Key_Data&KEY_TAMA) return;

		TopY = DrawTrapezoid(TopY,dy);


		if(p[i].y <= TopY){
			// 左側(TZOID_A)を更新する必要がある場合 //
			nLeft = i;//(nLeft-1 < 0) ? (n-1) : nLeft-1;
			i = (nLeft-1 < 0) ? (n-1) : nLeft-1;
			SetLineWork(&TZOID_A, p[nLeft ].x, p[i].x, p[i].y-p[nLeft ].y);
			//n2--;
		}
		if(p[j].y <= TopY){
			// 右側(TZOID_B)を更新する必要がある場合 //
			nRight = j;//(nRight+1 > n-1) ? 0 : nRight+1;
			j = (nRight+1 > n-1) ? 0 : nRight+1;
			SetLineWork(&TZOID_B, p[nRight].x, p[j].x, p[j].y-p[nRight].y);
			//n2--;
		}
	}
}

extern BYTE				*Grp_Vram;				// これは公開しない
extern long				Grp_Pitch;				// これも公開しない
extern DWORD			Grp_AddrY[480];			// Ｙ座標テーブル
extern DWORD			Col2D;


// 横ラインを引く //
static PVOID GrpHLineX(int x1,int x2,int y)
{
	int temp;
	if(x1>x2) temp = x1,x1 = x2,x2 = temp;
	if(x1<0) x1 = 0;
	if(x2>639) x2 = 639;
	//if(y<0 || y>479) return;
	//if(y<0) y = 0;
	//if(y>479) y = 479;


	_asm{
		PUSH EDI			// EDI 保存

		MOV EDI,Grp_Vram
		JZ  HL_END			// NULL チェック(高速化のために外す？)

		LEA EBX,Grp_AddrY
		MOV EAX,y			// EAX = y
		SHL EAX,2			// EAX = y*4 (DWORD)
		ADD EBX,EAX			// EBX = Grp_AddrY+y(DD) <Pointer>
		MOV EAX,[EBX]		// EAX = Grp_AddrY[y]

		ADD EDI,EAX			// EDI := Vram + Grp_AddrY[y]
		MOV EBX,x1			// EBX := x1
		MOV ECX,x2			// ECX := x2
		SUB ECX,EBX			// ECX := x2 - x1
		ADD EDI,EBX			// EDI := Vram + Grp_AddrY[y] + x1(EBX)
		INC ECX				// ECX := x2 - x1 + 1 (Length)
		MOV EBX,ECX			// EBX := x2 - x1 + 1
		AND EBX,3			// EBX := (x2-x1 +1)%4
		SHR ECX,2			// ECX := (x2-x1 +1)/4

		MOV EAX,Col2D

		REP STOSD			// DWORD 描画 (４の倍数で描ける範囲で)
		MOV ECX,EBX			// ECX := (x2-x1 +1)%4
		REP STOSB			// BYTE 描画 (残りを描き込む)

HL_END:
		POP EDI				// EDI 復元
	}
}

// 台形の描画 //
FINT DrawTrapezoid(int y,int dy)
{
	int		left_x = TZOID_A.x;
	int		det_x  = TZOID_B.x;// - left_x;

	do{
		GrpHLineX(left_x,det_x,y);

		_asm{
			//MOV		EAX,TZOID_B.dlx
			//ADD		TZOID_B.s,EAX
			MOV		EAX,TZOID_B.s
			ADD		EAX,TZOID_B.dlx
			MOV		TZOID_B.s,EAX

			MOV		EAX,TZOID_B.x
			ADC		EAX,TZOID_B.d
			MOV		TZOID_B.x,EAX
			MOV		det_x,EAX

			//MOV		EAX,TZOID_A.dlx
			//ADD		TZOID_A.s,EAX
			MOV		EAX,TZOID_A.s
			ADD		EAX,TZOID_A.dlx
			MOV		TZOID_A.s,EAX

			MOV		EAX,left_x
			ADC		EAX,TZOID_A.d
			MOV		left_x,EAX
		}

		//det_x -= left_x;
		y++;
	}
	while((--dy)>=0);

	TZOID_A.x = left_x;

	return y;
}

/*                                                                           */
/*   UT_MATH.c   整数限定の数学関数群                                        */
/*                                                                           */
/*                                                                           */

#include "UT_MATH.h"
#pragma message(PBGWIN_UT_MATH_H)


#define RANDH	0x015a
#define RANDL	0x4e35

static DWORD random_seed; // 乱数のたね //
//DWORD random_ref;


////ｓｉｎテーブル(ｃｏｓを含む)////
extern const signed int SIN256[256+64] = {
	0,6,12,18,25,31,37,43,49,56,62,68,74,80,86,92,97,103,109,115,120,126,131,136,
	142,147,152,157,162,167,171,176,181,185,189,193,197,201,205,209,212,216,219,
	222,225,228,231,234,236,238,241,243,244,246,248,249,251,252,253,254,254,255,
	255,255,256,255,255,255,254,254,253,252,251,249,248,246,244,243,241,238,236,
	234,231,228,225,222,219,216,212,209,205,201,197,193,189,185,181,176,171,167,
	162,157,152,147,142,136,131,126,120,115,109,103,97,92,86,80,74,68,62,56,49,43,
	37,31,25,18,12,6,0,-6,-12,-18,-25,-31,-37,-43,-49,-56,-62,-68,-74,-80,-86,-92,
	-97,-103,-109,-115,-120,-126,-131,-136,-142,-147,-152,-157,-162,-167,-171,
	-176,-181,-185,-189,-193,-197,-201,-205,-209,-212,-216,-219,-222,-225,-228,
	-231,-234,-236,-238,-241,-243,-244,-246,-248,-249,-251,-252,-253,-254,-254,
	-255,-255,-255,-256,-255,-255,-255,-254,-254,-253,-252,-251,-249,-248,-246,
	-244,-243,-241,-238,-236,-234,-231,-228,-225,-222,-219,-216,-212,-209,-205,
	-201,-197,-193,-189,-185,-181,-176,-171,-167,-162,-157,-152,-147,-142,-136,
	-131,-126,-120,-115,-109,-103,-97,-92,-86,-80,-74,-68,-62,-56,-49,-43,-37,-31,
	-25,-18,-12,-6,0,6	,12,18,25,31,37,43,49,56,62,68,74,80,86,92,97,103,109,115,
	120,126,131,136,142,147,152,157,162,167,171,176,181,185,189,193,197,201,205,
	209,212,216,219,222,225,228,231,234,236,238,241,243,244,246,248,249,251,252,
	253,254,254,255,255,255
};

////ｃｏｓテーブル、というのかな？////
extern const signed int *COS256 = &SIN256[64];


////ａｔａｎテーブル////
static const char ATAN256[]={
	0, 0, 0, 0, 1, 1, 1, 1,  1, 1, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 4,  4, 4, 4, 4, 4, 5, 5, 5,
	5, 5, 5, 6, 6, 6, 6, 6,  6, 6, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 9,  9, 9, 9, 9, 9,10,10,10,
	10,10,10,10,11,11,11,11, 11,11,11,12,12,12,12,12,
	12,12,13,13,13,13,13,13, 13,14,14,14,14,14,14,14,
	15,15,15,15,15,15,15,16, 16,16,16,16,16,16,17,17,
	17,17,17,17,17,17,18,18, 18,18,18,18,18,19,19,19,
	19,19,19,19,19,20,20,20, 20,20,20,20,20,21,21,21,
	21,21,21,21,21,21,22,22, 22,22,22,22,22,22,23,23,
	23,23,23,23,23,23,23,24, 24,24,24,24,24,24,24,24,
	25,25,25,25,25,25,25,25, 25,25,26,26,26,26,26,26,
	26,26,26,27,27,27,27,27, 27,27,27,27,27,28,28,28,
	28,28,28,28,28,28,28,28, 29,29,29,29,29,29,29,29,
	29,29,29,30,30,30,30,30, 30,30,30,30,30,30,31,31,
	31,31,31,31,31,31,31,31, 31,31,32,32,32,32,32,32
};


long __fastcall sinl(BYTE deg,int length)
{
	//return ((long)sinm(deg)*length)/256;
	return ((long)sinm(deg)*length)>>8;
}

long __fastcall cosl(BYTE deg,int length)
{
	//return ((long)cosm(deg)*length)/256;
	return ((long)cosm(deg)*length)>>8;
}

// length*256/(SIN(deg)>0 ? SIN(deg) : 256) //
long __fastcall sinDiv(BYTE deg, int length)
{
	register int sind = sinm(deg);
	return (length<<8) / (sind>0 ? sind : 256);
}

// length*256/(COS(deg)>0 ? COS(deg) : 256) //
long __fastcall cosDiv(BYTE deg, int length)
{
	register int cosd = cosm(deg);
	return (length<<8) / (cosd>0 ? cosd : 256);
}

BYTE pascal atan8(int x,int y)
{
	_asm{
			PUSH	ESI

			MOV		ECX,y			; ECX := y
			MOV		ESI,x			; ESI := x
			MOV		EAX,ECX
			OR		EAX,ESI
			JZ		FINISH

			MOV		EAX,ECX
			CDQ
			XOR		EAX,EDX
			SUB		EAX,EDX
			MOV		EBX,EAX			; EBX := ABS(y)

			MOV		EAX,ESI
			CDQ
			XOR		EAX,EDX
			SUB		EAX,EDX
			MOV		EDX,EAX			; EDX := ABS(x)

			CMP		EDX,EBX
			JE		JP_E
			JL		JP_L

			XCHG	EBX,EDX			; EBX := ABS(x) , EDX := ABS(y)
			MOV		EAX,EDX
			SHR		EDX,24
			SHL		EAX,8
			DIV		EBX
			MOV		EBX,offset ATAN256
			XLATB
			JMP		LAST_C0
			EVEN

	JP_E:
			MOV		AL,32
			JMP		LAST_C0
			EVEN

	JP_L:
			MOV		EAX,EDX
			SHR		EDX,24
			SHL		EAX,8
			DIV		EBX
			MOV		EBX,offset ATAN256
			XLATB
			NEG		AL
			ADD		AL,64

			// EBP = x , ECX = y
	LAST_C0:
			XOR		AH,AH
			OR		ESI,ESI
			JGE		LAST_C1
			NEG		AX
			ADD		AX,128

	LAST_C1:
			OR		ECX,ECX
			JGE		LAST_C2
			NEG		AL

	LAST_C2:
			XOR		AH,AH			; RETURN が BYTE だから必要ないのだが...

	FINISH:
			POP		ESI
	}
}

void __fastcall rnd_seed_set(DWORD val)
{
	random_seed = val;
}

int isqrt(int s)
{
	_asm{
			MOV		ECX,s
			MOV		EBX,ECX
			MOV		EAX,1
			JMP		SHORT CHK
		LP1:
			SUB		ECX,EAX
			ADD		EAX,2
		CHK:
			OR		ECX,ECX
			JGE		LP1
			SAR		EAX,1
			MOV		ECX,EAX
			IMUL	ECX
			SUB		EAX,ECX
			INC		EAX
			CMP		EAX,EBX
			JBE		FIN
			DEC		ECX
		FIN:
			MOV		EAX,ECX
	}
}

WORD __fastcall rnd(void)
{
	_asm{
		MOV	AX,RANDL
		MUL	word ptr random_seed+2
		MOV	CX,AX
		MOV	AX,RANDH
		MUL	word ptr random_seed
		ADD	CX,AX
		MOV	AX,RANDL
		MUL	word ptr random_seed
		ADD	AX,1
		ADC	DX,CX
		MOV	word ptr random_seed,AX
		MOV	AX,DX
		MOV	word ptr random_seed+2,AX
		AND	AH,7fh

		//MOV	result,AX
	}
}

#include <Windows.h>
#include <io.h>
#include "LZ_UTY.h"

bool sub_421060(BIT_DEVICE *a1, BIT_DEVICE *a2, int a3);
bool sub_421160(BIT_DEVICE *a1);
void sub_421290(BIT_DEVICE *a1, int a2);
void sub_421330(BIT_DEVICE *a1, int a2);
void sub_421390(int a1);
void sub_4213C0();
int sub_4213F0(int a1, DWORD *a2);
void sub_4214D0(int a1);
void sub_421540(int a1, int a2);
void sub_4215A0(int a1, int a2);
int sub_421630(int a1);
int sub_423080(BIT_DEVICE *a1);

PBG_FILEHEAD FileHead;
PBG_FILEINFO *FileInfo;
uint32_t dword_4F0F40;
char aScreenDBmp[0x100] = "Screen%d.BMP";
uint8_t byte_4D6F38[0x2000];
struct_4D8F38 unk_4D8F38[0x2000];

BIT_DEVICE *FilStartR(const char *a1, int a2) {
	if (a2)
		return NULL;
	BIT_DEVICE *v3 = BitFilCreate(a1, BD_FILE_READ);
	if (sub_421160(v3))
		return v3;
	BitDevRelease(v3);
	return NULL;
}

BIT_DEVICE *FilStartW(const char *a1, int a2, size_t a3) {
	BIT_DEVICE *v4;
	if (a2 != 1)
		return 0;
	v4 = BitFilCreate(a1, 1);
	FileInfo = (PBG_FILEINFO *)LocalAlloc(0x40u, 12 * a3);
	return v4;
}

void FilEnd(BIT_DEVICE *a1) {
	BitDevRelease(a1);
	LocalFree(FileInfo);
}

BYTE *MemExpand(BIT_DEVICE *a1, int a2) {
	BYTE *result; // eax
	BYTE *v3; // edi
	BIT_DEVICE *v4; // esi

	result = (BYTE *)LocalAlloc(0x40u, FileInfo[a2].m0);
	v3 = result;
	if (result)
	{
		v4 = BitMemCreate(result, 5, FileInfo[a2].m0);
		if (!sub_421060(a1, v4, a2))
		{
			LocalFree(v3);
			v3 = 0;
		}
		BitDevRelease(v4);
		result = v3;
	}
	return result;
}

void Compress(BIT_DEVICE *a1, BIT_DEVICE *a2, int a3) {
	BIT_DEVICE *v3; // edi
	int v4; // ebp
	signed int v5; // esi
	int v6; // eax
	signed int v7; // ebx
	signed int v8; // esi
	int v9; // eax
	int v10; // esi
	int v11; // eax
	int v12; // [esp+10h] [ebp-Ch]
	DWORD v13; // [esp+14h] [ebp-8h]
	int v14; // [esp+18h] [ebp-4h]
	int a2a; // [esp+24h] [ebp+8h]

	v3 = a2;
	a2->rack = 0;
	a1->rack = 0;
	a2->mask = 0x80;
	a1->mask = 0x80;
	a2->m4 = 0;
	a1->m4 = 0;
	v12 = 12 * a3;
	FileInfo[a3].m1 = ftell(a2->p.file);
	sub_4213C0();
	v4 = 1;
	v5 = 0;
	do
	{
		v6 = sub_423080(a1);
		if (v6 == -1)
			break;
		byte_4D6F38[++v5] = v6;
	} while (v5 < 18);
	a2a = v5;
	sub_421390(1);
	v7 = 0;
	v13 = 0;
	if (v5 > 0)
	{
		while (1)
		{
			if (v7 > a2a)
				v7 = a2a;
			if (v7 <= 2)
				break;
			PutBit(v3, 0);
			PutBits(v3, v13, 13);
			PutBits(v3, v7 - 3, 4);
			v8 = v7;
			if (v7 > 0)
				goto LABEL_10;
		LABEL_17:
			if ((signed int)a2a <= 0)
				goto LABEL_18;
		}
		v8 = 1;
		PutBit(v3, 1u);
		PutBits(v3, byte_4D6F38[v4], 8);
	LABEL_10:
		v14 = v8;
		do
		{
			sub_4214D0((v4 + 18) & 0x1FFF);
			v9 = sub_423080(a1);
			if (v9 == -1)
				--a2a;
			else
				byte_4D6F38[(v4 + 18) & 0x1FFF] = v9;
			v4 = (v4 + 1) & 0x1FFF;
			if (a2a)
				v7 = sub_4213F0(v4, &v13);
			--v14;
		} while (v14);
		goto LABEL_17;
	}
LABEL_18:
	PutBit(v3, 0);
	PutBits(v3, 0, 13);
	if (v3->mask != 0x80)
		PutChar(v3->rack, v3);
	if (a1->m5 == BD_MEMORY_READ)
	{
		v10 = 12 * a3;
		FileInfo[a3].m0 = a1->m6;
	}
	else
	{
		v11 = _fileno(a1->p.file);
		v10 = 12 * a3;
		FileInfo[a3].m0 = _filelength(v11);
	}
	FileInfo[a3].m2 = v3->m4;
	sub_421330(v3, a3);
	FileHead.sum += FileInfo[a3].m0 + FileInfo[a3].m1 + FileInfo[a3].m2;
}

bool sub_421060(BIT_DEVICE *a1, BIT_DEVICE *a2, int a3) {
	BIT_DEVICE *v3; // ebx
	int v4; // edi
	int v5; // esi
	uint8_t v6; // eax
	char v7; // ST20_1
	uint16_t v8; // eax
	uint8_t v9; // eax
	int v10; // ebp
	int v11; // ebx
	__int16 v13; // [esp+10h] [ebp-4h]

	v3 = a1;
	sub_421290(a1, a3);
	v4 = 0;
	a2->rack = 0;
	a1->rack = 0;
	a2->mask = 0x80;
	a1->mask = 0x80;
	a2->m4 = 0;
	a1->m4 = 0;
	sub_4213C0();
	v5 = 1;
	while (1)
	{
		while (GetBit(v3))
		{
			v6 = GetBits(v3, 8);
			v7 = v6;
			PutChar(v6, a2);
			byte_4D6F38[v5] = v7;
			v5 = ((WORD)v5 + 1) & 0x1FFF;
		}
		v8 = GetBits(v3, 13);
		v13 = v8;
		if (!v8)
			break;
		v9 = GetBits(v3, 4);
		v10 = v9 + 2;
		if (v9 + 2 >= 0)
		{
			do
			{
				v11 = (unsigned __int8)byte_4D6F38[((WORD)v4 + v13) & 0x1FFF];
				PutChar(v11, a2);
				byte_4D6F38[v5] = v11;
				v5 = ((WORD)v5 + 1) & 0x1FFF;
				++v4;
			} while (v4 <= v10);
			v3 = a1;
			v4 = 0;
		}
	}
	return FileInfo[a3].m2 == v3->m4;
}

bool sub_421160(BIT_DEVICE *a1) {
	BIT_DEVICE *v1; // esi
	PBG_FILEINFO *result; // eax
	DWORD v3; // eax
	int *v4; // eax
	DWORD v5; // ecx
	size_t v6; // edx
	int v7; // esi
	uint32_t *v8; // ecx
	int v9; // edi
	long v10;

	v1 = a1;
	if (!a1)
		return 0;
	v3 = a1->m5;
	if (v3)
	{
		if (v3 == BD_MEMORY_READ)
		{
			v4 = (int *)a1->p.m0;
			FileHead.name = *v4;
			FileHead.sum = v4[1];
			FileHead.n = v4[2];
		}
	}
	else
	{
		v10 = ftell(a1->p.file);
		fseek(v1->p.file, 0, 0);
		fread(&FileHead.name, 0xCu, 1u, v1->p.file);
	}
	if (FileHead.name != PBG_HEADNAME)
		return 0;
	result = (PBG_FILEINFO *)LocalAlloc(0x40u, 12 * FileHead.n);
	FileInfo = result;
	if (result)
	{
		v5 = v1->m5;
		if (v5)
		{
			if (v5 != BD_MEMORY_READ)
			{
			LABEL_15:
				v6 = FileHead.n;
				v7 = 0;
				if (FileHead.n)
				{
					v8 = &result[0].m1;
					do
					{
						v9 = *v8 + v8[1] + *(v8 - 1);
						v8 += 3;
						v7 += v9;
						--v6;
					} while (v6);
				}
				if (v7 == FileHead.sum)
					return 1;
				LocalFree(result);
				return 0;
			}
			memcpy((void *)result, (const void *)(v1->m1 + 12), 12 * FileHead.n);
		}
		else
		{
			fread((void *)result, 0xCu, FileHead.n, v1->p.file);
			fseek(v1->p.file, v10, 0);
		}
		result = FileInfo;
		goto LABEL_15;
	}
	return result;
}

void sub_421290(BIT_DEVICE *a1, int a2) {
	DWORD v2; // ecx
	v2 = a1->m5;
	if (v2)
	{
		if (v2 == BD_MEMORY_READ)
			a1->p.m0 = a1->m1 + FileInfo[a2].m1;
	}
	else
		fseek(a1->p.file, FileInfo[a2].m1, SEEK_SET);
}

void WriteHead(BIT_DEVICE *a1) {
	int v1; // edi

	if (a1->m5 == BD_FILE_WRITE)
	{
		v1 = ftell(a1->p.file);
		fseek(a1->p.file, 0, 0);
		fwrite(&FileHead.name, 0xC, 1, a1->p.file);
		fseek(a1->p.file, v1, 0);
	}
}

void sub_421330(BIT_DEVICE *a1, int a2)
{
	int v2; // ebx

	if (a1->m5 == BD_FILE_WRITE)
	{
		v2 = ftell(a1->p.file);
		fseek(a1->p.file, 4 * (3 * a2 + 3), 0);
		fwrite(&FileInfo[a2], sizeof(FileInfo[a2]), 1, a1->p.file);
		fseek(a1->p.file, v2, 0);
	}
}

void sub_421390(int a1) {
	dword_4F0F40 = a1;
	unk_4D8F38[a1].m0 = 0x2000;
	unk_4D8F38[a1].m2 = 0;
	unk_4D8F38[a1].m1 = 0;
}

void sub_4213C0() {
	memset(byte_4D6F38, 0, sizeof(byte_4D6F38));
	memset(unk_4D8F38, 0, sizeof(unk_4D8F38));
}

int sub_4213F0(int a1, DWORD *a2) {
	__int16 v2; // cx
	signed int v3; // esi
	signed int result; // eax
	int v5; // ebx
	__int16 v6; // di
	int v7; // edx
	int *v8; // edx
	int v9; // ecx
	signed int v10; // [esp+10h] [ebp-4h]

	v2 = a1;
	v3 = 0;
	if (!a1)
		return 0;
	v5 = dword_4F0F40;
	v10 = 0;
	while (1)
	{
		v6 = v5 - v2;
		do
		{
			v7 = byte_4D6F38[v2 & 0x1FFF] - byte_4D6F38[(v6 + v2) & 0x1FFF];
			if (byte_4D6F38[v2 & 0x1FFF] != byte_4D6F38[(v6 + v2) & 0x1FFF])
				break;
			++v3;
			++v2;
		} while (v3 < 18);
		result = v10;
		if (v3 >= v10)
		{
			result = v3;
			v10 = v3;
			*a2 = v5;
			if (v3 >= 18)
			{
				sub_4215A0(v5, a1);
				return v3;
			}
		}
		v3 = 0;
		v8 = (int *)(v7 < 0 ? 12 * v5 + 5082940 : 12 * v5 + 5082944);
		if (!*v8)
			break;
		v5 = *v8;
		v2 = a1;
	}
	*v8 = a1;
	unk_4D8F38[a1].m0 = v5;
	unk_4D8F38[a1].m2 = 0;
	unk_4D8F38[a1].m1 = 0;
	return result;
}

void sub_4214D0(int a1)
{
	int v1; // eax
	int v2; // edi

	v1 = 3 * a1;
	if (unk_4D8F38[a1].m0)
	{
		if (unk_4D8F38[a1].m2)
		{
			if (unk_4D8F38[a1].m1)
			{
				v2 = sub_421630(a1);
				sub_4214D0(v2);
				sub_4215A0(a1, v2);
			}
			else
			{
				sub_421540(a1, unk_4D8F38[a1].m2);
			}
		}
		else
		{
			sub_421540(a1, unk_4D8F38[a1].m1);
		}
	}
}

void sub_421540(int a1, int a2)
{
	int v2; // eax

	unk_4D8F38[a2].m0 = unk_4D8F38[a1].m0;
	v2 = unk_4D8F38[a1].m0;
	if (unk_4D8F38[v2].m2 == a1)
		unk_4D8F38[v2].m2 = a2;
	else
		unk_4D8F38[v2].m1 = a2;
	unk_4D8F38[a1].m0 = 0;
}

void sub_4215A0(int a1, int a2)
{
	int v2; // eax
	struct_4D8F38 *v3; // esi

	v2 = unk_4D8F38[a1].m0;
	if (unk_4D8F38[v2].m1 == a1)
		unk_4D8F38[v2].m1 = a2;
	else
		unk_4D8F38[v2].m2 = a2;
	v3 = unk_4D8F38 + a2;
	v3->m0 = unk_4D8F38[a1].m0;
	v3->m1 = unk_4D8F38[a1].m1;
	v3->m2 = unk_4D8F38[a1].m2;
	unk_4D8F38[unk_4D8F38[a2].m1].m0 = a2;
	unk_4D8F38[unk_4D8F38[a2].m2].m0 = a2;
	unk_4D8F38[a1].m0 = 0;
}

int sub_421630(int a1)
{
	int result; // eax
	int i; // ecx

	result = unk_4D8F38[a1].m1;
	for (i = unk_4D8F38[result].m2; i; i = unk_4D8F38[i].m2)
		result = i;
	return result;
}

BIT_DEVICE *BitFilCreate(const char *a1, int a2)
{
	BIT_DEVICE *v2; // esi
	char v3; // cl
	FILE *v4; // eax
	const char *v6; // [esp+Ch] [ebp-4h]

	v2 = (BIT_DEVICE *)malloc(0x20u);
	if (!v2)
		return NULL;

	switch (a2)
	{
	case BD_FILE_READ:
		v6 = "rb";
		break;
	case BD_FILE_WRITE:
		v6 = "wb";
		break;
	case 2:
		v6 = "rb";
		break;
	case 3:
		v6 = "wb";
		break;
	default:
		free(v2);
		return 0;
	}
	v2->p.file = fopen(a1, v6);
	if (!v2->p.file) {
		free(v2);
		return 0;
	}
	v2->m1 = 0;
	v2->mask = 0x80;
	v2->rack = 0;
	v2->m4 = 0;
	v2->m5 = a2;
	v2->m6 = 0;
	v2->m7 = 0;
	return v2;
}

BIT_DEVICE *BitMemCreate(BYTE *a1, int a2, size_t a3) {
	BIT_DEVICE *result; // eax
	result = (BIT_DEVICE *)malloc(0x20u);
	if (result)
	{
		if (a2 >= 4 && a2 <= 7)
		{
			result->p.m0 = a1;
			result->m1 = a1;
			result->mask = 0x80;
			result->rack = 0;
			result->m4 = 0;
			result->m5 = a2;
			result->m6 = a3;
			result->m7 = 0;
			return result;
		}
		free(result);
	}
	return 0;
}

void BitDevRelease(BIT_DEVICE *a1) {
	if (a1) {
		switch (a1->m5)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			fclose(a1->p.file);
		case 4:
		case 5:
		case 6:
		case 7:
			free(a1);
		}
	}
}

void PutBit(BIT_DEVICE *a1, BYTE a2) {
	char v2; // al
	unsigned __int8 *v3; // edx
	int v4; // eax
	int v5; // eax
	int v6; // ecx

	if (a2)
		a1->rack |= a1->mask;
	v2 = a1->mask >> 1;
	a1->mask = v2;
	if (!v2)
	{
		if (a1->m5 == BD_FILE_WRITE)
		{
			putc(a1->rack, a1->p.file);
		}
		else
		{
			if (a1->m5 != 5)
				return;
			*(a1->p.m0)++ = (a1->rack);
		}
		v5 = a1->rack;
		v6 = a1->m4;
		a1->rack = 0;
		a1->m4 = v5 + v6;
		a1->mask = 0x80;
	}
}

void PutBits(BIT_DEVICE *a1, DWORD a2, int a3) {
	unsigned int v3; // edi
	char v4; // cl
	DWORD v5; // ecx
	DWORD v6; // edx
	char v7; // al
	unsigned __int8 *v8; // edx
	int v9; // eax
	DWORD v10; // ecx
	DWORD v11; // eax

	v3 = 1 << (a3 - 1);
	if (a1->m5 == BD_FILE_WRITE)
	{
		for (; v3; v3 >>= 1)
		{
			if (v3 & a2)
				a1->rack |= a1->mask;
			v7 = a1->mask >> 1;
			a1->mask = v7;
			if (!v7)
			{
				putc(a1->rack, a1->p.file);
				v10 = a1->rack;
				v11 = a1->m4;
				a1->rack = 0;
				a1->m4 = v10 + v11;
				a1->mask = 0x80;
			}
		}
	}
	else if (a1->m5 == 5 && v3)
	{
		do
		{
			if (v3 & a2)
				a1->rack |= a1->mask;
			v4 = a1->mask >> 1;
			a1->mask = v4;
			if (!v4)
			{
				*(a1->p.m0) = a1->rack;
				v5 = a1->m4;
				++(a1->p.m0);
				v6 = a1->rack;
				a1->rack = 0;
				a1->m4 = v6 + v5;
				a1->mask = -128;
			}
			v3 >>= 1;
		} while (v3);
	}
}

void PutChar(uint32_t a1, BIT_DEVICE *a2) {
	switch (a2->m5)
	{
	case BD_FILE_WRITE:
	case 3:
		fputc(a1, a2->p.file);
		break;
	case 5:
	case 7:
		*(a2->p.m0) = a1;
		(a2->p.m0)++;
		break;
	default:
		return;
	}
}

BYTE GetBit(BIT_DEVICE *a1) {
	int v1; // eax
	unsigned int v3; // ecx
	int v4; // edx
	DWORD *v5; // ecx
	int v6; // eax
	int v7; // eax
	char v8; // cl

	v1 = a1->m5;
	if (v1)
	{
		if (v1 != BD_MEMORY_READ)
			return 0;
		if (a1->mask == 0x80)
		{
			v3 = a1->m7;
			if (v3 >= a1->m6)
			{
				return 0;
			}
			v4 = *(a1->p.m0)++;
			a1->rack= v4;
			a1->m4 += v4;
			a1->m7 = v3 + 1;
		}
	}
	else if (a1->mask == 0x80)
	{
		v6 = getc(a1->p.file);
		a1->rack = v6;
		if (v6 == -1)
		{
			return 0;
		}
		a1->m4 += v6;
	}
	v7 = a1->mask & a1->rack;
	v8 = a1->mask >> 1;
	a1->mask = v8;
	if (!v8)
		a1->mask = 0x80;
	return v7 != 0;
}

/*
WORD GetBits(BIT_DEVICE *a1, int a2) {
	DWORD v2; // eax
	WORD v3; // bx
	unsigned int v4; // edi
	unsigned __int8 v6; // al
	DWORD v7; // ebp
	DWORD v8; // edx
	char v9; // al
	DWORD *v10; // ecx
	int v11; // eax
	unsigned __int8 v12; // al
	char v13; // al

	v2 = a1->m5;
	v3 = 0;
	v4 = 1 << (a2 - 1);
	if (v2)
	{
		if (v2 != BD_MEMORY_READ)
			return 0;
		while (v4)
		{
			v6 = a1->mask;
			if (v6 == 0x80)
			{
				v7 = a1->m7;
				if (v7 >= a1->m6)
				{
					return 0;
				}
				v8 = *(a1->p.m0)++;
				a1->rack = v8;
				a1->m4 += v8;
				a1->m7 = v7 + 1;
			}
			if (v6 & (uint8_t)(a1->rack))
				v3 |= v4;
			v4 >>= 1;
			v9 = v6 >> 1;
			a1->mask = v9;
			if (!v9)
				a1->mask = 0x80;
		}
		return v3;
	}
	if (!v4)
		return v3;
	for (;;)
	{
		v12 = a1->mask;
		if ((uint8_t)(a1->rack) & v12)
			v3 |= v4;
		v4 >>= 1;
		v13 = v12 >> 1;
		a1->mask = v13;
		if (!v13) {
			a1->mask = 0x80;
			v11 = getc(a1->p.file);
			a1->rack = v11;
			if (v11 != -1)
				a1->m4 += v11;
			else
				return 0;
		}
		if (!v4)
			return v3;
	}
}
*/

WORD GetBits(BIT_DEVICE *a1, int a2) {
	WORD ret = 0;
	while (a2--) {
		ret = ret * 2 + GetBit(a1);
	}
	return ret;
}

int sub_423080(BIT_DEVICE *a1) {
	int result; // eax
	DWORD v2; // esi

	switch (a1->m5)
	{
	case BD_FILE_READ:
	case 2:
		return getc(a1->p.file);
	case BD_MEMORY_READ:
	case 6:
		v2 = a1->m7;
		if (v2 < a1->m6)
		{
			result = *(a1->p.m0)++;
			a1->m7 = v2 + 1;
		}
		else
			result = -1;
		return result;
	default:
		return 0;
	}
}

void GrpSetCaptureFilename(const char *a1) {
	sprintf(aScreenDBmp, "%.4s%%04d.BMP", a1);
	CaptureFlag = 1;
}
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BD_FILE_READ 0
#define BD_FILE_WRITE 1
#define BD_MEMORY_READ 4

#define PBG_HEADNAME 0x1A474250

typedef struct {
	union {
		FILE *file;
		BYTE *m0;
	} p;
	BYTE *m1;
	unsigned char mask;
	uint32_t rack;
	uint32_t m4;
	uint32_t m5;
	size_t m6;
	uint32_t m7;
} BIT_DEVICE;

typedef struct {
	uint32_t name;
	uint32_t sum;
	size_t n;
} PBG_FILEHEAD;

typedef struct {
	uint32_t m0;
	uint32_t m1;
	uint32_t m2;
} PBG_FILEINFO;

typedef PBG_FILEINFO struct_4D8F38;

extern PBG_FILEHEAD FileHead;
extern PBG_FILEINFO *FileInfo;
extern char aScreenDBmp[0x100];
extern BOOL CaptureFlag;

BIT_DEVICE *FilStartR(const char *a1, int a2);
BIT_DEVICE *FilStartW(const char *a1, int a2, size_t a3);
void FilEnd(BIT_DEVICE *a1);
#ifdef __cplusplus
extern "C" BYTE *MemExpand(BIT_DEVICE *a1, int a2);
#else
BYTE *MemExpand(BIT_DEVICE *a1, int a2);
#endif
void Compress(BIT_DEVICE *a1, BIT_DEVICE *a2, int a3);
void WriteHead(BIT_DEVICE *a1);
BIT_DEVICE *BitFilCreate(const char *a1, int a2);
BIT_DEVICE *BitMemCreate(BYTE *a1, int a2, size_t a3);
void BitDevRelease(BIT_DEVICE *a1);
void PutBit(BIT_DEVICE *a1, BYTE a2);
void PutBits(BIT_DEVICE *a1, DWORD a2, int a3);
void PutChar(uint32_t a1, BIT_DEVICE *a2);
BYTE GetBit(BIT_DEVICE *a1);
WORD GetBits(BIT_DEVICE *a1, int a2);
void GrpSetCaptureFilename(const char *a1);

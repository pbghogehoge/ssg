/*                                                                           */
/*   Score.cpp   スコア入出力関数                                            */
/*                                                                           */
/*                                                                           */

#include "Score.h"

//#define PBG_MASK_VALUE		((DWORDLONG)0x342d3a4de92c68ba)
//#define PBG_MASK_VALUE		((DWORDLONG)0xe92cd3a4d8ba6342)
#define PBG_MASK_VALUE		((DWORDLONG)0xb97eb2c6542d3a41)



const char *ScoreFileName = "秋霜SC.DAT";				// スコアデータ格納ファイル名
NR_SCORE_STRING		ScoreString[NR_RANK_MAX];	// スコアデータ文字列格納先

BYTE ExtraFlags = 0;

static NR_SCORE_DATA *ScoreData = NULL;		// スコアデータ格納先(名前空間で保護)

static BOOL LoadScoreData(void);			// スコアデータを読み込む
static BOOL ReleaseScoreData(void);			// スコアデータを解放する
static NR_NAME_DATA *SetNDataPtr(BYTE Dif);	// 難易度でポインタを振り分ける
static BOOL SetDefaultScoreData(void);		// スコアデータ初期値をセット

static BOOL _LoadSC(NR_NAME_DATA *NData, BIT_DEVICE *bd);
static BOOL _SaveSC(NR_NAME_DATA *NData, BIT_DEVICE *bd);
static BOOL _LoadExBit(BIT_DEVICE *bd, BYTE *flags);
static BOOL _SaveExBit(BIT_DEVICE *bd, BYTE flags);

static BYTE      _xGet8( BIT_DEVICE *bd, DWORDLONG *ExMask);
static DWORD     _xGet32(BIT_DEVICE *bd, DWORDLONG *ExMask);
static DWORDLONG _xGet64(BIT_DEVICE *bd, DWORDLONG *ExMask);

static void _xPut8( BIT_DEVICE *bd, BYTE  data,    DWORDLONG *ExMask);
static void _xPut32(BIT_DEVICE *bd, DWORD data,    DWORDLONG *ExMask);
static void _xPut64(BIT_DEVICE *bd, DWORDLONG data,DWORDLONG *ExMask);




// 現在のスコア列を取得する(名前挿入アリ) //
// NData == NULL の場合、挿入しません     //
int SetScoreString(NR_NAME_DATA *NData, BYTE Dif)
{
//	static char			*WTable[4] = {"WIDE ","HOMING","LASER","?????"};
	NR_SCORE_STRING		*Res;
	NR_NAME_DATA		*p;
	int					i,num,rank;
	__int64				temp;

	Res = ScoreString;


	// スコアデータを読み込む //
	if(NData != NULL){
		rank = IsHighScore(NData, Dif);
		if(rank==0) return 0;
	}
	else{
		rank = NR_RANK_MAX;
	}

	// ポインタをセットするぞ //
	if(!LoadScoreData()) return 0;
	p = SetNDataPtr(Dif);
	if(p == NULL){
		ReleaseScoreData();
		return 0;
	}

	if(rank!=0 && NData!=NULL){
		// まずは、スコアを下方向に押し出すのだ //
		for(i=NR_RANK_MAX-1; i>=rank; i--){
			p[i] = p[i-1];		// 構造体から構造体への代入
		}

		// 新しいデータを挿入だ //
		p[rank-1] = *NData;
	}

	// データの格納開始 //
	temp = num = 0;
	for(i=0; i<NR_RANK_MAX; i++){
		if(temp < p[i].Score){
			temp  = p[i].Score;
			num += 1;
		}

		Res[i].Rank = num;
		Res[i].x    = (640 + (50  + i*24*20)) << 6;
		Res[i].y    = (100 + i*48) << 6;
		Res[i].bMoveEnable = TRUE;

		strcpy(Res[i].Name  , p[i].Name);

		//strcpy(Res[i].Weapon, WTable[p[i].Weapon % 4]);
		Res[i].Weapon = p[i].Weapon % 4;

		sprintf(Res[i].Score,"%11I64d",p[i].Score);
		sprintf(Res[i].Evade,"%6d" ,p[i].Evade);
		sprintf(Res[i].Stage,"%1d" ,p[i].Stage);
	}

	ReleaseScoreData();

	return rank;
}


// ０：ハイスコアでない  それ以外：順位 //
int IsHighScore(NR_NAME_DATA *NData, BYTE Dif)
{
	NR_NAME_DATA	*temp = NULL;
	int				Rank;

	// ロードできないので失敗！ //
	if(!LoadScoreData()) return 0;

	// 難易度でポインタを振り分ける //
	temp = SetNDataPtr(Dif);
	if(temp == NULL){
		ReleaseScoreData();
		return 0;
	}

	// 該当個所はあるかな？ //
	for(Rank=0; Rank<NR_RANK_MAX ; Rank++){
		// スコアが等しい場合は、後から入ったほうが下の順位に //
		// なるようにするのだ（前作[秋霜玉(仮] と同じね）     //
		if(NData->Score > temp[Rank].Score) break;
	}
	if(Rank == NR_RANK_MAX) Rank = -1;

	// 解放を行う //
	ReleaseScoreData();

	// 順位を返す //
	return Rank+1;
}


// スコアデータを書き出す //
BOOL SaveScoreData(NR_NAME_DATA *NData, BYTE Dif)
{
	int				Rank;
	int				i;
	NR_NAME_DATA	*temp;
	BIT_DEVICE		*bd;

	// スコアデータを読み込む //
	Rank = IsHighScore(NData, Dif);

	// これは、ハイスコアじゃないね //
	if(Rank == 0) return FALSE;

	// ポインタをセットする //
	if(!LoadScoreData()) return FALSE;
	temp = SetNDataPtr(Dif);
	if(temp == NULL){
		ReleaseScoreData();
		return FALSE;
	}

	// まずは、スコアを下方向に押し出す //
	for(i=NR_RANK_MAX-1; i>=Rank; i--){
		temp[i] = temp[i-1];		// 構造体から構造体への代入
	}

	// 新しいデータを挿入だ //
	temp[Rank-1] = *NData;

	// 実際にファイルに出力 //
	bd = BitFilCreate(ScoreFileName, BD_FILE_WRITE);
	if(bd != NULL){
		_SaveSC(ScoreData->Easy,    bd);
		_SaveSC(ScoreData->Normal,  bd);
		_SaveSC(ScoreData->Hard,    bd);
		_SaveSC(ScoreData->Lunatic, bd);
		_SaveSC(ScoreData->Extra,   bd);

		if(bd->mask != 0x80) PutChar(bd->rack, bd);
		BitDevRelease(bd);
	}
	else{
		ReleaseScoreData();
		return FALSE;
	}

	ReleaseScoreData();
	return TRUE;
}


// スコアデータを読み込む //
static BOOL LoadScoreData(void)
{
	BIT_DEVICE		*bd;
	BOOL			bInit = FALSE;

	// すでにロード済みの場合(これは失敗にしない) //
	if(ScoreData) return TRUE;

	ScoreData = (NR_SCORE_DATA *)LocalAlloc(LPTR, sizeof(NR_SCORE_DATA));
	if(ScoreData == NULL) return FALSE;

	// ビット読み込みモードでファイルを開く //
	bd = BitFilCreate(ScoreFileName, BD_FILE_READ);
	if(bd != NULL){
		while(1){
			if(!_LoadSC(ScoreData->Easy,    bd)) break;
			if(!_LoadSC(ScoreData->Normal,  bd)) break;
			if(!_LoadSC(ScoreData->Hard,    bd)) break;
			if(!_LoadSC(ScoreData->Lunatic, bd)) break;
			if(!_LoadSC(ScoreData->Extra,   bd)) break;

			bInit = TRUE;
			break;
		}

		BitDevRelease(bd);
	}

	if(!bInit){
		// ファイルが存在しないか不正な場合、新たに作成する //
		// この時点では、ファイルに対して書き込みは行わない //
		return SetDefaultScoreData();
	}

	return TRUE;
}


// スコアデータを解放する //
static BOOL ReleaseScoreData(void)
{
	// まだロードされていない場合(これは失敗にする) //
	if(!ScoreData) return FALSE;

	// 解放～ //
	LocalFree(ScoreData);
	ScoreData = NULL;

	return TRUE;
}


// 難易度でポインタを振り分ける //
static NR_NAME_DATA *SetNDataPtr(BYTE Dif)
{
	if(NULL == ScoreData) return NULL;

	switch(Dif){
		case(GAME_EASY):	return ScoreData->Easy;
		case(GAME_NORMAL):	return ScoreData->Normal;
		case(GAME_HARD):	return ScoreData->Hard;
		case(GAME_LUNATIC):	return ScoreData->Lunatic;
		case(GAME_EXTRA):	return ScoreData->Extra;
		default:			return NULL;
	}
}

// スコアデータ初期値をセット //
static BOOL SetDefaultScoreData(void)
{
	int				i,j;
	NR_NAME_DATA	*temp;

	if(NULL == ScoreData) return FALSE;

	for(i=0; i<5; i++){
		temp = SetNDataPtr(i);
		if(temp == NULL) return FALSE;

		for(j=0; j<NR_RANK_MAX; j++){
			strcpy(temp[j].Name, "????????");
			temp[j].Score  = (NR_RANK_MAX-j) * 1200000;//1500000;
			temp[j].Evade  = (NR_RANK_MAX-j) * 50;
			temp[j].Stage  = (i<4) ? (NR_RANK_MAX-j) : 1;
			temp[j].Weapon = j % 3;
		}
	}

	return TRUE;
}

static BOOL _LoadSC(NR_NAME_DATA *NData, BIT_DEVICE *bd)
{
	int			i,j;
	DWORDLONG	CheckSum = 0;
	DWORDLONG	Mask     = PBG_MASK_VALUE;
	BYTE		flag     = 0;

	for(i=0; i<NR_RANK_MAX; i++){
		CheckSum = 0;
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// 名前を獲得する //
		for(j=0; j<NR_NAME_LEN; j++){
			NData[i].Name[j] = _xGet8(bd, &Mask);
			CheckSum += NData[i].Name[j];
		}
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// 得点を獲得する //
		NData[i].Score = _xGet64(bd, &Mask);
		CheckSum += NData[i].Score;
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// かすりを獲得する //
		NData[i].Evade = _xGet32(bd, &Mask);
		CheckSum += NData[i].Evade;
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// ステージを獲得する //
		NData[i].Stage = _xGet8(bd, &Mask);
		CheckSum += NData[i].Stage;
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// ウエポンを獲得する //
		NData[i].Weapon = _xGet8(bd, &Mask);
		CheckSum += NData[i].Weapon;
		if(flag != GetBit(bd)) return FALSE;
		else                   flag = 1-flag;

		// チェックサム比較 //
		if(CheckSum != _xGet64(bd, &Mask)) return FALSE;
	}

	return TRUE;
}

static BOOL _SaveSC(NR_NAME_DATA *NData, BIT_DEVICE *bd)
{
	int			i,j;
	DWORDLONG	CheckSum = 0;
	DWORDLONG	Mask     = PBG_MASK_VALUE;
	BYTE		flag     = 0;

	for(i=0; i<NR_RANK_MAX; i++){
		CheckSum = 0;
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// 名前を出力する //
		for(j=0; j<NR_NAME_LEN; j++){
			CheckSum += NData[i].Name[j];
			_xPut8(bd, NData[i].Name[j], &Mask);
		}
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// 得点を出力する //
		CheckSum += NData[i].Score;
		_xPut64(bd, NData[i].Score, &Mask);
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// かすりを出力する //
		CheckSum += NData[i].Evade;
		_xPut32(bd, NData[i].Evade, &Mask);
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// ステージを出力する //
		CheckSum += NData[i].Stage;
		_xPut8(bd, NData[i].Stage, &Mask);
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// ウエポンを出力する //
		CheckSum += NData[i].Weapon;
		_xPut8(bd, NData[i].Weapon, &Mask);
		PutBit(bd, flag); flag = 1-flag;	// ビット挿入

		// チェックサムを出力する //
		_xPut64(bd, CheckSum, &Mask);
	}

	return TRUE;
}


typedef union tagWORD64{
	DWORDLONG		data64;
	struct{
		WORD	d1;
		WORD	d2;
		WORD	d3;
		WORD	d4;
	};
} WORD64;

typedef union tagWORD32{
	DWORD		data32;
	struct{
		WORD	d1;
		WORD	d2;
	};
} WORD32;

static void _xPut8( BIT_DEVICE *bd, BYTE data, DWORDLONG *ExMask)
{
	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	data += (BYTE)(*ExMask);

	PutBits(bd, (DWORD)data, 8);
}

static BYTE _xGet8( BIT_DEVICE *bd, DWORDLONG *ExMask)
{
	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	return (BYTE)GetBits(bd, 8) - (BYTE)(*ExMask);
}

static void _xPut32(BIT_DEVICE *bd, DWORD data, DWORDLONG *ExMask)
{
	WORD32		temp;

	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	temp.data32 = data + (DWORD)(*ExMask);

	PutBits(bd, (DWORD)temp.d1, 16);
	PutBits(bd, (DWORD)temp.d2, 16);
}

static DWORD _xGet32(BIT_DEVICE *bd, DWORDLONG *ExMask)
{
	WORD32		temp;

	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	temp.d1 = (WORD)GetBits(bd, 16);
	temp.d2 = (WORD)GetBits(bd, 16);

	return temp.data32 - (DWORD)(*ExMask);
}

static void _xPut64(BIT_DEVICE *bd, DWORDLONG data, DWORDLONG *ExMask)
{
	WORD64		temp;

	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	temp.data64 = data + (DWORDLONG)(*ExMask);

	PutBits(bd, (DWORD)temp.d1, 16);
	PutBits(bd, (DWORD)temp.d2, 16);
	PutBits(bd, (DWORD)temp.d3, 16);
	PutBits(bd, (DWORD)temp.d4, 16);
}

static DWORDLONG _xGet64(BIT_DEVICE *bd, DWORDLONG *ExMask)
{
	WORD64		temp;

	*ExMask = (((*ExMask)&0x800000000000000)>>60)+((*ExMask)<<1);

	temp.d1 = (WORD)GetBits(bd, 16);
	temp.d2 = (WORD)GetBits(bd, 16);
	temp.d3 = (WORD)GetBits(bd, 16);
	temp.d4 = (WORD)GetBits(bd, 16);

	return temp.data64 - (DWORDLONG)(*ExMask);
}

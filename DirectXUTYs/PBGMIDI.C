/*                                                                           */
/*   PBGMIDI.c   ＭＩＤＩ管理用関数                                          */
/*                                                                           */
/*                                                                           */

#define PBGMIDI_SOURCE_COMPILE
#include "PBGMIDI.H"
#pragma message(PBGWIN_PBGMIDI_H)


// ローカルな関数 //
static void Mid_GMReset(void);
static BOOL Mid_Init(void);

// さらに？ローカルな関数 //
static WORD  ConvWord(WORD data);
static DWORD ConvDWord(DWORD data);
static DWORD GetWaitCount(LPBYTE *data);
static void  CALLBACK CBMid_TimeFunc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
static void  Mid_Parse(MID_TRACK *track);
static void MidFadeIOFunc(void);
static void Mid_ShortMsg(BYTE d1,BYTE d2,BYTE d3);	// ショートメッセージを出力する

// グローバル＆名前空間でローカルな変数 //
MID_DEVICE	Mid_Dev;
static MID_DATA		Mid_Data;
static TIME_DATA	Mid_Time;
WORD				Mid_PlayTable[16][128];			// スペアナ用
WORD				Mid_PlayTable2[16][128];		// レベルメーター用
BYTE				Mid_NoteTable[16][128];			// ノート表示用
BYTE				Mid_NoteWTable[16][128];		// ノート表示用(2)
BYTE				Mid_PanpodTable[16];			// パンポット
BYTE				Mid_ExpressionTable[16];		// エクスプレッション
BYTE				Mid_VolumeTable[16];			// ボリューム

static BYTE			Mid_MulTempo = MID_STDTEMPO;
DWORD				Mid_PlayTime = 0;


BOOL Mid_Start(WORD fnmode,WORD plmode)
{
	MIDIOUTCAPS		caps;
	int				i;
	UINT			mret;

	// 各変数の初期化 //
	Mid_Dev.fnmode  = fnmode;
	Mid_Dev.plmode  = plmode;
	Mid_Dev.mp      = NULL;
	Mid_Dev.nDevice = midiOutGetNumDevs()+1;	// 一つだけ多く準備する
	Mid_Dev.NowID   = -1 + 1;					// -1: MIDI_MAPPER
	Mid_PlayTime    = 0;

	// デバイスが存在しないならリターン //
	if(Mid_Dev.nDevice)
		Mid_Dev.name = (char **)LocalAlloc(LPTR,sizeof(char *)*(Mid_Dev.nDevice));
	else{
		Mid_Dev.name = NULL;
		return FALSE;
	}

	// デバイス名格納用のメモリ確保＆セット //
	for(i=0;i<Mid_Dev.nDevice;i++){
		midiOutGetDevCaps(i-1,&caps,sizeof(MIDIOUTCAPS));
		Mid_Dev.name[i] = (char *)LocalAlloc(LPTR,MAXPNAMELEN);
		strcpy(Mid_Dev.name[i],caps.szPname);
	}

	// 使用できるデバイスを探す(最初がMIDI_MAPPERなのがポイントだ!) //
	for(i=0;i<Mid_Dev.nDevice;i++){
		Mid_Dev.NowID = i;
		mret = midiOutOpen(&Mid_Dev.mp,Mid_Dev.NowID-1,0,0,CALLBACK_NULL);
		if(mret==MMSYSERR_NOERROR) return TRUE;
	}

	// 使用できるデバイスが存在しないとき //
	Mid_End();
	return FALSE;
}

void Mid_End(void)
{
	int i;

	Mid_Stop();
	Mid_Free();

	if(Mid_Dev.mp) midiOutClose(Mid_Dev.mp);

	if(Mid_Dev.name){
		for(i=0;i<Mid_Dev.nDevice;i++) LocalFree(Mid_Dev.name[i]);
		LocalFree(Mid_Dev.name);
		Mid_Dev.mp      = NULL;
		Mid_Dev.name    = NULL;
	}

	Mid_Dev.nDevice = Mid_Dev.NowID   = 0;
}

void Mid_Play(void)
{
	if(Mid_Data.data==NULL)       return;
	if(Mid_Dev.state==MIDST_PLAY) return;

	Mid_Dev.FadeFlag  = 0;
	Mid_Dev.MaxVolume = 127;
	Mid_Dev.NowVolume = 127;
	Mid_Volume(Mid_Dev.NowVolume);

	Mid_Init();

	switch(Mid_Dev.fnmode){
		case(MIDFN_CALLBACK):
			Mid_GMReset();
			timeGetDevCaps(&Mid_Time.caps,sizeof(TIMECAPS));
			timeBeginPeriod(Mid_Time.caps.wPeriodMin);
			Mid_Time.delay  = 10;
			Mid_Time.htimer = timeSetEvent(Mid_Time.delay,Mid_Time.caps.wPeriodMin,
												CBMid_TimeFunc,0,TIME_PERIODIC);
		break;

		case(MIDFN_MIDLOOP):
			Mid_GMReset();
		break;

		default:
		return;
	}

	Mid_Dev.state = MIDST_PLAY;
}

void Mid_Stop(void)
{
	int i;

	if(Mid_Dev.state==MIDST_STOP) return;

	Mid_PlayTime     = 0;
	Mid_Dev.FadeFlag = 0;

	switch(Mid_Dev.fnmode){
		case(MIDFN_CALLBACK):
			timeKillEvent(Mid_Time.htimer);
			timeEndPeriod(Mid_Time.caps.wPeriodMin);
		break;

		case(MIDFN_MIDLOOP):
		break;

		default:
		return;
	}

	for(i=0;i<16;i++){
		Mid_ShortMsg(0xb0+i,0x7b,0x00);		// オール・ノート・オフ
		Mid_ShortMsg(0xb0+i,0x78,0x00);		// オール・サウンド・オフ
	}

	midiOutReset(Mid_Dev.mp);
	Mid_Dev.state = MIDST_STOP;
}

// 各種テーブルの初期化 //
void Mid_TableInit(void)
{
	int		i,j;

	for(i=0;i<16;i++){
		for(j=0;j<128;j++){
			Mid_PlayTable[i][j]  = 0;
			Mid_PlayTable2[i][j] = 0;
			Mid_NoteTable[i][j]  = 0;
			Mid_NoteWTable[i][j] = 0;
		}

		Mid_PanpodTable[i]     = 0x40;
		Mid_ExpressionTable[i] = 0x7f;
		Mid_VolumeTable[i]     = 0x64;
	}
}

void Mid_Volume(BYTE volume)
{
	// マスター・ボリューム : F0 7F 7F 04 01 VolumeLowByte VolumeHighByte F7   //
	// 下位バイトは SC-88ST Pro では 00 として扱われるらしい(取扱説明書より) //

	BYTE	msg[8] = {0xf0,0x7f,0x7f,0x04,0x01,0x00,0x00,0xf7};
	MIDIHDR	mh;
/*
	union{
		DWORD	dd;
		struct{
			WORD	d1;
			WORD	d2;
		}w;
	} temp;
*/
	msg[6] = volume;

	mh.dwFlags        = 0;
	mh.dwOffset       = 0;
	mh.lpData         = msg;
	mh.dwBufferLength = mh.dwBytesRecorded = 8;
	midiOutPrepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
	midiOutLongMsg(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
	midiOutUnprepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));

	// これより下は削った方が良いかも //
	//temp.w.d1 = temp.w.d2 = volume;
	//midiOutSetVolume(Mid_Dev.mp,temp.dd);
}

void Mid_Tempo(char tempo)
{
	Mid_MulTempo = MID_STDTEMPO + tempo;
}

void Mid_FadeOut(BYTE speed)
{
	Mid_Dev.FadeFlag  = -1;
	Mid_Dev.FadeCount = 0;

	// MaxVolume,FadeWait に 1 だけ加算しているのは、０除算防止のため //
	Mid_Dev.FadeWait  = ((256-speed)*4)/(Mid_Dev.MaxVolume+1) + 1;
}

void Mid_Pan(char pan)
{
	int		i;
	int		value = 0x40 + pan;

	for(i=0;i<16;i++)
		Mid_ShortMsg(0xb0+i,0x0a,value);
}

static void Mid_ShortMsg(BYTE d1,BYTE d2,BYTE d3)
{
	BYTE	data[4];

	data[0] = d1;
	data[1] = d2;
	data[2] = d3;
	data[3] = 0;

	midiOutShortMsg(Mid_Dev.mp,*((DWORD *)data));
}

void Mid_GMReset(void)
{
	// GM SystemOn : F0H 7EH 7FH 09H 01H F7H //

	DWORD	time;
	BYTE	msg[6] = {0xf0,0x7e,0x7f,0x09,0x01,0xf7};
	MIDIHDR	mh;

	mh.dwFlags        = 0;
	mh.dwOffset       = 0;
	mh.dwBufferLength = mh.dwBytesRecorded = 11;

	mh.lpData         = msg;
	mh.dwBufferLength = mh.dwBytesRecorded = 6;
	midiOutPrepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
	midiOutLongMsg(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
	midiOutUnprepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));

	// ここで50ms以上待つこと! //
	time = timeGetTime();
	while(timeGetTime()-time<=50);
}

BOOL Mid_ChgDev(char pos)
{
	int		i,temp;
	UINT	mret;

	// 各関数に合わせて停止処理を行う //
	Mid_Stop();

	midiOutClose(Mid_Dev.mp);

	temp = Mid_Dev.nDevice + Mid_Dev.NowID + ((pos<0) ? -1 : 1);
	for(i=0;i<Mid_Dev.nDevice;i++){
		Mid_Dev.NowID = (pos<0) ? (temp-i)%(Mid_Dev.nDevice) : (temp+i)%(Mid_Dev.nDevice);
		mret = midiOutOpen(&Mid_Dev.mp,Mid_Dev.NowID-1,0,0,CALLBACK_NULL);
		if(mret==MMSYSERR_NOERROR){
			Mid_Play();
			return TRUE;
		}
	}

	// あり得ないはずだが... //
	return FALSE;
}

BOOL Mid_Load(char *filename)
{
	FILE				*fp;
	MID_FILEST			midhead;
	MID_MAINST			*midmain;
	MID_TRACKST			midtrack;
	MID_TRACK			*pt;
	DWORD				size;
	int					i;

	Mid_Free();
	if((fp=fopen(filename,"rb"))==NULL) return FALSE;
	fread(&midhead,sizeof(MID_FILEST),1,fp);
	if(midhead.MThd!=mmioFOURCC('M','T','h','d')){
		fclose(fp);
		return FALSE;
	}

	size = ConvDWord(midhead.size);
	midmain = (MID_MAINST *)LocalAlloc(LPTR,size);
	fread(midmain,size,1,fp);
	Mid_Data.format   = ConvWord(midmain->format);
	Mid_Data.track    = ConvWord(midmain->track);
	Mid_Data.timebase = ConvWord(midmain->timebase);
	Mid_Data.tempo    = 1000000;
	LocalFree(midmain);

	Mid_Data.data = (MID_TRACK *)LocalAlloc(LPTR,sizeof(MID_TRACK)*Mid_Data.track);
	ZeroMemory(Mid_Data.data,sizeof(MID_TRACK)*Mid_Data.track);

	for(i=0;i<Mid_Data.track;i++){
		fread(&midtrack,sizeof(MID_TRACKST),1,fp);
		pt = &(Mid_Data.data[i]);
		pt->size = ConvDWord(midtrack.size);
		pt->data = (BYTE *)LocalAlloc(LPTR,pt->size);
		pt->play = TRUE;
		fread(pt->data,pt->size,1,fp);
	}

	fclose(fp);
	Mid_Init();

	return TRUE;
}

BOOL PMid_Load(BIT_DEVICE *in,DWORD n)
{
	BYTE				*data,*dp;
	MID_FILEST			*midhead;
	MID_MAINST			*midmain;
	MID_TRACKST			*midtrack;
	MID_TRACK			*pt;
	DWORD				size;
	int					i;

	Mid_Free();

	if((data=MemExpand(in,n))==NULL) return FALSE;
	midhead = (MID_FILEST *)data;
	//fread(&midhead,sizeof(MID_FILEST),1,fp);
	if(midhead->MThd!=mmioFOURCC('M','T','h','d')){
		LocalFree(data);
		return FALSE;
	}

	size = ConvDWord(midhead->size);
	midmain = (MID_MAINST *)((BYTE *)data+sizeof(MID_FILEST));
	//fread(midmain,size,1,fp);
	Mid_Data.format   = ConvWord(midmain->format);
	Mid_Data.track    = ConvWord(midmain->track);
	Mid_Data.timebase = ConvWord(midmain->timebase);
	Mid_Data.tempo    = 1000000;

	Mid_Data.data = (MID_TRACK *)LocalAlloc(LPTR,sizeof(MID_TRACK)*Mid_Data.track);

	ZeroMemory(Mid_Data.data,sizeof(MID_TRACK)*Mid_Data.track);

	dp = (BYTE *)(data+sizeof(MID_FILEST)+size);
	for(i=0;i<Mid_Data.track;i++){
		midtrack = (MID_TRACKST *)dp;
		dp+=sizeof(MID_TRACKST);
		//fread(&midtrack,sizeof(MID_TRACKST),1,fp);
		pt = &(Mid_Data.data[i]);
		pt->size = ConvDWord(midtrack->size);
		pt->data = (BYTE *)LocalAlloc(LPTR,pt->size);
		pt->play = TRUE;
		//fread(pt->data,pt->size,1,fp);
		memcpy(pt->data,dp,pt->size);
		dp+=pt->size;
	}

	LocalFree(data);
	Mid_Init();

	return TRUE;
}

static BOOL Mid_Init(void)
{
	MID_TRACK	*p;
	int 		i;

	//Mid_Fade = 0;
	//Mid_MulTempo = MID_STDTEMPO;
	Mid_PlayTime    = 0;

	Mid_Data.playcount1 = 0;
	Mid_Data.playcount2 = 0;
	Mid_Data.fticks     = 0;
	Mid_Data.nticks     = 0;

	for(i=0;i<Mid_Data.track;i++){
		p = &(Mid_Data.data[i]);
		p->work  = p->data;
		p->play  = TRUE;
		p->count = GetWaitCount(&(p->work));		// 初期ウェイトカウントを読むの
	}

	return TRUE;
}

BOOL Mid_Free(void)
{
	int i;

	if(Mid_Data.data == NULL) return FALSE;

	for(i=0;i<Mid_Data.track;i++)
		LocalFree(Mid_Data.data[i].data);

	LocalFree(Mid_Data.data);
	Mid_Data.data  = NULL;
	Mid_Data.track = 0;

	return TRUE;
}

static WORD ConvWord(WORD data)
{
	WORD temp;

	((BYTE *)&temp)[0] = ((BYTE *)&data)[1];
	((BYTE *)&temp)[1] = ((BYTE *)&data)[0];

	return temp;
}

static DWORD ConvDWord(DWORD data)
{
	DWORD temp;

	((BYTE *)&temp)[0] = ((BYTE *)&data)[3];
	((BYTE *)&temp)[1] = ((BYTE *)&data)[2];
	((BYTE *)&temp)[2] = ((BYTE *)&data)[1];
	((BYTE *)&temp)[3] = ((BYTE *)&data)[0];

	return temp;
}

static DWORD GetWaitCount(LPBYTE *data)
{
	BYTE	temp;
	DWORD	ret = 0;

	do{
		temp = **data;
		++*data;
		ret = (ret<<7)+(temp&0x7f);
	}while(temp&0x80);

	return ret;
}

static void MidFadeIOFunc(void)
{
	int		track;

	if(Mid_Dev.FadeFlag==0) return;

	if(Mid_Dev.FadeCount % Mid_Dev.FadeWait == 0){
		Mid_Dev.NowVolume += Mid_Dev.FadeFlag;
		for(track=0;track<16;track++)
			Mid_ShortMsg(0xb0+track,0x07,Mid_VolumeTable[track]*Mid_Dev.NowVolume/(Mid_Dev.MaxVolume+1));
		//Mid_Volume(Mid_Dev.NowVolume);
		if(Mid_Dev.NowVolume==0 || Mid_Dev.NowVolume==Mid_Dev.MaxVolume){
			Mid_Dev.FadeFlag = 0;
			Mid_Stop();
		}
	}

	Mid_Dev.FadeCount++;
}

void Mid_LoopFunc(DWORD time)
{
	int			i;
	BOOL		flag = FALSE;
	MID_TRACK	*p;
	DWORDLONG	now  = Mid_Data.playcount2 + (DWORDLONG)Mid_Data.playcount1*Mid_Data.timebase*1000/Mid_Data.tempo;

	if(Mid_Dev.mp     == NULL) return;
	if(Mid_Data.data  == NULL) return;
	if(Mid_Dev.fnmode != MIDFN_MIDLOOP) return;

	Mid_PlayTime += time;

	for(i=0;i<Mid_Data.track;i++){
		p = &(Mid_Data.data[i]);
		if(p->play){
			flag = TRUE;
			while(p->play && p->count<=now)
				Mid_Parse(p);
		}
	}

	Mid_Data.playcount1+=((time*Mid_MulTempo)>>7);

	MidFadeIOFunc();

	if(!flag){
		switch(Mid_Dev.plmode){
			case(MIDPL_NORM):Mid_Init();	break;
			case(MIDPL_STOP):Mid_Stop();	break;
		}
	}
}

static void CALLBACK CBMid_TimeFunc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	int			i;
	BOOL		flag = FALSE;
	MID_TRACK	*p;
	DWORDLONG	now  = Mid_Data.playcount2 + (DWORDLONG)Mid_Data.playcount1*Mid_Data.timebase*1000/Mid_Data.tempo;

	for(i=0;i<Mid_Data.track;i++){
		p = &(Mid_Data.data[i]);
		if(p->play){
			flag = TRUE;
			while(p->play && p->count<=now)
				Mid_Parse(p);
		}
	}

	Mid_PlayTime += Mid_Time.delay;

	Mid_Data.playcount1+=((Mid_Time.delay*Mid_MulTempo)>>7);

//	Mid_Data.nticks = (Mid_Time.delay*1000 + Mid_Data.fticks) / Mid_Data.tempo;
//	Mid_Data.fticks = Mid_Time.delay*1000 % Mid_Data.tempo;
//	Mid_Data.nticks  = (Mid_Data.fticks + Mid_PlayTime * Mid_Data.timebase * 1000) / Mid_Data.tempo;
//	Mid_Data.fticks += (Mid_Data.timebase * Mid_PlayTime * 1000) - (Mid_Data.nticks * Mid_Data.tempo);

	MidFadeIOFunc();

	if(!flag){
		switch(Mid_Dev.plmode){
			case(MIDPL_NORM):Mid_Init();	break;
			case(MIDPL_STOP):Mid_Stop();	break;
		}
	}
}

static void Mid_Parse(MID_TRACK *track)
{
	int		i,count,countwork;
	BYTE	st1,st2;
	BYTE	data[4] = {0,0,0,0};

	st1 = *(track->work);
	if(st1<0x80)	st1 = track->status;
	else			track->work++;
	st2 = st1 & 0xf0;

	switch(st2){
		case(0xf0):					// ？バイト
			if(st1 == 0xf0){			// エクスクルーシブ
				MIDIHDR mh;
				countwork         = GetWaitCount(&(track->work));
				mh.lpData         = LocalAlloc(LPTR,countwork+1);
				mh.lpData[0]      = (BYTE)0xf0;
				mh.dwFlags        = mh.dwOffset = 0;
				mh.dwBufferLength = mh.dwBytesRecorded = countwork+1;
				for(i=0;i<countwork;i++)
					mh.lpData[i+1] = *(track->work++);
				midiOutPrepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
				midiOutLongMsg(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
				midiOutUnprepareHeader(Mid_Dev.mp,&mh,sizeof(MIDIHDR));
				LocalFree(mh.lpData);
			}
			else{						// 制御用データ(出力のないものだけ出力)
				BYTE code = *(track->work++);
				countwork = GetWaitCount(&(track->work));
				if(code==0x2f){			// トラック終了
					track->play = FALSE;
					return;
				}
				else if(code==0x51){	// テンポ
					Mid_Data.playcount2 += (DWORDLONG)Mid_Data.playcount1*Mid_Data.timebase*1000/Mid_Data.tempo;
					Mid_Data.playcount1 = 0;
					Mid_Data.tempo      = 0;
					for(i=0;i<countwork;i++)
						Mid_Data.tempo += (Mid_Data.tempo<<8)+(*(track->work++));
					// ここに謎の一行があります //
					break;
				}
				else					// その他(読み飛ばし)
					track->work += countwork;
			}
		break;

		case(0xb0):				// コントロールチェンジ
			switch(*(track->work)){
				case(0x07):	// ボリューム
					Mid_VolumeTable[st1&0x0f] = *(track->work+1);
				break;

				case(0x0a):	// パンポット
					Mid_PanpodTable[st1&0x0f] = *(track->work+1);
				break;

				case(0x0b):	// エクスプレッション
					Mid_ExpressionTable[st1&0x0f] = *(track->work+1);
				break;
			}

			data[0] = st1;
			data[1] = *(track->work++);
			data[2] = *(track->work++);
			midiOutShortMsg(Mid_Dev.mp,*((DWORD *)data));
		break;

		case(0x80):					// ノートオフ
			Mid_NoteTable[st1&0x0f][*(track->work)]  = *(track->work+1) = 0;

		case(0x90):case(0xa0):		// ３バイト：発音 or 変更
			if(Mid_PlayTable[st1&0x0f][*(track->work)] < *(track->work+1)){
				Mid_PlayTable[st1&0x0f][*(track->work)]  = *(track->work+1);
				Mid_PlayTable2[st1&0x0f][*(track->work)] = *(track->work+1);
			}
			//Mid_PlayTable[st1&0x0f][*(track->work)]  += *(track->work+1);
			//Mid_PlayTable2[st1&0x0f][*(track->work)] += *(track->work+1);
			Mid_NoteTable[st1&0x0f][*(track->work)]  = *(track->work+1);
			if(Mid_NoteTable[st1&0x0f][*(track->work)])
				Mid_NoteWTable[st1&0x0f][*(track->work)] = 5;

		case(0xe0):		// ３バイト
			data[0] = st1;
			data[1] = *(track->work++);
			data[2] = *(track->work++);
			midiOutShortMsg(Mid_Dev.mp,*((DWORD *)data));
		break;

		case(0xc0):case(0xd0):				// ２バイト
			data[0] = st1;
			data[1] = *(track->work++);
			midiOutShortMsg(Mid_Dev.mp,*((DWORD *)data));
		break;
	}

	track->status = st1;
	count = GetWaitCount(&(track->work));
	track->count += count;
}

char *Mid_GetTitle(void)
{
	static char temp[1000];
	int i;
	BYTE *p;

	memset(temp,0,1000);

	// 通常のファイル用 たまに変なファイルだと間違ったものを表示するが... //
	for(i=0;i<Mid_Data.track;i++){
		p = Mid_Data.data[i].data;
		while(!(p[0]==0xff && p[1]==0x2f && p[2]==0x00)){
			if(p[0]==0xff && p[1]==0x03){
				memcpy(temp,p+3,p[2]);
				return temp;
			}
			p++;
		}
	}

	// タイトルのはずなのに別のところに記述しているファイル用 //
	for(i=0;i<Mid_Data.track;i++){
		p = Mid_Data.data[i].data;
		while(!(p[0]==0xff && p[1]==0x2f && p[2]==0x00)){
			if(p[0]==0xff && p[1]==0x01){
				memcpy(temp,p+3,p[2]);
				return p+3;
			}
			p++;
		}
	}

	return temp;
}


// playcount1 の取得 //
DWORDLONG Mid_GetPlaycount1(void)
{
	return Mid_Data.playcount1;
}


// playcount2 の取得 //
DWORDLONG Mid_GetPlaycount2(void)
{
	return Mid_Data.playcount2;
}


// 全情報を取得 //
void Mid_GetData(MID_DATA *pData)
{
	*pData = Mid_Data;
}

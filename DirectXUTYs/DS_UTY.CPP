/*                                                                           */
/*   DS_UTY.cpp   効果音関連の処理                                           */
/*                                                                           */
/*                                                                           */

#include "DS_UTY.h"
#include <stdio.h>
#pragma message(PBGWIN_DS_UTY_H)

//#pragma optimize("",off)


// サウンド停止用補助関数 //
static void _Snd_Stop(LPSE_STRUCT se,int no);

LPDIRECTSOUND			Snd_DS = NULL;
LPSE_STRUCT				Snd_OBJ[SND_OBJ_MAX];
int						SndXMID = SND_XMID_DEFAULT;


FBOOL SndInit(HWND hWnd,int x_mid)
{
	HRESULT		dsrval;

	try{
		dsrval = DirectSoundCreate(NULL,&Snd_DS,NULL);
		if(dsrval != DS_OK) throw("DirectSoundCreate に失敗");

		dsrval = Snd_DS->SetCooperativeLevel(hWnd,DSSCL_NORMAL);
		if(dsrval != DS_OK) throw("SetCooperativeLevel(DSound) に失敗");

		SndXMID = x_mid;
	}
	catch(char *error){
		ErrInsert(error);
		SndCleanup();
		return FALSE;
	}

	return TRUE;
}

FVOID SndCleanup(void)
{
	int		i;

	// ここで SE_STRUCT 関連の解放を行う //
	for(i=0;i<SND_OBJ_MAX;i++){
		SndRelease(i);
	}

	// 念のため... //
	for(i=0;i<SND_OBJ_MAX;i++) Snd_OBJ[i] = NULL;

	// ＤＳオブジェクトを解放 //
	RELEASE(Snd_DS);
}

FBOOL SndWAVLoad(char *filename,BYTE id,int max)
{
	WAVEHEAD		wh;
	FILE			*fp;
	LPSE_STRUCT		p;
	HRESULT			dsrval;
	DSBUFFERDESC	dsBD;
	int				i;
	LPSE_STRUCT		*se;

	if(Snd_DS==NULL) return FALSE;

	if(id>=SND_OBJ_MAX) return FALSE;
	se = &Snd_OBJ[id];

	// SE_STRUCT 用のメモリを確保する //
	*se = p = (LPSE_STRUCT)LocalAlloc(LPTR,sizeof(SE_STRUCT)+(max-1)*sizeof(IDirectSoundBuffer *));
	if(p == NULL) return FALSE;


	// 読み込みは後で圧縮対応にするので今のところはちょっと手抜き //
	if((fp=fopen(filename,"rb"))==NULL) return FALSE;

	fread(&wh,sizeof(WAVEHEAD),1,fp);				// ヘッダ読み込み
	p->data = (BYTE *)LocalAlloc(LPTR,wh.DataLen);	// データ用メモリ確保
	fread(p->data,wh.DataLen,1,fp);					// データ読み込み
	fclose(fp);										// ファイルクローズ

	p->max  = max;
	p->now  = 0;
	p->size = wh.DataLen;

	dsBD.dwBufferBytes = wh.DataLen;
	dsBD.dwFlags       = DSBCAPS_STATIC|DSBCAPS_CTRLDEFAULT|DSBCAPS_GETCURRENTPOSITION2;
	dsBD.dwReserved    = 0;
	dsBD.dwSize        = sizeof(DSBUFFERDESC);
	dsBD.lpwfxFormat   = (LPWAVEFORMATEX)&wh.tag;

	dsrval = Snd_DS->CreateSoundBuffer(&dsBD,&p->buf[0],NULL);
	if(dsrval != DS_OK){
		ErrInsert("CreateSoundBuffer に失敗");
		return FALSE;
	}

	if(!SndFillBuffer(p->buf[0],p->data,p->size)){
		ErrInsert("SndFillBuffer に失敗 (PBGUTY_X.LIB)");
		return FALSE;
	}

	for(i=1;i<max;i++){
		dsrval = Snd_DS->DuplicateSoundBuffer(p->buf[0],&p->buf[i]);
		if(dsrval != DS_OK){
			if(!SndFillBuffer(p->buf[i],p->data,p->size)) return FALSE;
		}
	}

	return TRUE;
}

FBOOL SndWAVLoadP(BIT_DEVICE *in,DWORD n,BYTE id,int max)
{
	BYTE			*data;
	WAVEHEAD		*wh;
	WAVEHEAD_FACT	*whf;
	LPSE_STRUCT		p;
	HRESULT			dsrval;
	DSBUFFERDESC	dsBD;
	int				i;
	LPSE_STRUCT		*se;

	if(Snd_DS==NULL) return FALSE;

	if(id>=SND_OBJ_MAX) return FALSE;
	se = &Snd_OBJ[id];

	// SE_STRUCT 用のメモリを確保する //
	*se = p = (LPSE_STRUCT)LocalAlloc(LPTR,sizeof(SE_STRUCT)+(max-1)*sizeof(IDirectSoundBuffer *));
	if(p == NULL) return FALSE;

	// データを解凍して読み込む //
	if((data=MemExpand(in,n))==NULL) return FALSE;
	wh  = (WAVEHEAD *)data;
	whf = (WAVEHEAD_FACT *)data;

	if(whf->FACT != MAKEFOURCC('f','a','c','t')){
		p->data = (BYTE *)LocalAlloc(LPTR,wh->DataLen);
		memcpy(p->data,data+sizeof(WAVEHEAD),wh->DataLen);

		p->max  = max;
		p->now  = 0;
		p->size = wh->DataLen;

		dsBD.dwBufferBytes = wh->DataLen;
		dsBD.dwFlags       = DSBCAPS_STATIC|DSBCAPS_CTRLDEFAULT|DSBCAPS_GETCURRENTPOSITION2;
		dsBD.dwReserved    = 0;
		dsBD.dwSize        = sizeof(DSBUFFERDESC);
		dsBD.lpwfxFormat   = (LPWAVEFORMATEX)&wh->tag;
	}
	else{
		p->data = (BYTE *)LocalAlloc(LPTR,whf->DataLen);
		memcpy(p->data,data+sizeof(WAVEHEAD_FACT),whf->DataLen);

		p->max  = max;
		p->now  = 0;
		p->size = whf->DataLen;

		dsBD.dwBufferBytes = whf->DataLen;
		dsBD.dwFlags       = DSBCAPS_STATIC|DSBCAPS_CTRLDEFAULT|DSBCAPS_GETCURRENTPOSITION2;
		dsBD.dwReserved    = 0;
		dsBD.dwSize        = sizeof(DSBUFFERDESC);
		dsBD.lpwfxFormat   = (LPWAVEFORMATEX)&whf->tag;
	}

	dsrval = Snd_DS->CreateSoundBuffer(&dsBD,&p->buf[0],NULL);
	if(dsrval != DS_OK){
		ErrInsert("CreateSoundBuffer に失敗");
		return FALSE;
	}

	if(!SndFillBuffer(p->buf[0],p->data,p->size)){
		ErrInsert("SndFillBuffer に失敗 (PBGUTY_X.LIB)");
		return FALSE;
	}

	for(i=1;i<max;i++){
		dsrval = Snd_DS->DuplicateSoundBuffer(p->buf[0],&p->buf[i]);
		if(dsrval != DS_OK){
			if(!SndFillBuffer(p->buf[i],p->data,p->size)) return FALSE;
		}
	}

	LocalFree(data);

	return TRUE;
}

FVOID SndPlay(BYTE id)
{
	HRESULT	dsrval;
	DWORD	st;
	LPSE_STRUCT se;

	if(id>=SND_OBJ_MAX) return;

	se = Snd_OBJ[id];
	if(se == NULL || se->buf[se->now] == NULL) return;

	dsrval = se->buf[se->now]->GetStatus(&st);
	if(dsrval != DS_OK) st=0;
	if((st&DSBSTATUS_PLAYING)==DSBSTATUS_PLAYING) _Snd_Stop(se,se->now);

	se->buf[se->now]->SetPan(0);
	se->buf[se->now]->Play(0,0,0);
	se->now = (se->now+1)%(se->max);
}

FVOID SndPlayEX(BYTE id,int x,DWORD flag)
{
	HRESULT	dsrval;
	DWORD	st;
	DWORD	pan = (x - SndXMID)>>4;
	LPSE_STRUCT se;

	if(id>=SND_OBJ_MAX) return;

	se = Snd_OBJ[id];
	if(se == NULL || se->buf[se->now] == NULL) return;

	dsrval = se->buf[se->now]->GetStatus(&st);
	if(dsrval != DS_OK) st=0;
	if((st&DSBSTATUS_PLAYING)==DSBSTATUS_PLAYING) _Snd_Stop(se,se->now);

	se->buf[se->now]->SetPan(pan);
	se->buf[se->now]->Play(0,0,flag);
	se->now = (se->now+1)%(se->max);
}

FVOID SndStop(BYTE id)
{
	int i;
	LPSE_STRUCT se;

	if(id>=SND_OBJ_MAX) return;

	se = Snd_OBJ[id];
	if(se == NULL) return;

	for(i=0;i<se->max;i++){
		if(se->buf[i]==NULL) continue;
		se->buf[i]->Stop();
		se->buf[i]->SetCurrentPosition(0);
	}
}

FVOID SndStopAll(void)
{
	int i;

	for(i=0;i<SND_OBJ_MAX;i++) SndStop(i);
}

FBOOL SndFillBuffer(IDirectSoundBuffer *ds,BYTE *data,DWORD size)
{
	LPVOID	pMem1,pMem2;
	DWORD	dwSize1,dwSize2;
	HRESULT	dsrval;

	if(ds == NULL) return FALSE;

	dsrval = ds->Lock(0,size,&pMem1,&dwSize1,&pMem2,&dwSize2,0);
	if(dsrval != DS_OK) return FALSE;

	CopyMemory(pMem1,data,dwSize1);
	if(dwSize2 != 0)
		CopyMemory(pMem2,data+dwSize1,dwSize2);

	ds->Unlock(pMem1,dwSize1,pMem2,dwSize2);

	return TRUE;
}

FVOID SndRelease(BYTE id)
{
	int i;
	LPSE_STRUCT se;

	if(id>=SND_OBJ_MAX) return;

	se = Snd_OBJ[id];
	if(se == NULL) return;

	for(i=0;i<se->max;i++){
		if(se->buf[i] != NULL){
			se->buf[i]->Release();
			se->buf[i] = NULL;
		}
	}

	if(se->data != NULL) LocalFree(se->data);
	se->data = NULL;

	LocalFree(se);
}

FBOOL SndRestore(void)
{
	return FALSE;
}

static void _Snd_Stop(LPSE_STRUCT se,int no)
{
	if(se == NULL || se->buf[no]==NULL) return;

	se->buf[no]->Stop();
	se->buf[no]->SetCurrentPosition(0);
}

//#pragma optimize("",on)

/*                                                                           */
/*   DemoPlay.cpp   デモプレイ処理                                           */
/*                                                                           */
/*                                                                           */

#include "DemoPlay.h"


BOOL			DemoplaySaveEnable = FALSE;		// デモプレイのセーブが動作しているか
BOOL			DemoplayLoadEnable = FALSE;		// デモプレイのロードが動作しているか
DEMOPLAY_INFO	DemoInfo;						// デモプレイ情報
WORD			DemoBuffer[DEMOBUF_MAX];		// デモプレイ用バッファ
DWORD			DemoBufPtr;						// デモプレイ用バッファPtr(Ｃのそれではない)
CONFIG_DATA		ConfigTemp;						// コンフィグのデータ一時保存用

//DWORD RndBuf[RNDBUF_MAX];



// デモプレイデータの準備 //
FVOID DemoplayInit(void)
{
	// 乱数の準備 //
	DemoInfo.RndSeed = (DWORD)(rnd()+1)*(DWORD)(rnd()+1);
	rnd_seed_set(DemoInfo.RndSeed);

	DemoInfo.Exp    = Viv.exp;
	DemoInfo.Weapon = Viv.weapon;
	DemoInfo.CfgDat = ConfigDat;

	DemoBufPtr         = 0;
	DemoplaySaveEnable = TRUE;
}


// デモプレイデータを保存する //
FVOID DemoplayRecord(WORD key)
{
	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr++] = key;

	// バッファが最後に来たか、ＥＳＣが押された場合 //
	if(DemoBufPtr==DEMOBUF_MAX || (key&KEY_ESC)){
		DemoBufPtr--;
		DemoplaySave();
		DemoplaySaveEnable = FALSE;
	}
}


// デモプレイデータをファイルに書き込む //
FVOID DemoplaySave(void)
{
	char		buf[100];
	FILE		*fp;

	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr] = KEY_ESC;
	DemoInfo.DataSize = DemoBufPtr+1;

	sprintf(buf,"STG%dDemo.DAT",GameStage);
	fp = fopen(buf,"wb");
	if(fp==NULL) return;

	fwrite(&DemoInfo,sizeof(DEMOPLAY_INFO),1,fp);
	fwrite(DemoBuffer,sizeof(WORD)*(DemoBufPtr+1),1,fp);
	fclose(fp);

	DemoplaySaveEnable = FALSE;
}


// デモプレイデータをロードする //
FBOOL DemoplayLoad(int stage)
{
	BIT_DEVICE		*in;
	BYTE			*temp,*p;

	// ファイルを開く //
	in=FilStartR("ENEMY.DAT",BD_FILE_READ);
	if(in == NULL) return FALSE;

	// 展開 //
	temp = MemExpand(in,stage-1+18);
	if(temp == NULL) return FALSE;
	FilEnd(in);

	p = temp;
	memcpy(&DemoInfo,p,sizeof(DEMOPLAY_INFO));

	p = temp + sizeof(DEMOPLAY_INFO);
	memcpy(DemoBuffer,p,sizeof(WORD)*DemoInfo.DataSize);

	LocalFree(temp);

	DemoBufPtr = 0;
	DemoplayLoadEnable = TRUE;

	// コンフィグの初期化 //
	ConfigTemp = ConfigDat;

	ConfigDat.BombStock   = DemoInfo.CfgDat.BombStock;
	ConfigDat.GameLevel   = DemoInfo.CfgDat.GameLevel;
	ConfigDat.PlayerStock = DemoInfo.CfgDat.PlayerStock;
	ConfigDat.InputFlags  = DemoInfo.CfgDat.InputFlags;

	Viv.exp    = DemoInfo.Exp;
	Viv.weapon = DemoInfo.Weapon;
	Viv.left   = ConfigDat.PlayerStock;
	Viv.bomb   = ConfigDat.BombStock;

	// 乱数の初期化 //
	rnd_seed_set(DemoInfo.RndSeed);

	return TRUE;
}


// Key_Data を返す //
FWORD DemoplayMove(void)
{
	DWORD ptr;

	if(!DemoplayLoadEnable) return KEY_ESC;

	ptr = DemoBufPtr;
	if(ptr>=DemoInfo.DataSize){
		ptr = DemoBufPtr;
		DemoplayLoadEnable = FALSE;
	}
	else DemoBufPtr++;

	return DemoBuffer[ptr];
}


// デモプレイロードの事後処理 //
FVOID DemoplayLoadCleanup(void)
{
	ConfigDat          = ConfigTemp;
	DemoplayLoadEnable = FALSE;
}


// リプレイデータの準備 //
FVOID ReplayInit(void)
{
	// これは同じ //
	DemoplayInit();
}


// リプレイデータを保存する //
FVOID ReplayRecord(WORD key)
{
	if(!DemoplaySaveEnable) return;

	DemoBuffer[DemoBufPtr++] = key;

	if(DemoBufPtr == DEMOBUF_MAX || (key&KEY_ESC)){
		DemoBufPtr--;
		ReplaySave();					// 強制吐き出し
		DemoplaySaveEnable = FALSE;
	}
}


// リプレイデータをファイルに書き込む //
FVOID ReplaySave(void)
{
	BIT_DEVICE		*in, *out;
	char			buf[100];

	if(!DemoplaySaveEnable) return;

	// すぐさま、無効化する //
	DemoplaySaveEnable = FALSE;

	DemoBuffer[DemoBufPtr] = KEY_ESC;
	DemoInfo.DataSize = DemoBufPtr+1;

	// ファイル名を確定 //
	if(GameStage == GRAPH_ID_EXSTAGE) strcpy(buf, "秋霜りぷEx.DAT");
	else                              sprintf(buf, "秋霜りぷ%d.DAT", GameStage);

	// 仮ヘッダね //
	FileHead.name = PBG_HEADNAME;
	FileHead.n    = 2;
	FileHead.sum  = 0;

	out = FilStartW(buf, BD_FILE_WRITE, FileHead.n);
	if(NULL == out) return;

	fwrite(&FileHead,sizeof(PBG_FILEHEAD),1,out->p.file);
	fwrite(FileInfo,sizeof(PBG_FILEINFO),FileHead.n,out->p.file);

	in = BitMemCreate((BYTE *)&DemoInfo, BD_MEMORY_READ, sizeof(DEMOPLAY_INFO));
	Compress(in, out, 0);
	BitDevRelease(in);

	in = BitMemCreate((BYTE *)DemoBuffer, BD_MEMORY_READ, sizeof(WORD) * DemoInfo.DataSize);
	Compress(in, out, 1);
	BitDevRelease(in);

	WriteHead(out);
	FilEnd(out);
}


// リプレイデータをロードする //
FBOOL ReplayLoad(int stage)
{
	BIT_DEVICE		*in;
	BYTE			*temp;
	char			buf[100];

	// ファイル名を確定 //
	if(GameStage == GRAPH_ID_EXSTAGE) strcpy(buf, "秋霜りぷEx.DAT");
	else                              sprintf(buf, "秋霜りぷ%d.DAT", stage);

	in = FilStartR(buf, BD_FILE_READ);
	if(NULL == in) return FALSE;

	// ヘッダの格納先は０番である //
	temp = MemExpand(in, 0);
	if(NULL == temp){
		FilEnd(in);
		return FALSE;
	}
	memcpy(&DemoInfo, temp, sizeof(DEMOPLAY_INFO));
	LocalFree(temp);

	// データの格納先は１番ですね //
	temp = MemExpand(in, 1);
	if(NULL == temp){
		FilEnd(in);
		return FALSE;
	}
	memcpy(DemoBuffer, temp, sizeof(WORD)*DemoInfo.DataSize);
	LocalFree(temp);

	FilEnd(in);

	DemoBufPtr = 0;
	DemoplayLoadEnable = TRUE;

	// 現在のコンフィグを保持する //
	ConfigTemp = ConfigDat;

	// そのときのコンフィグを転送 //
	ConfigDat.BombStock   = DemoInfo.CfgDat.BombStock;
	ConfigDat.GameLevel   = DemoInfo.CfgDat.GameLevel;
	ConfigDat.PlayerStock = DemoInfo.CfgDat.PlayerStock;
	ConfigDat.InputFlags  = DemoInfo.CfgDat.InputFlags;

	// 本体の性能記述 //
	Viv.exp    = DemoInfo.Exp;
	Viv.weapon = DemoInfo.Weapon;
	Viv.left   = ConfigDat.PlayerStock;
	Viv.bomb   = ConfigDat.BombStock;

	// 最後に乱数もそろえる //
	rnd_seed_set(DemoInfo.RndSeed);

	return TRUE;
}


// Key_Data を返す //
FWORD ReplayMove(void)
{
	DWORD	ptr;

	if(!DemoplayLoadEnable) return KEY_ESC;

	ptr = DemoBufPtr;

	if(ptr >= DemoInfo.DataSize){
		ptr = DemoBufPtr;
		DemoplayLoadEnable = FALSE;
	}
	else
		DemoBufPtr++;

	return DemoBuffer[ptr];
}


// リプレイデータロードの事後処理 //
FVOID ReplayLoadCleanup(void)
{
	ConfigDat = ConfigTemp;
	DemoplayLoadEnable = FALSE;
}

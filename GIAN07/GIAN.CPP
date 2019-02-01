/*                                                                           */
/*   GIAN.cpp   ゲーム全体の管理                                             */
/*                                                                           */
/*                                                                           */

#include "GIAN.h"
#include <time.h>


///// [グローバル変数] /////
CONFIG_DATA		ConfigDat;
//HIGH_SCORE		*HighScore;
//char			ScoreTable[8][80];
DWORD			GameCount;
BYTE			GameStage;

#ifdef PBG_DEBUG
DEBUG_DATA		DebugDat;
#endif



///// [ 関数(非公開) ] /////
static void ConfigInit(void);

#ifdef PBG_DEBUG
static void DebugInit(void);
#endif



// コンフィグの内容を初期化する //
extern void ConfigLoad(void)
{
	FILE		*fp;
	DWORD		tmp;

#ifdef PBG_DEBUG
	DebugInit();
#endif

	while(1){
		fp = fopen("秋霜CFG.DAT","rb");
		if(fp==NULL) break;

		fread(&ConfigDat,sizeof(CONFIG_DATA),1,fp);
		fclose(fp);

		tmp = 0;
		tmp += (ConfigDat.BitDepth << 1);
		tmp += (ConfigDat.BombStock << 3);
		tmp += (ConfigDat.DeviceID << 2);
		tmp += (ConfigDat.DrawMode << 6);
		tmp += (ConfigDat.ExtraStgFlags << 5);
		tmp += (ConfigDat.GameLevel << 2);
		tmp += (ConfigDat.GraphFlags << 1);
		tmp += (ConfigDat.InputFlags << 3);
		tmp += (ConfigDat.PadBomb << 6);
		tmp += (ConfigDat.PadCancel << 3);
		tmp += (ConfigDat.PadShift << 1);
		tmp += (ConfigDat.PadTama << 4);
		tmp += (ConfigDat.PlayerStock << 5);
		tmp += (ConfigDat.SoundFlags << 2);
		if(tmp != ConfigDat.CheckSum) break;

		// 列挙したデバイス数と比較して異常ならここで修正する //
		// ここに来た場合は、ファイルが壊れているかビデオカードが抜かれたかの //
		// どちらかなので、強制的に break しない。                            //
		if(ConfigDat.DeviceID>=DxEnumNow) ConfigDat.DeviceID = 0;

		if(ConfigDat.PlayerStock>6) break;
		if(ConfigDat.PlayerStock<0) break;
		if(ConfigDat.BombStock>3) break;

		// ここでは、ビット深度が８か１６である事を保証する //
		// まずあり得ないことだが、ビデオカードが８ビットをサポートしていない //
		// 場合のチェックも行い、起動ができなくなる事を防ぐ。                 //
		if(ConfigDat.BitDepth!=8 && ConfigDat.BitDepth!=16) break;
		if(ConfigDat.BitDepth== 8 && DxEnum[ConfigDat.DeviceID].b8Bit ==FALSE) break;		// 256
		if(ConfigDat.BitDepth==16 && DxEnum[ConfigDat.DeviceID].b16Bit==FALSE) break;		// 2D
		if(ConfigDat.BitDepth==16 && DxEnum[ConfigDat.DeviceID].D3D->b16Bit==FALSE) break;	// 3D

		if(ConfigDat.DrawMode>3) break;
		if(ConfigDat.GameLevel>3) break;
		if(ConfigDat.GraphFlags&GRPF_MASK) break;

		if(ConfigDat.SoundFlags&SNDF_MASK) break;

		if(ConfigDat.InputFlags&INPF_MASK) break;

		if(ConfigDat.DebugFlags&DBGF_MASK) break;

		if(ConfigDat.PadTama   > 32) break;
		if(ConfigDat.PadBomb   > 32) break;
		if(ConfigDat.PadShift  > 32) break;
		if(ConfigDat.PadCancel > 32) break;

		// これだけは、特別扱い //
		if(ConfigDat.StageSelect > 6) ConfigDat.StageSelect = 0;

		return;
	}

	ConfigInit();
}

// コンフィグの内容を保存する //
extern void ConfigSave(void)
{
	FILE		*fp;
	BYTE		*p;
	DWORD		sum;

	fp = fopen("秋霜CFG.DAT","wb");
	if(fp==NULL) return;

	ConfigDat.CheckSum = 0;
	ConfigDat.CheckSum += (ConfigDat.BitDepth << 1);
	ConfigDat.CheckSum += (ConfigDat.BombStock << 3);
	ConfigDat.CheckSum += (ConfigDat.DeviceID << 2);
	ConfigDat.CheckSum += (ConfigDat.DrawMode << 6);
	ConfigDat.CheckSum += (ConfigDat.ExtraStgFlags << 5);
	ConfigDat.CheckSum += (ConfigDat.GameLevel << 2);
	ConfigDat.CheckSum += (ConfigDat.GraphFlags << 1);
	ConfigDat.CheckSum += (ConfigDat.InputFlags << 3);
	ConfigDat.CheckSum += (ConfigDat.PadBomb << 6);
	ConfigDat.CheckSum += (ConfigDat.PadCancel << 3);
	ConfigDat.CheckSum += (ConfigDat.PadShift << 1);
	ConfigDat.CheckSum += (ConfigDat.PadTama << 4);
	ConfigDat.CheckSum += (ConfigDat.PlayerStock << 5);
	ConfigDat.CheckSum += (ConfigDat.SoundFlags << 2);

	fwrite(&ConfigDat,sizeof(CONFIG_DATA),1,fp);
	fclose(fp);

#ifdef PBG_DEBUG
	fp = fopen("秋霜DBG.DAT","wb");
	if(fp==NULL) return;
	fwrite(&DebugDat,sizeof(DEBUG_DATA),1,fp);
	fclose(fp);
#endif
}

#ifdef PBG_DEBUG
#include <time.h>
// デバッグ情報を出力する //
extern FVOID OutputDebugMsg(void)
{
	static DWORD	fps,count;
	static DWORD	time,temp;
	//extern InputConfig			IConfig;
	char	*DItem[4] = {"Easy","Norm","Hard","Luna"};
	char	buf[100];

	if(!DebugDat.MsgDisplay) return;

	temp = timeGetTime();
	if(temp-time<=1000) count++;
	else{
		fps   = count;
		count = 0;
		time  = temp;
	}

	sprintf(buf,"%03d Fps",fps);
	GrpPut16(0,0,buf);

	sprintf(buf,"%2dBppMode",ConfigDat.BitDepth);
	GrpPut16(0,32,buf);
	//sprintf(buf,"%s",DItem[ConfigDat.GameLevel]);
	//GrpPut16(0,50,buf);

	sprintf(buf,"%s",DItem[PlayRank.GameLevel]);
	GrpPut16(0,50,buf);
	sprintf(buf,"%s<Cfg>",DItem[ConfigDat.GameLevel]);
	GrpPut16(0,68,buf);
	sprintf(buf,"Pr %d",PlayRank.Rank);
	GrpPut16(0,86,buf);


	sprintf(buf,"Enemy %3d",EnemyNow);
	GrpPut16(0,96+40,buf);

	sprintf(buf,"Tama1 %3d",Tama1Now);
	GrpPut16(0,128+40,buf);
	sprintf(buf,"Tama2 %3d",Tama2Now);
	GrpPut16(0,148+40,buf);
	sprintf(buf,"Laser %3d",LaserNow);
	GrpPut16(0,176+40,buf);
	sprintf(buf,"HLaser %2d",HLaserNow);
	GrpPut16(0,196+40,buf);

	sprintf(buf,"MTama %3d",MaidTamaNow);
	GrpPut16(0,224+40,buf);

	sprintf(buf,"Item  %3d",ItemNow);
	GrpPut16(0,252+40,buf);

	sprintf(buf,"Pow   %3d",Viv.exp);
	GrpPut16(0,290+40,buf);

	sprintf(buf,"SSPD  %3d",ScrollInfo.ScrollSpeed);
	GrpPut16(0,320+40,buf);

	GrpPut16(0,440,"Gian07");
	GrpPut16(0,460,"DebugMode");

	_strdate(buf);
	GrpPut16(640-128,0,"Date");
	GrpPut16(640-128,20,buf);

	_strtime(buf);
	GrpPut16(640-128,50,"Time");
	GrpPut16(640-128,70,buf);

	GrpPut16(640-128,100,"SCL Count");
	sprintf(buf," %5d",GameCount);
	GrpPut16(640-128,120,buf);

	sprintf(buf,"Left   %d",Viv.left);
	GrpPut16(640-128,440,buf);
	sprintf(buf,"Credit %d",Viv.credit);		// -1 に注意だ！！
	GrpPut16(640-128,460,buf);
}
#endif

extern FVOID StdStatusOutput(void)
{
	static DWORD	fps,count;
	static DWORD	time,temp;
	//extern InputConfig			IConfig;
	char	*DItem[4] = {"Easy","Norm","Hard","Luna"};
	char	buf[100];

//	if(!DebugDat.MsgDisplay) return;

	temp = timeGetTime();
	if(temp-time<=1000) count++;
	else{
		fps   = count;
		count = 0;
		time  = temp;
	}
/*
	sprintf(buf,"%03d Fps",fps);
	GrpPut16(0,0,buf);

	sprintf(buf,"%2dBppMode",ConfigDat.BitDepth);
	GrpPut16(0,32,buf);

	sprintf(buf,"%s<Cfg>",DItem[ConfigDat.GameLevel]);
	GrpPut16(0,68,buf);
*/
//	GrpPut16(0,440,"G07");
//	GrpPut16(0,460,"12/5 Ver");

	_strdate(buf);
	GrpPut16(640-128,0,"Date");
	GrpPut16(640-128,20,buf);

	_strtime(buf);
	GrpPut16(640-128,50,"Time");
	GrpPut16(640-128,70,buf);

	sprintf(buf,"Bomb   %d",Viv.bomb);
	GrpPut16(640-128,400,buf);

	sprintf(buf,"Left   %d",Viv.left);
	GrpPut16(640-128,440,buf);
	sprintf(buf,"Credit %d",Viv.credit);		// -1 に注意だ！！
	GrpPut16(640-128,460,buf);
}

extern FVOID swap16(WORD *a,WORD *b)
{
	WORD temp = *a;

	*a = *b;
	*b = temp;
}

static void ConfigInit(void)
{
	// 難易度関連 //
	ConfigDat.BombStock   = 2;
	ConfigDat.GameLevel   = GAME_NORMAL;
	ConfigDat.PlayerStock = 2;

	// グラフィック関連 //
	ConfigDat.DeviceID   = 0;
	ConfigDat.BitDepth   = 8;
	ConfigDat.DrawMode   = 1;		// 60Fps Mode
	ConfigDat.GraphFlags = 0;

	// サウンド・ＭＩＤＩ関連 //
	ConfigDat.SoundFlags = SNDF_WAVE_ENABLE|SNDF_MIDI_ENABLE;

	// 入力関連 //
	ConfigDat.InputFlags = INPF_Z_MSKIP_ENABLE;

	// デバッグ関連 //
	ConfigDat.DebugFlags = 0;

	// パッド関連 //
	ConfigDat.PadTama   = 1;
	ConfigDat.PadBomb   = 2;
	ConfigDat.PadShift  = 0;
	ConfigDat.PadCancel = 0;

	ConfigDat.ExtraStgFlags = 0;

	// その他 //
	ConfigDat.StageSelect = 0;
}

#ifdef PBG_DEBUG
static void DebugInit(void)
{
	FILE	*fp;

	fp = fopen("秋霜DBG.DAT","rb");
	if(fp==NULL){
		DebugDat.Hit        = TRUE;
		DebugDat.MsgDisplay = TRUE;
		DebugDat.DemoSave   = FALSE;
		DebugDat.StgSelect  = 1;
	}
	else{
		fread(&DebugDat,sizeof(DEBUG_DATA),1,fp);
		fclose(fp);
	}
}
#endif

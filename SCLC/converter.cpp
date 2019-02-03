/*
 * [座標を指定する際の注意]
 *  座標指定には、GX_???,GY_??? 等のグローバル定数が使える
 *  ちなみに、グラフィック座標で指定すること(x64 座標じゃない)
 */

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "CALC.H"
#include "..\\Gian07SrcFiles\\GIAN.H"

// ログファイル名(定義しない場合、ログを出力しない) //
#define SCL_LOGFILENAME	"SCL_LOG.TXT"

#define IsPBGCode(c)	(isspace(c) || (c)==',' || (c)==';' || (c)=='\0' || (c)=='\n')

#define PE_ERROR		0x00000001	// エラー(終了する)
#define PE_WARN			0x00000002	// 警告(出力のみ)
#define PE_STRING		0x00000003	// 文字列(行数表示無し)

#define CONV_EOF		0x00000000	// 終了
#define CONV_NORMAL		0x00000001	// 独立した文字列
#define CONV_COMMA		0x00000002	// コンマ
#define CONV_EXCMD		0x00000008	// コンバータ制御用コマンド
#define CONV_GLABEL		0x00000010	// グローバルラベル
#define CONV_LABEL		0x00000020	// ローカルラベル

#define CMD_MAX		17			// CMD の数
#define CMD_TA		0x00		// TA
#define CMD_TR		0x01		// TR
#define CMD_SSP		0x02		// SSP
#define CMD_EFC		0x03		// EFC
#define CMD_NAME	0x04		// NAME
#define CMD_BOSS	0x05		// BOSS

#define CMD_MWOPEN		0x06		// MWOPEN
#define CMD_MWCLOSE		0x07		// MWCLOSE
#define CMD_MSG			0x08		// MSG
#define CMD_KEY			0x09		// KEY
#define CMD_NPG			0x0a		// NPG
#define CMD_FACE		0x0b		// FACE
#define CMD_MUSIC		0x0c		// MUSIC
#define CMD_BOSSDEAD	0x0d		// BOSSDEAD
#define CMD_LOADFACE	0x0e		// LOADFACE
#define CMD_WAITEX		0x0f		// WAITEX
#define CMD_STAGECLEAR	0x10		// STAGECLEAR

const char *Cmd[CMD_MAX] = {"TA","TR","SSP","EFC","NAME","BOSS",
							"MWOPEN","MWCLOSE","MSG","KEY","NPG","FACE","MUSIC",
							"BOSSDEAD","LOADFACE","WAITEX","STAGECLEAR"};


typedef struct lbl{
	char			*name;		// ラベル名
	long			addr;		// アドレス格納用
	DWORD			line;		// エラーの行数保存用
	struct lbl		*next;		// 次のラベルへのポインタ
} LABEL_ST,*PLABEL_ST;

typedef struct cst{
	char			*name;		// 定数名
	char			*value;		// 定数の値
	struct cst		*next;		// 次の定数へのポインタ
} CONST_ST,*PCONST_ST;

typedef struct{
	FILE		*in;		// 入力用ファイルポインタ
	FILE		*out;		// 出力用ファイルポインタ
	FILE		*log;		// ログ用ファイルポインタ

	char		*paline;	// 現在解析中の行バッファ
	char		*paword;	// 現在解析中の単語バッファ
	char		*paptr;		// 行における解析中単語へのポインタ

	DWORD		line;		// コンバート中の行
	DWORD		block;		// 何番目の敵ブロックか
	long		addr;		// 現在位置
	DWORD		szline;		// 一行の最大長

	DWORD		kind;		// 敵の種類
	long		*saddr;		// ＥＣＬ開始アドレス配列

	PCONST_ST	lconst;		// ローカル定数
	PCONST_ST	gconst;		// グローバル定数

	LABEL_ST	*llabel;	// ローカルラベル
	LABEL_ST	*llabel_t;	// ローカルラベル（先読み用）
	LABEL_ST	*glabel;	// グローバルラベル
	LABEL_ST	*glabel_t;	// グローバルラベル（先読み用）
} CONVERTER;


// グローバル変数 //
DWORD			*pConvertLine = NULL;		// コンバート行の監視用
FILE			**ppLogFile   = NULL;		// ログファイルのポインタ監視用
BOOL			PrintConst    = FALSE;		// 定数の定義を出力するか

// 関数 //
void  SetupConverter(CONVERTER *cvt);							// コンバータを初期化する
void  EndConverter(CONVERTER *cvt);								// コンバータの後始末をする

void  ConvertConst(CONST_ST *cst,char *s);						// 定数を置換する
void  SetConst(PCONST_ST *cst,char *name,char *value);			// 定数をセットする
void  ReleaseConst(CONST_ST *cst);								// 定数用のメモリを解放する

DWORD GetNextWord(CONVERTER *cvt);								// 次の単語に移動する
DWORD GetCmd(char *cmd);										// CMD定数を得る
char  *GetNextWordEx(CONVERTER *cvt);							// 次の文字列を取得(MSG専用)

void  WriteData1( CONVERTER *cvt,BYTE  data);					// １バイト書き込む
void  WriteData2( CONVERTER *cvt,WORD  data);					// ２バイト書き込む
void  WriteData4( CONVERTER *cvt,DWORD data);					// ３バイト書き込む

char *ItoS(int val);											// int整数から文字列へ
BOOL  IsName(char *s);											// その文字列は名前か？
void  CalcError(char *s);										// Calc() 用エラー出力関数
void  PrintError(char *s,DWORD flags);							// エラーを出力する


void main(int argc,char *argv[])
{
	CONVERTER	cvt;
	char		buffer[1000];
	char		*p,path[1000]="";
	DWORD		SclTime = 0;
	DWORD		EnemyTime = 0;
	DWORD		temp,timetemp;
	int			dtimetemp;
	short		x,y;
	BYTE		n;

	puts("");
	puts("");

#ifdef SCL_LOGFILENAME
	puts("[内部拡張情報]");
	puts(" ログファイルの出力先  :  "SCL_LOGFILENAME"\n");
#endif
	puts("[内蔵関数のバージョン情報]");
	puts(" >> BossConverter(Level2)   : Version 0.02 <<");
	puts(" >> SCL_Converter++         : Version 0.04 <<\n\n");

	SetupConverter(&cvt);

	if(argc!=2)
		PrintError("SCLC <FILENAME>",PE_ERROR);

	cvt.in = fopen(argv[1],"r");
	if(cvt.in==NULL)
		PrintError("ファイルがみつからないよ",PE_ERROR);

	if(strchr(argv[1],'\\')!=NULL){
		strcpy(path,argv[1]);
		for(p=path+strlen(path);*(p-1)!='\\';p--);
		*p = '\0';
	}

	while((temp=GetNextWord(&cvt))!=CONV_EOF){
		if(temp!=CONV_NORMAL)
			PrintError("謎のコマンド？があるよ",PE_ERROR);

		switch(GetCmd(cvt.paword)){
			case(CMD_TA):
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("時間の指定が無いよ",PE_ERROR);
				timetemp = Calc(cvt.paword);
				if(timetemp==SclTime){	// 一致したときは何もしない
					PrintError("この TA は必要がないよ(コンバートしません)",PE_WARN);
					break;
				}
				if(timetemp<SclTime)
					PrintError("あらあら、時間が戻っていますよ",PE_ERROR);

				// TIME 命令を書き込む //
				SclTime = timetemp;
				WriteData1(&cvt,(BYTE)SCL_TIME);
				WriteData4(&cvt,(DWORD)SclTime);
				//printf("time = %d\n",SclTime);
			break;

			case(CMD_TR):
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("時間の指定が無いよ",PE_ERROR);
				dtimetemp = Calc(cvt.paword);
				if(dtimetemp==0){	// 変化しないときは何もしない
					PrintError("この TR は必要がないよ(コンバートしません)",PE_WARN);
					break;
				}
				if(dtimetemp<0)
					PrintError("TR 命令で負の数は指定できません",PE_ERROR);
				// TIME 命令を書き込む //
				SclTime += dtimetemp;
				WriteData1(&cvt,(BYTE)SCL_TIME);
				WriteData4(&cvt,(DWORD)SclTime);
				//printf("time = %d\n",SclTime);
			break;

			case(CMD_SSP):
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("スクロールスピードの指定が無いよ",PE_ERROR);
				WriteData1(&cvt,(BYTE)SCL_SSP);
				WriteData2(&cvt,(WORD)Calc(cvt.paword));
			break;

			case(CMD_EFC):	// エフェクト命令(一部Level2)
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("エフェクト番号の指定が無いよ",PE_ERROR);
				WriteData1(&cvt,(BYTE)SCL_EFC);
				WriteData1(&cvt,(BYTE)Calc(cvt.paword));
			break;

			case(CMD_NAME):
				if(cvt.out!=NULL)
					PrintError("現ヴァージョンでは出力ファイルの変更はできないの",PE_ERROR);

				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("名前の指定が無いよ",PE_ERROR);

				strcpy(buffer,path);strcat(buffer,cvt.paword);
				cvt.out = fopen(buffer,"wb");
				if(cvt.out==NULL)
					PrintError("ファイルに書き込めないの",PE_ERROR);

				sprintf(buffer,">出力ファイル    : %s",cvt.paword);
				PrintError(buffer,PE_STRING);

				// 開始時間：０をセットする //
				WriteData1(&cvt,(BYTE)SCL_TIME);
				WriteData4(&cvt,(DWORD)0);
				//printf("time = %d\n",0);
			break;

			case(CMD_BOSS):			// ボス発生(仕様変更有り X,Y,ID)
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("ボスＸ座標の指定が無いよ",PE_ERROR);
				x = Calc(cvt.paword);

				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("BossX の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("ボスＹ座標の指定が無いよ",PE_ERROR);
				y = Calc(cvt.paword);

				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("BossY の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("ボス番号の指定が無いよ",PE_ERROR);
				n = Calc(cvt.paword);

				WriteData1(&cvt,(BYTE)SCL_BOSS);
				WriteData2(&cvt,(short)x);
				WriteData2(&cvt,(short)y);
				WriteData1(&cvt,(BYTE)n);
				printf("T[%5d]:Boss \t",SclTime);
			break;

			case(CMD_BOSSDEAD):
				WriteData1(&cvt,(BYTE)SCL_BOSSDEAD);
			break;

			case(CMD_WAITEX):		// 特殊待ち(引数：<条件定数(BYTE)>,<オプション(DWORD)>)
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("待ち条件の指定が無いよ",PE_ERROR);
				x = Calc(cvt.paword);
				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("待ち条件の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("オプションの指定が無いよ",PE_ERROR);
				y = Calc(cvt.paword);

				WriteData1(&cvt,(BYTE)SCL_WAITEX);
				WriteData1(&cvt,(BYTE)x);
				WriteData4(&cvt,(DWORD)y);
			break;

			case(CMD_STAGECLEAR):	// ステージクリア(引数：無し)
				WriteData1(&cvt,(BYTE)SCL_STAGECLEAR);
			break;

			case(CMD_MWOPEN):		// メッセージウィンドウ・オープン(Level_2)
				WriteData1(&cvt,(BYTE)SCL_MWOPEN);
			break;

			case(CMD_MWCLOSE):		// メッセージウィンドウ・クローズ(Level_2)
				WriteData1(&cvt,(BYTE)SCL_MWCLOSE);
			break;

			case(CMD_MSG):			// メッセージ送信(Level_2)
				WriteData1(&cvt,(BYTE)SCL_MSG);
				for(p=GetNextWordEx(&cvt);(*p)!='\0';p++)
					WriteData1(&cvt,(BYTE)*p);
				WriteData1(&cvt,(BYTE)'\0');
				printf("\nT[%5d]:MSG\t\"%s\"\n",SclTime,cvt.paword);
				char logtemp[200];
				sprintf(logtemp,"[%d]MSG",SclTime);
				PrintError(logtemp,PE_STRING);
			break;

			case(CMD_FACE):			// 顔グラ表示(Level_2)
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("顔番号の指定が無いよ",PE_ERROR);
				WriteData1(&cvt,(BYTE)SCL_FACE);
				WriteData1(&cvt,(BYTE)Calc(cvt.paword));
				printf("\nT[%5d]:顔G\t\"%s\"\n",SclTime,cvt.paword);
			break;

			case(CMD_LOADFACE):		// 顔グラロード(Level_2)
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("SurfaceID の指定が無いよ",PE_ERROR);
				x = Calc(cvt.paword);
				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("SurfaceID の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("ファイル番号の指定が無いよ",PE_ERROR);
				y = Calc(cvt.paword);

				WriteData1(&cvt,(BYTE)SCL_LOADFACE);
				WriteData1(&cvt,(BYTE)x);
				WriteData1(&cvt,(BYTE)y);
			break;

			case(CMD_MUSIC):	// 曲の変更＆再生
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("曲番号の指定が無いよ",PE_ERROR);
				WriteData1(&cvt,(BYTE)SCL_MUSIC);
				WriteData1(&cvt,(BYTE)Calc(cvt.paword));
			break;

			case(CMD_KEY):			// キーボード待ち(Level_2)
				WriteData1(&cvt,(BYTE)SCL_KEY);
			break;

			case(CMD_NPG):			// 新しいページへ(Level_2)
				WriteData1(&cvt,(BYTE)SCL_NPG);
			break;

			default:	// 敵の座標指定
				x = Calc(cvt.paword);
				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("EnemyX の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("Ｙ座標の指定がないよ",PE_ERROR);
				y = Calc(cvt.paword);

				// 敵の種類を省略できると便利かも... //
				if(GetNextWord(&cvt)!=CONV_COMMA)
					PrintError("EnemyY の後のコンマがないよ",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("敵の種類の指定がないよ",PE_ERROR);
				n = Calc(cvt.paword);

				// 敵の座標を出力する //
				WriteData1(&cvt,(BYTE)SCL_ENEMY);
				WriteData2(&cvt,(short)x);
				WriteData2(&cvt,(short)y);
				WriteData1(&cvt,(BYTE)n);
				if(EnemyTime!=SclTime){
					printf("T[%5d]:Enemy\t",SclTime);
					EnemyTime = SclTime;
				}
			break;
		}
	}

	EndConverter(&cvt);
	puts("\n何かキーを押すと終了するよ");getch();
}

void SetupConverter(CONVERTER *cvt)
{
	cvt->szline = 1000;
	cvt->paline = (char *)LocalAlloc(LPTR,cvt->szline);
	cvt->paword = (char *)LocalAlloc(LPTR,cvt->szline);
	cvt->paptr  = cvt->paline;

	if(cvt->paline==NULL || cvt->paword==NULL)
		PrintError("コンバータ生成用のメモリが足りないよ",PE_ERROR);

	cvt->lconst   = cvt->gconst   = NULL;

	cvt->paline[0] = '\0';
	cvt->paword[0] = '\0';

	cvt->line  = 0;
	cvt->block = 0;
	cvt->addr  = 0;
	//cvt->kind  = STD_KIND;

	cvt->saddr = NULL;
	cvt->out   = NULL;
	cvt->in    = NULL;
	cvt->log   = NULL;

	// コンバータの変数監視用ポインタ等をセット //
	pConvertLine    = &cvt->line;
	ppLogFile       = &cvt->log;

	// ログファイル有効化 //
#ifdef SCL_LOGFILENAME
	cvt->log = fopen(SCL_LOGFILENAME,"w");
#endif

	CalcSetup(CalcError);

	// 座標用グローバル定数 //
	SetConst(&cvt->gconst,"X_MIN",ItoS(X_MIN));
	SetConst(&cvt->gconst,"X_MID",ItoS(X_MID));
	SetConst(&cvt->gconst,"X_MAX",ItoS(X_MAX));
	SetConst(&cvt->gconst,"X_RND",ItoS(X_RNDV));

	SetConst(&cvt->gconst,"Y_MIN",ItoS(Y_MIN));
	SetConst(&cvt->gconst,"Y_MID",ItoS(Y_MID));
	SetConst(&cvt->gconst,"Y_MAX",ItoS(Y_MAX));
	SetConst(&cvt->gconst,"Y_RND",ItoS(Y_RNDV));

	// エフェクト用定数 //
	SetConst(&cvt->gconst,"WARN"		,ItoS(SEFC_WARN));
	SetConst(&cvt->gconst,"WARNSTOP"	,ItoS(SEFC_WARNSTOP));
	SetConst(&cvt->gconst,"MUSICFADE"	,ItoS(SEFC_MUSICFADE));
	SetConst(&cvt->gconst,"STG2BOSS"	,ItoS(SEFC_STG2BOSS));
	SetConst(&cvt->gconst,"RASTERON"	,ItoS(SEFC_RASTERON));
	SetConst(&cvt->gconst,"RASTEROFF"	,ItoS(SEFC_RASTEROFF));
	SetConst(&cvt->gconst,"CFADEIN"		,ItoS(SEFC_CFADEIN));
	SetConst(&cvt->gconst,"CFADEOUT"	,ItoS(SEFC_CFADEOUT));

	// WAITEX 用定数 //
	SetConst(&cvt->gconst,"BOSSLEFT",ItoS(SWAIT_BOSSLEFT));
	SetConst(&cvt->gconst,"BOSSHP"	,ItoS(SWAIT_BOSSHP));
}

void EndConverter(CONVERTER *cvt)
{
	// End of SCL を出力する //
	WriteData1(cvt,(BYTE)SCL_END);

	ReleaseConst(cvt->lconst);
	ReleaseConst(cvt->gconst);

	LocalFree(cvt->paline);
	LocalFree(cvt->paword);
	LocalFree(cvt->saddr);

	if(cvt->in ) fclose(cvt->in);
	if(cvt->out) fclose(cvt->out);
	if(cvt->log) fclose(cvt->log);
}

DWORD GetNextWord(CONVERTER *cvt)
{
	int		i;
	BOOL	blabel   = FALSE;
	BOOL	bComment = FALSE;

	// スペーススキップ //
	while(isspace(*(cvt->paptr))) cvt->paptr++;

	if(*(cvt->paptr)=='{') bComment = TRUE;

	// 必要に応じてファイルから読み込み //
	while(*(cvt->paptr)=='\0' || *(cvt->paptr)=='\n' || *(cvt->paptr)==';' || bComment){

		// 範囲コメントの処理 //
		while(*(cvt->paptr)=='{' || bComment==TRUE){
			bComment = TRUE;
			while(!(*(cvt->paptr)=='}' || *(cvt->paptr)=='\0')) cvt->paptr++;
			if(*(cvt->paptr)=='}'){
				bComment = FALSE;
				cvt->paptr++;
				while(isspace(*(cvt->paptr))) cvt->paptr++;
			}
			else break;
		}

		if(fgets(cvt->paline,cvt->szline,cvt->in)==NULL) return CONV_EOF;
		cvt->line++;

		// 定数変換処理 //
		ConvertConst(cvt->lconst,cvt->paline);
		ConvertConst(cvt->gconst,cvt->paline);

		cvt->paptr = cvt->paline;

		if(*(cvt->paptr)=='{') bComment = TRUE;

		while(isspace(*(cvt->paptr))) cvt->paptr++;

		if(*(cvt->paptr)=='{') bComment = TRUE;
	}

	// コンマの場合は特別扱い //
	if(*(cvt->paptr)==','){
		cvt->paword[0] = ',' ;
		cvt->paword[1] = '\0';
		cvt->paptr++;
		return CONV_COMMA;
	}

	// paword[] をセットするよ //
	for(i=0;!IsPBGCode(*(cvt->paptr));i++){
		cvt->paword[i] = *cvt->paptr;
		cvt->paptr++;

		// もし、ラベルならばコロンは付けずに出力 //
		if(*(cvt->paptr-1)==':'){
			blabel = TRUE;
			if(!i) cvt->paword[i++]=':';
			break;
		}
	}
	cvt->paword[i] = '\0';

	// paword[] が何者かで返す値を決定する //
	switch(cvt->paword[0]){
		case('#'):
			if(!blabel) return CONV_EXCMD;
		break;

		case('@'):
			if(blabel){
				strcpy(cvt->paword,cvt->paword+1);		// @ を取り除く
				return CONV_GLABEL;
			}
			else return CONV_NORMAL;					// JMP 系命令の引数の場合
		break;

		case(':'): break;

		default:
			if(blabel) return CONV_LABEL;
			else       return CONV_NORMAL;
		break;
	}

	// エラー出力＆Ｃコンパイラ対策のダミー値Return //
	PrintError("変なラベルがあるよ",PE_ERROR);
	return CONV_EOF;
}

char *GetNextWordEx(CONVERTER *cvt)
{
	int		i;

	// スペーススキップ //
	while(isspace(*(cvt->paptr))) cvt->paptr++;

	// 必要に応じてファイルから読み込み //
	while(*(cvt->paptr)=='\0' || *(cvt->paptr)=='\n' || *(cvt->paptr)==';'){
		if(fgets(cvt->paline,cvt->szline,cvt->in)==NULL)
			PrintError("文字列の指定がないよぉ",PE_ERROR);
		cvt->line++;

		// 定数変換処理 //
		//ConvertConst(cvt->lconst,cvt->paline);
		//ConvertConst(cvt->gconst,cvt->paline);

		cvt->paptr = cvt->paline;
		while(isspace(*(cvt->paptr))) cvt->paptr++;
	}

	if(*(cvt->paptr)!='\"') PrintError("文字列指定は \" でくくって",PE_ERROR);

	cvt->paptr++;
	for(i=0;*(cvt->paptr)!='\"';i++){
		if(*(cvt->paptr)=='\0' || *(cvt->paptr)=='\n' || *(cvt->paptr)==';')
			PrintError("改行する前に \" でくくってほしい",PE_ERROR);
		cvt->paword[i] = *(cvt->paptr);
		cvt->paptr++;
	}
	cvt->paptr++;
	cvt->paword[i] = '\0';

	return cvt->paword;
}

DWORD GetCmd(char *cmd)
{
	DWORD i;

	for(i=0;i<CMD_MAX;i++)
		if(strcmp(cmd,Cmd[i])==0) break;

	return i;
}

void ConvertConst(CONST_ST *cst,char *s)
{
	CONST_ST	*chk;
	char		temp[1000],*p;
	int			len;

	for(chk=cst;chk!=NULL;chk=chk->next){
		len = strlen(chk->name);
		for(p=s;(*p)!='\0';p++){
			if(strncmp(p,chk->name,len)==0 && (!(isalnum(p[len])||p[len]=='_'))){
				strcpy(temp,p+len);
				strcpy(p,chk->value);
				p += strlen(chk->value);
				strcpy(p,temp);
			}
		}
	}
}

void SetConst(PCONST_ST *cst,char *name,char *value)
{
	CONST_ST	*newConst;
	char		buffer[1000];

	if(!IsName(name))
		PrintError("定義済みの定数、または定数名に使用出来ない文字があります",PE_ERROR);

	newConst        = (PCONST_ST)LocalAlloc(LPTR,sizeof(CONST_ST));
	newConst->name  = (char *)   LocalAlloc(LPTR,strlen(name )+1);
	newConst->value = (char *)   LocalAlloc(LPTR,strlen(value)+1);

	if(newConst==NULL || newConst->name==NULL || newConst->value==NULL)
		PrintError("定数用のメモリが足りないよ",PE_ERROR);

	strcpy(newConst->name ,name);
	strcpy(newConst->value,value);
	newConst->next  = *cst;
	*cst            = newConst;

	if(PrintConst){
		sprintf(buffer,"[ 定数 ] %5d行 : %s = %s が定義された",*pConvertLine,name,value);
		PrintError(buffer,PE_STRING);
	}
}

void ReleaseConst(CONST_ST *cst)
{
	if(cst){
		ReleaseConst(cst->next);
		LocalFree(cst->name);
		LocalFree(cst->value);
		LocalFree(cst);
	}
}

void WriteData1(CONVERTER *cvt,BYTE data)
{
	if(cvt->out==NULL)
		PrintError("NAME で出力ファイルが指定されていないよ",PE_ERROR);

	fwrite(&data,sizeof(BYTE),1,cvt->out);
	cvt->addr += sizeof(BYTE);
}

void WriteData2(CONVERTER *cvt,WORD data)
{
	if(cvt->out==NULL)
		PrintError("NAME で出力ファイルが指定されていないよ",PE_ERROR);

	fwrite(&data,sizeof(WORD),1,cvt->out);
	cvt->addr += sizeof(WORD);
}

void WriteData4(CONVERTER *cvt,DWORD data)
{
	if(cvt->out==NULL)
		PrintError("NAME で出力ファイルが指定されていないよ",PE_ERROR);

	fwrite(&data,sizeof(DWORD),1,cvt->out);
	cvt->addr += sizeof(DWORD);
}

char *ItoS(int val)
{
	static char buf[1000];

	sprintf(buf,"%d",val);
	return buf;
}

BOOL IsName(char *s)
{
	// 先頭の一文字がアルファベットでなければ //
	if(!isalpha(*s)) return FALSE;

	for(;(*s)!='\0';s++){
		// アルファベットか数字でなければ //
		if(!(isalnum(*s)||((*s)=='_'))) return FALSE;
	}

	return TRUE;
}

void CalcError(char *s)
{
	PrintError(s,PE_ERROR);
}

void PrintError(char *s,DWORD flags)
{
	char	buffer[1000],sline[10];
	DWORD	line;

	// 行数セット処理 //
	line = (pConvertLine) ? (*pConvertLine) : 0;
	if(line) sprintf(sline,"%5d",line);
	else     sprintf(sline,"xxxxx");

	switch(flags){
		case(PE_ERROR):
			sprintf(buffer,"\n[エラー] %s行 : %s",sline,s);
			puts(buffer);strcat(buffer,"\n");
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
			puts("\n何かキーを押すと終了するよ");getch();
			exit(-1);
		break;

		case(PE_WARN):
			sprintf(buffer,"\n[ 警告 ] %s行 : %s",sline,s);
			puts(buffer);strcat(buffer,"\n");
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
		break;

		case(PE_STRING):
			puts(s);sprintf(buffer,"%s\n",s);
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
		break;
	}
}

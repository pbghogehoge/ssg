#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#include "CALC.H"
#include "ECLCDEF.H"
#include "..\\Gian07SrcFiles\\GIAN.H"

/*                                                                                               *
 * [座標指定上の注意]                                                                            *
 *  @座標はグラフィック座標で指定する(x64 座標ではなくなった)                                    *
 *                                                                                               *
 * [ＥＣＬＣのすごくなった点]                                                                    *
 *  @定数が使用できるようになった                                                                *
 *  @メモリの許す限り定数、ラベルを作成できる                                                    *
 *  @定数、ラベルにはローカルとグローバルの２種類が存在する                                      *
 *  @エラーチェックなどの解析部が読みやすくなった                                                *
 *  @１行に１命令以上記述することができる                                                        *
 *                                                                                               *
 * [名前について]                                                                                *
 *  @先頭の文字はアルファベットである                                                            *
 *  @ほかの文字は、0,1,2,..,9,_ もしくはアルファベットである                                     *
 *                                                                                               *
 * [ＪＭＰ系命令について]                                                                        *
 *  @グローバルラベルを参照するときはラベル名の前に必ず＠をつけること                            *
 *  @従って、＠のないものはローカルラベルとして扱われることに注意                                *
 *                                                                                               *
 * [定数置換(#DEFINE)について]                                                                   *
 *  @置換すべき文字列と一致しても、次の１文字がアルファベットか数字の場合は置換されない          *
 *  (ex) #DEFINE BAKA 92 としたとき BAKA3 という文字列は置換されない                             *
 *                                                                                               *
 * [コンバータ制御用命令について]                                                                *
 *  #ENEMY  - #END  : 敵ブロックを生成します                                                     *
 *  #NAME           : 出力ファイル名を指定します。これが無いと、エラーになります                 *
 *  #LOG            : ログファイル名を指定します。この宣言の後から有効になります                 *
 *  #DEFINE         : 定数を定義します。なお、ブロック内にあった場合はローカルになります         *
 *  #PRAGMA         : 現時点ではこの命令は使用できません                                         *
 *  #KIND           : 敵の種類を指定します。指定が無かった場合、敵の種類は STD_KIND になります   *
 *                                                                                               *
 * [ＥＣＬ命令について]                                                                          *
 *  @ECL.H 及び ECLCDEF.H を参照してください                                                     *
 *                                                                                               */


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

#define STD_KIND		20			// デフォルトの敵の種類

#define EXCMD_MAX		7			// EXCMD の数
#define EXCMD_ENEMY		0x00		// #ENEMY
#define EXCMD_END		0x01		// #END
#define EXCMD_NAME		0x02		// #NAME
#define EXCMD_LOG		0x03		// #LOG
#define EXCMD_DEFINE	0x04		// #DEFINE
#define EXCMD_PRAGMA	0x05		// #PRAGMA
#define EXCMD_KIND		0x06		// #KIND


const char *ExCmd[EXCMD_MAX] =
	{"#ENEMY","#END","#NAME","#LOG","#DEFINE","#PRAGMA","#KIND"};


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
void  UpdateCmdLenH(char *name);								// コマンド長ヘッダの更新

void  ConvertConst(CONST_ST *cst,char *s);						// 定数を置換する
void  SetConst(PCONST_ST *cst,char *name,char *value);			// 定数をセットする
void  SetLabel(PLABEL_ST *lbl,char *name,long addr);			// ラベルをセットする
void  SetLabelT(PLABEL_ST *lbl,char *name,long addr);			// ラベル(TEMP)をセットする
void  ReleaseConst(CONST_ST *cst);								// 定数用のメモリを解放する
void  ReleaseLabel(LABEL_ST *lbl);								// ラベル用のメモリを解放する

DWORD GetNextWord(CONVERTER *cvt);								// 次の単語に移動する
DWORD GetExCmd(char *cmd);										// EXCMD定数を得る
long  GetAddress(LABEL_ST *lbl,char *name);						// name に対するアドレスを調べる

void  ParseDefine(CONVERTER *cvt,PCONST_ST *cst);				// #DEFINE  を解析する
void  ParsePragma(CONVERTER *cvt);								// #PRAGMA  を解析する
void  ParseEnemy( CONVERTER *cvt);								// #ENEMY   を解析する
void  ParseECL(   CONVERTER *cvt);								// ECL１命令を解析する

void  WriteLabelT(CONVERTER *cvt,LABEL_ST *lbl,LABEL_ST *tmp);	// ラベルの後書き
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
	DWORD		temp;
	char		buffer[1000];
	char		*p,path[1000]="";

	puts("");
	puts("");
	puts("[内蔵関数のバージョン情報]");
	puts(" >> ECL_Converter  : Version 0.04 <<");
	printf(" >> %s <<\n",PBGWIN_ECL_H);
	puts("\n[更新履歴++]");
	puts(" >> 注釈に {...} を追加 <<");
	puts(" >> 単項演算子 '-' を追加 <<\n\n\n");

	SetupConverter(&cvt);

	if(argc!=2)
		PrintError("ECLC <FILENAME>",PE_ERROR);

	cvt.in = fopen(argv[1],"r");
	if(cvt.in==NULL)
		PrintError("ファイルがみつからないよ",PE_ERROR);

	if(strchr(argv[1],'\\')!=NULL){
		strcpy(path,argv[1]);
		for(p=path+strlen(path);*(p-1)!='\\';p--);
		*p = '\0';
	}

	// ECL_LEN.H を更新する(必要に応じて消すのだ) //
	//UpdateCmdLenH("D:\\PROJECT\\GIAN_MAIN\\ECL_LEN.H");
	UpdateCmdLenH("D:\\Project\\Gian07SrcFiles\\ECL_LEN.H");

	while((temp=GetNextWord(&cvt))!=CONV_EOF){
		if(temp!=CONV_EXCMD)
			PrintError("ここはブロック内じゃないよ",PE_ERROR);

		switch(GetExCmd(cvt.paword)){
			case(EXCMD_DEFINE): ParseDefine(&cvt,&cvt.gconst);	break;
			case(EXCMD_PRAGMA): ParsePragma(&cvt);				break;
			case(EXCMD_ENEMY) : ParseEnemy(&cvt);				break;

			case(EXCMD_NAME):
				if(cvt.out!=NULL)
					PrintError("出力ファイルの変更はできない",PE_ERROR);

				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("#NAME の使い方がおかしいよ",PE_ERROR);

				strcpy(buffer,path);strcat(buffer,cvt.paword);
				cvt.out = fopen(buffer,"wb");
				if(cvt.out==NULL)
					PrintError("ファイルに書き込めない",PE_ERROR);

				sprintf(buffer,">出力ファイル    : %s",cvt.paword);
				PrintError(buffer,PE_STRING);
			break;

			case(EXCMD_LOG):
				if(cvt.log!=NULL)
					PrintError("現ヴァージョンではログファイルの変更はできない",PE_ERROR);

				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("#LOG の使い方がおかしいよ",PE_ERROR);

				strcpy(buffer,path);strcat(buffer,cvt.paword);
				cvt.log = fopen(buffer,"wb");
				if(cvt.log==NULL)
					PrintError("ファイルに書き込めない",PE_ERROR);

				sprintf(buffer,">ログファイル    : %s",cvt.paword);
				PrintError(buffer,PE_STRING);
			break;

			case(EXCMD_KIND):
				if(cvt.saddr!=NULL)
					PrintError("#ENEMY ブロックの後で #KIND の変更は行えない",PE_ERROR);
				if(GetNextWord(&cvt)!=CONV_NORMAL)
					PrintError("#KIND の使い方がおかしいよ",PE_ERROR);
				cvt.kind = Calc(cvt.paword);
				if(cvt.kind<=0)
					PrintError("#KIND の引数には１以上を指定してほしい",PE_ERROR);
			break;

			case(EXCMD_END):
				PrintError("#END に対応する制御用コマンドがないよ",PE_ERROR);
			break;

			default:
				PrintError("定義されていない制御用コマンドです",PE_ERROR);
			break;
		}
	}

	// グローバルラベルの後書き処理を行う //
	WriteLabelT(&cvt,cvt.glabel,cvt.glabel_t);

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
	cvt->llabel   = cvt->glabel   = NULL;
	cvt->llabel_t = cvt->glabel_t = NULL;

	cvt->paline[0] = '\0';
	cvt->paword[0] = '\0';

	cvt->line  = 0;
	cvt->block = 0;
	cvt->addr  = 0;
	cvt->kind  = STD_KIND;

	cvt->saddr = NULL;
	cvt->out   = NULL;
	cvt->in    = NULL;
	cvt->log   = NULL;

	// コンバータの変数監視用ポインタ等をセット //
	pConvertLine    = &cvt->line;
	ppLogFile       = &cvt->log;

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

	// 弾の種類用グローバル定数 //
	SetConst(&cvt->gconst,"T_NORM"    ,ItoS(T_NORM));
	SetConst(&cvt->gconst,"T_NORM_A"  ,ItoS(T_NORM_A));
	SetConst(&cvt->gconst,"T_HOMING"  ,ItoS(T_HOMING));
	SetConst(&cvt->gconst,"T_HOMING_M",ItoS(T_HOMING_M));
	SetConst(&cvt->gconst,"T_ROLL"    ,ItoS(T_ROLL));
	SetConst(&cvt->gconst,"T_ROLL_A"  ,ItoS(T_ROLL_A));
	SetConst(&cvt->gconst,"T_ROLL_R"  ,ItoS(T_ROLL_R));
	SetConst(&cvt->gconst,"T_GRAVITY" ,ItoS(T_GRAVITY));
	SetConst(&cvt->gconst,"T_CHANGE"  ,ItoS(T_CHANGE));

	// 弾オプション用グローバル定数 //
	SetConst(&cvt->gconst,"TOP_NONE" ,ItoS(TOP_NONE));
	SetConst(&cvt->gconst,"TOP_WAVE" ,ItoS(TOP_WAVE));
	SetConst(&cvt->gconst,"TOP_ROLL" ,ItoS(TOP_ROLL));
	SetConst(&cvt->gconst,"TOP_PURU" ,ItoS(TOP_PURU));
	SetConst(&cvt->gconst,"TOP_REFX" ,ItoS(TOP_REFX));
	SetConst(&cvt->gconst,"TOP_REFY" ,ItoS(TOP_REFY));
	SetConst(&cvt->gconst,"TOP_REFXY",ItoS(TOP_REFXY));
	SetConst(&cvt->gconst,"TOP_DIV"  ,ItoS(TOP_DIV));
	SetConst(&cvt->gconst,"TOP_BOMB" ,ItoS(TOP_BOMB));

	// 弾発射コマンド用グローバル定数 //
	SetConst(&cvt->gconst,"TC_WAY"   ,ItoS(TC_WAY));
	SetConst(&cvt->gconst,"TC_ALL"   ,ItoS(TC_ALL));
	SetConst(&cvt->gconst,"TC_RND"   ,ItoS(TC_RND));
	SetConst(&cvt->gconst,"TC_WAYS"  ,ItoS(TC_WAYS));
	SetConst(&cvt->gconst,"TC_ALLS"  ,ItoS(TC_ALLS));
	SetConst(&cvt->gconst,"TC_RNDS"  ,ItoS(TC_RNDS));
	SetConst(&cvt->gconst,"TC_WAYZ"  ,ItoS(TC_WAYZ));
	SetConst(&cvt->gconst,"TC_ALLZ"  ,ItoS(TC_ALLZ));
	SetConst(&cvt->gconst,"TC_RNDZ"  ,ItoS(TC_RNDZ));
	SetConst(&cvt->gconst,"TC_WAYSZ" ,ItoS(TC_WAYSZ));
	SetConst(&cvt->gconst,"TC_ALLSZ" ,ItoS(TC_ALLSZ));
	SetConst(&cvt->gconst,"TC_RNDSZ" ,ItoS(TC_RNDSZ));

	// レーザーの種類 //
	SetConst(&cvt->gconst,"LS_SHORT" ,ItoS(LS_SHORT));
	SetConst(&cvt->gconst,"LS_REF"   ,ItoS(LS_REF));

	// 太レーザーの種類 //
	SetConst(&cvt->gconst,"LLS_LONG"  ,ItoS(LLS_LONG));
	SetConst(&cvt->gconst,"LLS_SETDEG",ItoS(LLS_SETDEG));

	// レーザーの発射方向 //
	SetConst(&cvt->gconst,"LC_WAY" ,ItoS(LC_WAY));
	SetConst(&cvt->gconst,"LC_ALL" ,ItoS(LC_ALL));
	SetConst(&cvt->gconst,"LC_RND" ,ItoS(LC_RND));
	SetConst(&cvt->gconst,"LC_WAYZ",ItoS(LC_WAYZ));
	SetConst(&cvt->gconst,"LC_ALLZ",ItoS(LC_ALLZ));
	SetConst(&cvt->gconst,"LC_RNDZ",ItoS(LC_RNDZ));

	// レーザーコマンド構造体参照用定数 //
	SetConst(&cvt->gconst,"LCMD_D",ItoS(ECLCST_LCMD_D));
	SetConst(&cvt->gconst,"LCMD_DW",ItoS(ECLCST_LCMD_DW));
	SetConst(&cvt->gconst,"LCMD_N",ItoS(ECLCST_LCMD_N));
	SetConst(&cvt->gconst,"LCMD_C",ItoS(ECLCST_LCMD_C));
	SetConst(&cvt->gconst,"LCMD_L",ItoS(ECLCST_LCMD_L));
	SetConst(&cvt->gconst,"LCMD_V",ItoS(ECLCST_LCMD_V));

	// 弾コマンド構造体参照用定数 //
	SetConst(&cvt->gconst,"TCMD_D"  ,ItoS(ECLCST_TCMD_D));
	SetConst(&cvt->gconst,"TCMD_DW" ,ItoS(ECLCST_TCMD_DW));
	SetConst(&cvt->gconst,"TCMD_N"  ,ItoS(ECLCST_TCMD_N));
	SetConst(&cvt->gconst,"TCMD_NS" ,ItoS(ECLCST_TCMD_NS));
	SetConst(&cvt->gconst,"TCMD_V"  ,ItoS(ECLCST_TCMD_V));
	SetConst(&cvt->gconst,"TCMD_C"  ,ItoS(ECLCST_TCMD_C));
	SetConst(&cvt->gconst,"TCMD_A"  ,ItoS(ECLCST_TCMD_A));
	SetConst(&cvt->gconst,"TCMD_REP",ItoS(ECLCST_TCMD_REP));
	SetConst(&cvt->gconst,"TCMD_VD" ,ItoS(ECLCST_TCMD_VD));

	// 敵データ構造体参照用定数 //
	SetConst(&cvt->gconst,"ENEMY_X" ,ItoS(ECLCST_ENEMY_X));
	SetConst(&cvt->gconst,"ENEMY_Y" ,ItoS(ECLCST_ENEMY_Y));
	SetConst(&cvt->gconst,"ENEMY_D" ,ItoS(ECLCST_ENEMY_D));

	// 割り込みベクタ指定用定数 //
	SetConst(&cvt->gconst,"VECT_BOSSLEFT",ItoS(ECLVECT_BOSSLEFT));
	SetConst(&cvt->gconst,"VECT_HP",ItoS(ECLVECT_HP));

	// 太レーザー用定数 //
	SetConst(&cvt->gconst,"LLASERALL",ItoS(ECLCST_LLASERALL));

	// INT 用定数 //
	SetConst(&cvt->gconst,"SNAKEON",ItoS(ECLINT_SNAKEON));
}

void EndConverter(CONVERTER *cvt)
{
	fseek(cvt->out,sizeof(DWORD),SEEK_SET);
	fwrite(cvt->saddr,sizeof(DWORD),cvt->block,cvt->out);

	ReleaseConst(cvt->lconst);
	ReleaseConst(cvt->gconst);
	ReleaseLabel(cvt->llabel);
	ReleaseLabel(cvt->llabel_t);
	ReleaseLabel(cvt->glabel);
	ReleaseLabel(cvt->glabel_t);

	LocalFree(cvt->paline);
	LocalFree(cvt->paword);
	LocalFree(cvt->saddr);

	if(cvt->in ) fclose(cvt->in);
	if(cvt->out) fclose(cvt->out);
	if(cvt->log) fclose(cvt->log);
}

void UpdateCmdLenH(char *name)
{
	FILE		*fp;
	char		buf[1000];
	int			i,j;
	int			size;
	ECLC_CMD	*cmd;

	if((fp=fopen(name,"w"))==NULL){
		sprintf(buf,"%s の生成に失敗しました",name);
		PrintError(buf,PE_WARN);
		return;
	}

	fprintf(fp,"/*\n * %s  : 敵の命令長定義ファイル\n",name);
	_strdate(buf);	fprintf(fp," * 作成日時    %s  ",buf);
    _strtime(buf);	fprintf(fp,"%s\n */\n\n\n",buf);

	fprintf(fp,"const BYTE ECL_CmdLen[256] = {\n");
	for(i=j=0;i<ECL_CMDMAX;i++,j++){
		for(cmd=CMD_BUF[i].cmd,size=1;*cmd!=DT_END;cmd++){
			switch(*cmd){
				case(DT_U4):case(DT_S4):case(DT_JMP):
					size+=2;
				case(DT_U2):case(DT_S2):
					size+=1;
				case(DT_U1):case(DT_S1):
					size+=1;
			}
		}
		for(;j<CMD_BUF[i].cmd_val;j++)
			fprintf(fp,"\t%5d,\t\t// \n",0);

		fprintf(fp,"\t%5d,\t\t// %s\n",size,CMD_BUF[i].name);
		//fprintf(fp,"ECL_CmdLen[%#04x] = %5d;\n",CMD_BUF[i].cmd_val,size);
		//fprintf(fp,"#define LEN_%-15s%5d     // \n",CMD_BUF[i].name,size);
	}
	for(;j<255;j++)
		fprintf(fp,"\t%5d,\t\t// \n",0);
	fprintf(fp,"\t%5d \t\t// \n};\n",0);

	fclose(fp);
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

	PrintError("変なラベルがある",PE_ERROR);
	return CONV_EOF;
}

DWORD GetExCmd(char *cmd)
{
	DWORD i;

	for(i=0;i<EXCMD_MAX;i++)
		if(strcmp(cmd,ExCmd[i])==0) break;

	return i;
}

long GetAddress(LABEL_ST *lbl,char *name)
{
	for(;lbl!=NULL;lbl=lbl->next){
		if(strcmp(lbl->name,name)==0)
			return lbl->addr;
	}

	// ラベルが見つからない場合は－１を返す //
	return -1;
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
	*cst       = newConst;

	if(PrintConst){
		sprintf(buffer,"[ 定数 ] %5d行 : %s = %s が定義された",*pConvertLine,name,value);
		PrintError(buffer,PE_STRING);
	}
}

void SetLabel(PLABEL_ST *lbl,char *name,long addr)
{
	LABEL_ST *p;

	for(p=(*lbl);p!=NULL;p=p->next){
		if(strcmp(p->name,name)==0)
			PrintError("同名のラベルがすでに存在しているよ",PE_ERROR);
	}

	SetLabelT(lbl,name,addr);
}

void SetLabelT(PLABEL_ST *lbl,char *name,long addr)
{
	LABEL_ST *newConst;

	if(!IsName(name))
		PrintError("ラベル名に使用出来ない文字があります",PE_ERROR);

	newConst       = (PLABEL_ST)LocalAlloc(LPTR,sizeof(LABEL_ST));
	newConst->name = (char *)   LocalAlloc(LPTR,strlen(name)+1);

	if(newConst==NULL || newConst->name==NULL)
		PrintError("ラベル用のメモリが足りないよ",PE_ERROR);

	strcpy(newConst->name,name);
	newConst->addr = addr;
	newConst->next = *lbl;
	newConst->line = *pConvertLine;
	*lbl      = newConst;
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

void ReleaseLabel(LABEL_ST *lbl)
{
	if(lbl){
		ReleaseLabel(lbl->next);
		LocalFree(lbl->name);
		LocalFree(lbl);
	}
}

void ParseDefine(CONVERTER *cvt,PCONST_ST *cst)
{
	char buf1[1000],buf2[1000];

	if(GetNextWord(cvt)!=CONV_NORMAL)
		PrintError("#DEFINE の第１引数がおかしい",PE_ERROR);
	strcpy(buf1,cvt->paword);

	if(GetNextWord(cvt)!=CONV_NORMAL)
		PrintError("#DEFINE の第２引数がおかしい",PE_ERROR);
	strcpy(buf2,cvt->paword);

	SetConst(cst,buf1,buf2);
}

void ParsePragma(CONVERTER *cvt)
{
	//PrintError("#PRAGMA は現在サポートされていないの",PE_WARN);
	if(GetNextWord(cvt)!=CONV_NORMAL)
		PrintError("#PRAGMA の使い方がおかしい",PE_ERROR);

	if(strcmp(cvt->paword,"PRINTCONST")==0)
		PrintConst = TRUE;
}

void ParseEnemy(CONVERTER *cvt)
{
	DWORD	i;
	char	buffer[1000];

	if(cvt->block==0){
		cvt->saddr = (long *)LocalAlloc(LPTR,sizeof(DWORD)*cvt->kind);
		if(cvt->saddr==NULL)
			PrintError("開始アドレステーブル用のメモリが確保できない",PE_ERROR);

		WriteData4(cvt,cvt->kind);
		for(i=0;i<cvt->kind;i++){
			WriteData4(cvt, sizeof(DWORD)*(cvt->kind+1));
			cvt->saddr[i] = sizeof(DWORD)*(cvt->kind+1);
		}

		sprintf(buffer,">敵の種類        : %d",cvt->kind);
		PrintError(buffer,PE_STRING);
	}

	if(cvt->block>=cvt->kind)
		PrintError("敵の種類が最大数を超えちゃったよ",PE_ERROR);

	cvt->saddr[cvt->block++] = cvt->addr;

	while(1){
		switch(GetNextWord(cvt)){
			// 通常のＥＣＬコマンドを解析する //
			case(CONV_NORMAL): ParseECL(cvt);	break;

			// 不正な符号が現れた場合、エラーを出力する //
			case(CONV_EOF):   PrintError("予期せぬEOF が検出されたよ",PE_ERROR);	break;
			case(CONV_COMMA): PrintError("変なところにコンマがあるよ",PE_ERROR);	break;

			// グローバルラベル or ローカルラベルの実体を解析する //
			case(CONV_GLABEL): SetLabel(&cvt->glabel,cvt->paword,cvt->addr);	break;
			case(CONV_LABEL):  SetLabel(&cvt->llabel,cvt->paword,cvt->addr);	break;

			// 制御用命令を解析する //
			case(CONV_EXCMD):
				switch(GetExCmd(cvt->paword)){
					// ローカル定数をセットする or #PRAGMA の解析 //
					case(EXCMD_DEFINE): ParseDefine(cvt,&cvt->lconst);	break;
					case(EXCMD_PRAGMA): ParsePragma(cvt);				break;

					case(EXCMD_ENEMY):case(EXCMD_LOG):case(EXCMD_NAME):
						PrintError("このブロックでは使用できない制御用命令があります",PE_ERROR);
					break;

					// 敵ブロックの終了 //
					case(EXCMD_END):
						// ローカルラベルの後書き処理を行う //
						WriteLabelT(cvt,cvt->llabel,cvt->llabel_t);

						// ローカル定数、ローカルラベルの解放 //
						ReleaseConst(cvt->lconst);		cvt->lconst   = NULL;
						ReleaseLabel(cvt->llabel);		cvt->llabel   = NULL;
						ReleaseLabel(cvt->llabel_t);	cvt->llabel_t = NULL;
					return;

					default:
						PrintError("定義されていない制御用コマンドだよ",PE_ERROR);
					break;
				}
			break;
		}
	}
}

void ParseECL(CONVERTER *cvt)
{
	int		i,n;
	long	addr;

	// cvt->paword にはＥＣＬ命令が入っている //
	for(i=0;i<ECL_CMDMAX;i++)
		if(strcmp(cvt->paword,CMD_BUF[i].name)==0) break;

	if(i==ECL_CMDMAX)
		PrintError("定義されていないＥＣＬ命令です",PE_ERROR);

	// コマンドの値を出力する //
	WriteData1(cvt,CMD_BUF[i].cmd_val);

	// 引数の値を出力する //
	for(n=0;CMD_BUF[i].cmd[n]!=DT_END;n++){
		if(GetNextWord(cvt)!=CONV_NORMAL)
			PrintError("ＥＣＬ命令の引数がおかしいよ",PE_ERROR);

		switch(CMD_BUF[i].cmd[n]){
			// １バイトから４バイトのデータを出力する //
			case(DT_U1):case(DT_S1): WriteData1(cvt,(BYTE) Calc(cvt->paword));	break;
			case(DT_U2):case(DT_S2): WriteData2(cvt,(WORD) Calc(cvt->paword));	break;
			case(DT_U4):case(DT_S4): WriteData4(cvt,(DWORD)Calc(cvt->paword));	break;

			// ラベル(のアドレス)を出力する(まだ存在しない場合は自分のアドレスを出力) //
			case(DT_JMP):
				if(cvt->paword[0]=='@'){	// グローバル(＋１に注意)
					addr=GetAddress(cvt->glabel,cvt->paword+1);
					if(addr<0) SetLabelT(&cvt->glabel_t,cvt->paword+1,cvt->addr);
				}
				else{						// ローカル
					addr=GetAddress(cvt->llabel,cvt->paword);
					if(addr<0) SetLabelT(&cvt->llabel_t,cvt->paword,cvt->addr);
				}
				WriteData4(cvt,addr);		// ダミー値であったとしても書き込むのだ
			break;
		}

		if(CMD_BUF[i].cmd[n+1]!=DT_END && GetNextWord(cvt)!=CONV_COMMA)
			PrintError("あらあら、コンマが足りません",PE_ERROR);
	}
}

void WriteLabelT(CONVERTER *cvt,LABEL_ST *lbl,LABEL_ST *tmp)
{
	LABEL_ST	*p;
	long		file_addr;

	file_addr = cvt->addr;

	for(;tmp!=NULL;tmp=tmp->next){
		for(p=lbl;p!=NULL;p=p->next){
			if(strcmp(p->name,tmp->name)==0){
				fseek(cvt->out,tmp->addr,SEEK_SET);
				WriteData4(cvt,p->addr);
				break;
			}
		}
		if(p==NULL){
			cvt->line = tmp->line;
			PrintError("指定されたラベルは存在しません",PE_ERROR);
		}
	}

	cvt->addr = file_addr;
	fseek(cvt->out,file_addr,SEEK_SET);
}

void WriteData1(CONVERTER *cvt,BYTE data)
{
	if(cvt->out==NULL)
		PrintError("#NAME で出力ファイルが指定されていないよ",PE_ERROR);

	fwrite(&data,sizeof(BYTE),1,cvt->out);
	cvt->addr += sizeof(BYTE);
}

void WriteData2(CONVERTER *cvt,WORD data)
{
	if(cvt->out==NULL)
		PrintError("#NAME で出力ファイルが指定されていないよ",PE_ERROR);

	fwrite(&data,sizeof(WORD),1,cvt->out);
	cvt->addr += sizeof(WORD);
}

void WriteData4(CONVERTER *cvt,DWORD data)
{
	if(cvt->out==NULL)
		PrintError("#NAME で出力ファイルが指定されていないよ",PE_ERROR);

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
			sprintf(buffer,"[エラー] %s行 : %s",sline,s);
			puts(buffer);strcat(buffer,"\n");
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
			puts("\n何かキーを押すと終了するよ");getch();
			exit(-1);
		break;

		case(PE_WARN):
			sprintf(buffer,"[ 警告 ] %s行 : %s",sline,s);
			puts(buffer);strcat(buffer,"\n");
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
		break;

		case(PE_STRING):
			puts(s);sprintf(buffer,"%s\n",s);
			if(*ppLogFile!=NULL) fputs(buffer,*ppLogFile);
		break;
	}
}

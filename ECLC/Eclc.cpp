/*****************************************************************************/
/*   ECLC.C   敵の行動制御言語のデータコンバータ                             */
/*                                                                           */
/*****************************************************************************/

#define ECLC_VER "2.00"
#define TARGET   "Neo Gian"

#include <stdio.h>
#include <string.h>


#include "TYPE.H"
#include "CALC.H"
#include "PBG_FILE.H"

#include "ECL.H"
#include "ECLCDEF.C"


/*****定数********************************************************************/
#define LABEL_MAX		50			// ラベルの最大数

#define ST_NORM			0			// 改行でも終了でもない
#define ST_EOF			1			// 終了
#define ST_NEWL			2			// 改行
#define ST_LABEL		3			// ラベル
#define ST_CONVERT		4			// コンバータ命令
/*****************************************************************************/


/*****型**********************************************************************/
typedef struct{
	char name[20] ;		// ラベル名(もちろんコロンは無し！)
	BYTE addr     ;		// ラベルのアドレス(0xffはアドレスだけ未定義)
} LABEL;

typedef struct{
	long file_addr;		// ファイルにおけるアドレス？？？
	int  label_no ;		// 何番目のラベルか
	int  linetemp ;		// エラー出力用の行数
} ADTMP;
/*****************************************************************************/


/*****変数********************************************************************/
FILE  *READ_FP            ;		// 読み込み用ファイルポインタ
FILE  *WRITE_FP           ;		// 書き込み用ファイルポインタ

WORD  LINE         = 0    ;		// コンバート中の行
WORD  BLOCK        = 0    ;		// 全ブロック数
int   CONVERT_PTR  = 0    ;		// 書き込んだ容量(ブロックごとに初期化される)
BYTE  COMMAND             ;		// 現在解析中のコマンド(CMDBUF[?])

int   BLOCK_FLAG   = 0    ;		// 0:ブロック外  1:ブロック内
int   CONVERT_FLAG = 1    ;		// 1:コンバート中

int   LABEL_FLAG   = 0    ;		// 1:ラベルを許可
int   EOF_FLAG     = 0    ;		// 1:EOFを許可
int   NEWL_FLAG    = 1    ;		// 1:NEW_LINEを許可

char  INPUT[20]           ;		// 入力するファイル名
char  OUTPUT[20]          ;		// 出力するファイル名
char  STRING_BUF[260]=""  ;		// 現在解析中の文字列

LABEL LABEL_BUF[LABEL_MAX];		// ラベル格納用バッファ
int   LABEL_NOW  = 0      ;		// ブロック内に存在するラベルの数

ADTMP ADDR_BUF[LABEL_MAX] ;		// ラベル先読みバッファ
int   ADDR_NOW   = 0      ;		// 先読みした数
/*****************************************************************************/


/*****関数********************************************************************/
void name_check(void)       ;	// #NAME   を解析する
void start_check(void)      ;	// #START  を解析する
void end_check(void)        ;	// #END    を解析する
void command_set(void)	    ;	// COMMAND をセットする(#END解析を含む)
void parse_command(void)    ;	// COMMAND を解析する

int  get_string(void)       ;	// 次の文字列を得る

void label_set(char *name)  ;	// ラベルをセットする
void adtmp_set(int label_no);	// 先読みバッファをセットする
void addr_write(void)       ;	// 先読みバッファにあるアドレスを書き込む
BYTE label_line(char *name) ;	// 行数を得る(未定義なら先読みバッファにセット)

void print_error(char *s)   ;	// エラー文字列とその行数を表示して終了
void debug_print(void)      ;	// デバッグ情報を出力する

void fput_char(char data)   ;	// charデータを出力する & カウンタ++
void fput_BYTE(BYTE data)   ;	// BYTEデータを出力する & カウンタ++
void fput_int(int data)     ;	// int データを出力する & カウンタ+=2
void fput_WORD(WORD data)   ;	// WORDデータを出力する & カウンタ+=2
/*****************************************************************************/



int main(int argc,char *argv[])
{
	int temp;

	puts("");
	puts("");

	if(argc!=2){
		puts("[使用方法] ECLC <File name>");
		return -1;
	}

	strcpy(INPUT,Fil_namup(argv[1]));
	if(strcmp(INPUT,"/DEBUG")==0) debug_print();
	if(*Fil_getex(INPUT)=='\0') strcpy(INPUT,Fil_chgex(INPUT,"ECL"));

	if((READ_FP=fopen(INPUT,"r"))==NULL){
		printf("%s が見つからない\n",INPUT);
		return -1;
	}

	name_check();	// #NAME に関する処理

	// メインループ //
	while(CONVERT_FLAG==1){	// ここの条件ははずしちゃだめ! //
		while((temp=get_string())==ST_NEWL);
		if(temp==ST_EOF) break;

		if(BLOCK_FLAG==0) start_check();	// #START チェック
		else              command_set();	// #END チェック or COMMAND セット

		if(CONVERT_FLAG==1) parse_command();	// コマンド解析
	}

	printf("\n-----コンバート情報-----\n");
	printf("行       : %5u\n",LINE);
	printf("ブロック : %5u\n",BLOCK);
}

void name_check(void)
{
	while(get_string()==ST_NEWL);
	if(strcmp(STRING_BUF,"#NAME")!=0)
		print_error("#NAME が見つからない");
	if(get_string()!=ST_NORM)
		print_error("#NAME の引数が見つからない");
	sprintf(OUTPUT,"%3s_000.EDT",STRING_BUF);
}

void start_check(void)
{
	char buf[260];

	if(strcmp(STRING_BUF,"#END")==0)
		print_error("#END の場所がおかしい");
	if(strcmp(STRING_BUF,"#START")!=0){
		print_error("#STARTが見つからない");
	}

	BLOCK_FLAG=1;
	BLOCK++;
	COMMAND=COMMAND_START;	// SETUPとして...

	LABEL_FLAG = 1;		// 1:ラベルを許可
	EOF_FLAG   = 0;		// 1:EOFを許可
	NEWL_FLAG  = 0;		// 1:NEW_LINEを許可

	LABEL_NOW = ADDR_NOW = 0;
	CONVERT_PTR = 0;

	fclose(WRITE_FP);
	sprintf(OUTPUT+5,"%03d.EDT",BLOCK);
	if((WRITE_FP=fopen(OUTPUT,"wb"))==NULL){
		sprintf(buf,"ファイル %s が開けない",OUTPUT);
		print_error(buf);
	}
}

void end_check(void)
{
	char buf[260];

	if(strcmp(STRING_BUF,"#START")==0)
		print_error("#START の場所がおかしい");
	if(strcmp(STRING_BUF,"#END")!=0) return;

	LABEL_FLAG = 0;		// 1:ラベルを許可
	EOF_FLAG   = 1;		// 1:EOFを許可
	NEWL_FLAG  = 1;		// 1:NEW_LINEを許可

	addr_write();	// 先読みアドレスを書き込む
	BLOCK_FLAG  = 0;
	COMMAND=COMMAND_END;	// ENDとして...
}

void command_set(void)
{
	int i;
	char buf[260];

	end_check();
	if(COMMAND==COMMAND_END) return;

	for(i=0;i<ECLC_CMD_MAX;i++){
		if(strcmp(CMD_BUF[i].name,STRING_BUF)==0){
			COMMAND=i;	// 注意
			return;
		}
	}

	sprintf(buf,"謎の命令 %s がいる",STRING_BUF);
	print_error(buf);
}

void parse_command(void)
{
	int temp;
	char buf[260];
	ECLC_CMD *p = &CMD_BUF[COMMAND].cmd[0];

	fput_BYTE(CMD_BUF[COMMAND].cmd_val);

	NEWL_FLAG = 0;
	for(;*p!=DT_END;p++){
		get_string();
		switch(*p){
			case(DT_C):
				fput_char(calc(STRING_BUF));
			break;

			case(DT_8):
				fput_BYTE(calc(STRING_BUF));
			break;

			case(DT_I):
				fput_int(calc(STRING_BUF));
			break;

			case(DT_16):
				fput_WORD(calc(STRING_BUF));
			break;

			case(DT_J):
				fput_BYTE(label_line(STRING_BUF));
			break;
		}
	}
	NEWL_FLAG = 1;

	temp=get_string();
	if((temp!=ST_NEWL) && CONVERT_FLAG==1){
		sprintf(buf,"命令 %s の引数が多すぎる",CMD_BUF[COMMAND].name);
		print_error(buf);
	}
}

int get_string(void)
{
	static char file_buf[260]="\n";
	static char *ptr = file_buf;

	char s[260];
	int i;
	int c;


	HEAD: i=0;

	if((*ptr=='\n')||(*ptr==';')){
		if(NEWL_FLAG==0){
			sprintf(s,"命令 %s の引数が足りない",CMD_BUF[COMMAND].name);
			print_error(s);
		}
		if(fgets(file_buf,256,READ_FP)==NULL){
			if(EOF_FLAG==0)
				print_error("ファイルが変なところで終わっている");
			CONVERT_FLAG = 0;
			return ST_EOF;
		}
		ptr=file_buf;		// ポインタを先頭に戻す
		LINE++;				// 行数カウンタをインクリメントする
		STRING_BUF[0]='\0';	// 意味はあまり無いが安全のため...
		return ST_NEWL;
	}

	while((*ptr==' ')||(*ptr=='\t')) ptr++;
	if(*ptr==':') print_error("ラベル名がない");
	while((*ptr!=' ')&&(*ptr!='\t')&&(*ptr!=';')&&(*ptr!='\n')&&(*ptr!=':')){
		STRING_BUF[i++]=*(ptr++);
	}

	switch(*ptr){
		case(';'):case('\n'):case(' '):case('\t'):
			STRING_BUF[i]='\0';
		return (STRING_BUF[0]=='#') ? ST_CONVERT : ST_NORM;

		case(':'):				// ラベルのとき(コロンははずす)
			if(LABEL_FLAG==0)
				print_error("変なところにラベルがある");

			STRING_BUF[i]='\0';
			ptr++;	// かなり重要！
			label_set(STRING_BUF);
		goto HEAD;
	}
}

void label_set(char *name)
{
	int i;
	char buf[260];

	for(i=0;i<LABEL_NOW;i++){
		if(strcmp(name,LABEL_BUF[i].name)==0){
			if(LABEL_BUF[i].addr==0xff){
				LABEL_BUF[i].addr = CONVERT_PTR;
				return;
			}
			else{
				sprintf(buf,"ラベル %s が二重定義されている",name);
				print_error(buf);
			}
		}
	}

	strcpy(LABEL_BUF[LABEL_NOW].name,name);
	LABEL_BUF[LABEL_NOW].addr = CONVERT_PTR;
	LABEL_NOW++;
}

void adtmp_set(int label_no)
{
	ADDR_BUF[ADDR_NOW].file_addr = ftell(WRITE_FP);
	ADDR_BUF[ADDR_NOW].label_no  = label_no;
	ADDR_BUF[ADDR_NOW].linetemp  = LINE;
	ADDR_NOW++;
}

void addr_write(void)
{
	int i;
	LABEL *lp;
	char buf[260];
	long file_temp = ftell(WRITE_FP);

	for(i=0;i<ADDR_NOW;i++){
		lp=&LABEL_BUF[ADDR_BUF[i].label_no];
		if(lp->addr==0xff){
			LINE=ADDR_BUF[i].linetemp;
			sprintf(buf,"未定義のラベル %s を参照している",lp->name);
			print_error(buf);
		}
		fseek(WRITE_FP,ADDR_BUF[i].file_addr,SEEK_SET);
		fputc(lp->addr,WRITE_FP);
	}

	fseek(WRITE_FP,file_temp,SEEK_SET);
}

BYTE label_line(char *name)
{
	int i;

	for(i=0;i<LABEL_NOW;i++){
		if(strcmp(name,LABEL_BUF[i].name)==0){
			// ラベルのアドレスが未定義であった場合 //
			if(LABEL_BUF[i].addr==0xff) adtmp_set(i);
			return LABEL_BUF[i].addr;
		}
	}

	// ラベルが定義されていなかったとき //
	adtmp_set(LABEL_NOW);
	strcpy(LABEL_BUF[LABEL_NOW].name,name);
	LABEL_BUF[LABEL_NOW].addr = 0xff;
	LABEL_NOW++;
	return 0;		// ダミーの値を返す
}

void print_error(char *s)
{
	printf("だめだめ %5u行 : %s\n",LINE,s);
	exit(-1);
}

void debug_print(void)
{
	CONVERT_INFO *p;
	int i,j,val;

	for(i=0;i<ECLC_CMD_MAX;i++){
		p=&CMD_BUF[i];
		for(j=0,val=1;p->cmd[j]!=DT_END;j++){
			// break が無いところがポイント //
			switch(p->cmd[j]){
				case(DT_I):case(DT_16):           val++;
				case(DT_C):case(DT_8):case(DT_J): val++;
			}
		}
		printf("0x%02x:%20s:%5d Bytes\n",p->cmd_val,p->name,val);
	}

	exit(0);
}

void fput_char(char data)
{
	fputc((int)data,WRITE_FP);
	CONVERT_PTR++;
}

void fput_BYTE(BYTE data)
{
	fputc((int)data,WRITE_FP);
	CONVERT_PTR++;
}

void fput_int(int data)
{
	fwrite(&data,sizeof(data),1,WRITE_FP);
	CONVERT_PTR+=2;
}

void fput_WORD(WORD data)
{
	fwrite(&data,sizeof(data),1,WRITE_FP);
	CONVERT_PTR+=2;
}

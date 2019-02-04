/*                                                                           */
/*   DX_ERROR.c   DirectX のエラー出力用関数                                 */
/*                                                                           */
/*                                                                           */

#include "DX_ERROR.h"
#include <time.h>
#pragma message(PBGWIN_DX_ERROR_H)


// グローバル変数 //
static ERROR_DATA ErrorBuf;
static char			ErrorOut[1000];
//static FILE			*ErrorFP = NULL;


// エラー用バッファの初期化 //
extern void ErrSetup(void)
{
	ErrorBuf.s    = NULL;
	ErrorBuf.next = NULL;
}

// エラー文字列を追加 //
extern void ErrInsert(const char *s)
{
	ERROR_DATA *temp;

	// 自己参照構造体を作成 //
	temp = (ERROR_DATA *)LocalAlloc(LPTR,sizeof(ERROR_DATA));
	if(temp==NULL) return;

	// 文字列の追加 //
	temp->s = LocalAlloc(LPTR,sizeof(char)*(strlen(s)+2));
	if(temp->s==NULL){
		LocalFree(temp);
		return;
	}
	strcpy(temp->s,s);

	// ポインタ更新 //
	temp->next    = ErrorBuf.next;
	ErrorBuf.next = temp;

	DebugOut(s);
}

// エラー文字列出力＆メモリ解放 //
extern void ErrCleanup(void)
{
	ERROR_DATA *p = ErrorBuf.next,*temp;

	// 文字列を表示しながらメモリを解放 //
	while(p!=NULL){
		//MessageBox(NULL,p->s,"DirectX 関連のエラー",MB_OK);
		temp = p->next;
		LocalFree(p->s);
		LocalFree(p);
		p = temp;
	}

	//if(ErrorFP!=NULL) fclose(ErrorFP);
	ErrSetup();
}

extern void DebugSetup(const char *filename)
{
	char buf[1000];
	FILE *fp;

	//if(ErrorFP) return;
	strcpy(ErrorOut,filename);

	fp = fopen(ErrorOut,"a");
	if(fp==NULL) return;
	fseek(fp,0,SEEK_END);
	_strdate(buf);
	fprintf(fp,"[%s]",buf);
	_strtime(buf);
	fprintf(fp,"[%s]\n",buf);
	fclose(fp);
}

extern void DebugCleanup(void)
{
	strcpy(ErrorOut,"");
	//if(ErrorFP!=NULL) fclose(ErrorFP);
	//ErrorFP=NULL;
}

extern void DebugOut(const char *s)
{
	FILE *fp;
	fp= fopen(ErrorOut,"a");
	if(fp==NULL) return;
	fseek(fp,0,SEEK_END);
	fprintf(fp,"Error : %s\n",s);
	fclose(fp);
}

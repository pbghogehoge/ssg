/*                                                                           */
/*   CALC.cpp   式の解析                                                     */
/*                                                                           */
/*                                                                           */

#include "CALC.h"

/*
 * [新機能]
 *
 * 単項演算子 - を追加(二項演算子 + - * / よりも優先順位が高いことに注意)
 * 演算子の追加(^べき乗 |ＯＲ &ＡＮＤ <左シフト >右シフト)
 * 括弧に関するエラーチェックの強化
 * 演算用スタックのオーバー・アンダーフローのチェック
 * エラー出力関数を変更できるようにした
 */


///// [ 変数 ] /////
static char ope_dat[CALC_DATA_MAX];					// 演算子用スタック
static int  val_dat[CALC_DATA_MAX];					// 数値保持用スタック
static int  ope_pow[256];							// 演算子の優先順位(名前が変だな)
static int  ope_sp;									// 演算子用スタックポインタ
static int  val_sp;									// 数値保持用スタックポインタ
static void (*calc_err)(char *s);					// エラー出力用関数


///// [ 関数 ] /////
static void calc_top(void);							// 先頭で計算処理をして、ｓｐを更新する
static void ope_push(char ope);						// 演算子(ope)をPushする
static void val_push(int  val);						// 数値(val)をPushする



void CalcSetup(void (*func)(char *s))
{
	int i;

	// エラー出力用関数をセットする //
	calc_err = func;

	// 演算子の優先順位をセットする //
	for(i=0;i<256;i++) ope_pow[i]=0;
	ope_pow['(']=-1;
	ope_pow['+']=ope_pow['-']= 1;
	ope_pow['*']=ope_pow['/']= 2;
	ope_pow['^']=3;
	ope_pow['|']=ope_pow['&']=ope_pow['<']=ope_pow['>']=4;
	ope_pow[OPE1_MINUS]= 5;
}

int Calc(char *factor)
{
	int		value;
	int		paren_count = 0;		// 括弧の数カウンタ
	BOOL	bMinusFlag  = TRUE;		// 単項演算子フラグ

	if(calc_err==(void *)0) return 0;

	ope_dat[0] = 0;
	ope_sp     = 0;
	val_sp     = -1;	// いいのか？

	while(*factor!='\0'){
		switch(*factor){
			case('('):
				bMinusFlag = TRUE;	// 単項演算子フラグの初期化
				ope_push(*factor);
				paren_count++;factor++;
			break;

			case(')'):
				if(--paren_count<0)
					calc_err("左かっこの数が少ない");

				while(ope_dat[ope_sp]!='(') calc_top();	// 結果をこきだす
				ope_sp--;factor++;
			break;

			case('-'):
				if(bMinusFlag){
					bMinusFlag = FALSE;
					ope_push(OPE1_MINUS);
					factor++;
					break;
				}
			case('+'):case('*'):case('/'):case('^'):case('|'):case('&'):
			case('>'):case('<'):
				if(ope_pow[ope_dat[ope_sp]]>=ope_pow[*factor]) calc_top();
				ope_push(*factor);
				factor++;
			break;

			case('0'):case('1'):case('2'):case('3'):case('4'):
			case('5'):case('6'):case('7'):case('8'):case('9'):
				for(value=0;*factor>='0'&&*factor<='9';factor++){
					value*=10;
					value+=(*factor)-'0';
				}
				val_push(value);
				bMinusFlag = FALSE;	// 代入されたらフラグは必ずオフにする
			break;

			default:
					calc_err("謎の演算子もしくは定義されていない定数がある");
			break;
		}
	}

	if(paren_count>0)
		calc_err("右かっこの数が少ない");

	while(ope_sp>0) calc_top();	// 結果をこきだす

	return val_dat[0];
}

static void calc_top(void)
{
	int i;

	switch(ope_dat[ope_sp]){
		case(OPE1_MINUS):		// 単項演算子 -
			val_dat[val_sp]*=(-1);
			if((--ope_sp)<0)
				calc_err("スタックアンダーフローです");
		return;

		case('+'):				// 加算(二項)
			val_dat[val_sp-1]+=val_dat[val_sp];
		break;

		case('-'):				// 減算(二項)
			val_dat[val_sp-1]-=val_dat[val_sp];
		break;

		case('*'):				// 乗算(二項)
			val_dat[val_sp-1]*=val_dat[val_sp];
		break;

		case('/'):				// 除算(二項)
			if(val_dat[val_sp]==0)
				calc_err("ゼロで除算しようとしている");

			val_dat[val_sp-1]/=val_dat[val_sp];
		break;

		case('^'):				// べき乗
			if(val_dat[val_sp]==0){		// ０乗
				val_dat[val_sp-1]=1;
				break;
			}
			i=val_dat[val_sp-1];
			while((--val_dat[val_sp])>0)
				val_dat[val_sp-1]*=i;
		break;

		case('&'):				// 論理積
			val_dat[val_sp-1]&=val_dat[val_sp];
		break;

		case('|'):				// 論理和
			val_dat[val_sp-1]|=val_dat[val_sp];
		break;

		case('<'):				// 左シフト
			val_dat[val_sp-1]=val_dat[val_sp-1]<<val_dat[val_sp];
		break;

		case('>'):				// 右シフト
			val_dat[val_sp-1]=val_dat[val_sp-1]>>val_dat[val_sp];
		break;
	}

	// エラーの場合はデクリメントされなくても良いので... //
	if((--val_sp)<0 || (--ope_sp)<0)
		calc_err("スタックアンダーフローです");
}

static void ope_push(char ope)
{
	if(ope_sp+1>=CALC_DATA_MAX)
		calc_err("スタックオーバーフローです");

	ope_dat[++ope_sp]=ope;
}

static void val_push(int value)
{
	if(val_sp+1>=CALC_DATA_MAX)
		calc_err("スタックオーバーフローです");

	val_dat[++val_sp]=value;
}

/*                                                                           */
/*   CALC.h   式の解析                                                       */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_CALC_H
#define PBGWIN_CALC_H		"CALC : Version 0.02 : Update 2000/02/03"
#pragma message(PBGWIN_CALC_H)

#include <windows.h>



///// [ 定数 ] /////
#define CALC_DATA_MAX	100		// スタック領域の大きさ
#define OPE1_MINUS		'@'		// 単項演算子 - に対する記号



///// [ 関数 ] /////
extern void CalcSetup(void (*func)(char *s));		// 計算の準備をする
extern int  Calc(char *factor);						// 引数：空白の無い式



#endif

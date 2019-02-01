/*************************************************************************************************/
/*   TAMA.H   たまに関する定義とかいろいろ                                                       */
/*                                                                                               */
/*************************************************************************************************/

#ifndef PBGWIN_TAMA_H
#define PBGWIN_TAMA_H		"TAMA : Ver 0.62 : Update 2000/02/17"
//#pragma message(PBGWIN_TAMA_H)


///// [更新履歴] /////


/* -> ここから、ちと古いよ(1999...)
 * 変更しなけりゃいけないこと、とか
 * >ox,oy を廃止すること(TAMA.C も変更しなきゃならんし、面倒だな...)
 *
 * > 5/17 (2:43)  : ox,oy を廃止した。 弾の種類によっては自動的にクリッピングをするようにした
 *                : 上の奴との関連により .flag の機能を拡張しました(下の定数参照)
 *
 * > 6/13 (8:05)  : どっと単位のクリッピング
 */


#include <windows.h>
#include "PBGUTY_X.h"



////弾定数////
#define TAMA_MAX	(801*3)		// 弾の最大発生数
#define TAMA_EVADE	1			// 弾のかすり値

#define TAMA1_POINT		10000				// 弾の得点
#define TAMA2_POINT		15000				// 弾の得点

#define TAMA_HITX		(2*64)			// 弾の当たり判定
#define TAMA_HITY		(4*64)			// 弾の当たり判定
#define TAMA_EVX_SMALL	(( 8+ 8)*64)	// たま（小）のかすり判定(x)
#define TAMA_EVY_SMALL	((16+ 8)*64)	// たま（小）のかすり判定(y)
#define TAMA_EVX_LARGE	(( 8+16)*64)	// たま（大）のかすり判定(x)
#define TAMA_EVY_LARGE	((16+16)*64)	// たま（大）のかすり判定(y)

#define TAMA_SMALL	0x00	// 弾が小型弾である場合の上位４ビット
#define TAMA_LARGE	0x10	// 弾が大型弾である場合の上位４ビット
#define TAMA_ANGLE	0x20	// 弾が方向指定系である場合の上位４ビット
#define TAMA_EXTRA	0x30	// 弾がエキストラ用である場合の上位４ビット
#define TAMA_EXTRA2	0x40	// 弾が「おふだ」である場合の上位４ビット
#define TAMA_REN	0x04	// 弾の連射属性
#define TAMA_ZSET	0x08	// 弾のサボテン(自機)セット属性
#define TAMASP_RND0	0x00	// 速度ランダム無し
#define TAMASP_RND1	0x40	// 速度ランダム？？
#define TAMASP_RND2	0x80	// 速度ランダム？？
#define TAMASP_RND3	0xc0	// 速度ランダム？？


////弾の種類定数(上位４ビットは現在、使用目的がない)////
#define T_NORM		0x00	// 通常弾			:(vx,vy)で移動します
#define T_NORM_A	0x01	// 加速弾			:rep 加速回数?
#define T_HOMING	0x02	// ｎ回ホーミング	:rep ホーミング回数 / a 加速度
#define T_HOMING_M	0x03	// ｎ％ホーミング	:a 加速度 / vd ホーミング率
#define T_ROLL		0x04	// 回転弾			:rep 回転時間 / vd 角速度
#define T_ROLL_A	0x05	// 回転弾(加速)		:上の奴 + a 加速度
#define T_ROLL_R	0x06	// 回転弾(反転)		:上の奴と同じ
#define T_GRAVITY	0x07	// 落下弾			:(vx,vy)＆vyに(加速度a)がかかる
#define T_CHANGE	0x08	// 角度変更弾		:rep フレームでvdに角度変更
#define T_SBHOMING	0x09	// サボテン用ホーミング
#define T_SBHBOMB	0x0a	// サボテン用ホーミングボム


////弾オプション定数(下位４ビットはオプションの成分指定用)////
#define TOP_NONE	0x00	// オプションなし
#define TOP_WAVE	0x10	// 波		: 振幅
#define TOP_ROLL	0x20	// 回転		: 回転半径
#define TOP_PURU	0x30	// ぷるぷる	: ぷるぷる度
#define TOP_REFX	0x40	// 反射Ｘ	: 反射回数
#define TOP_REFY	0x50	// 反射Ｙ	: 反射回数
#define TOP_REFXY	0x60	// 反射ＸＹ	: 反射回数
#define TOP_DIV		0x70	// 分裂		: 分裂時の弾コマンド
#define TOP_BOMB	0x80	// ボム???	: 爆発半径


////弾コマンド定数////
#define TC_WAY		0x00	// 扇状発射
#define TC_ALL		0x01	// 全方向発射
#define TC_RND		0x02	// 基本角セット有りランダム
#define TC_WAYS		0x04	// 扇状発射＆連射
#define TC_ALLS		0x05	// 全方向発射＆連射
#define TC_RNDS		0x06	// 基本角セット有りランダム＆連射
#define TC_WAYZ		0x08	// 扇状発射＆サボテンセット
#define TC_ALLZ		0x09	// 全方向発射＆サボテンセット
#define TC_RNDZ		0x0a	// 基本角サボテンセットランダム
#define TC_WAYSZ	0x0c	// 扇状発射＆連射＆サボテンセット
#define TC_ALLSZ	0x0d	// 全方向発射＆連射＆サボテンセット
#define TC_RNDSZ	0x0e	// 基本角サボテンセットランダム＆連射


////弾エフェクト定数(下位４ビットの使用方法は現在考案中!!)////
#define TE_NONE		0x00	// エフェクトなし
#define TE_ROLL1	0x10	// 回転ためエフェクト
#define TE_ROLL2	0x20	// 回転ためエフェクト
#define TE_WARN		0x30	// Warning表示
#define TE_ROCK		0x40	// ロックオン
#define TE_CIRCLE1	0x50	// わっかエフェクト(小->大)
#define TE_CIRCLE2	0x60	// わっかエフェクト(大->小)
#define TE_DELETE	0xf0	// 消去エフェクト


////弾フラグ定数////
#define TF_NONE		0x00	// フラグが立っていない状態
#define TF_CLIP		0x01	// 画面外に出ても消去しない
#define TF_EVADE	0x02	// 一回かすっている場合
#define TF_DELETE	0x80	// その弾を消去する



////弾コマンド構造体(安全性アップ)////
typedef struct{
	int  x,y;		// 弾の発射位置

	BYTE d;			// 発射角
	BYTE dw;		// 発射幅
	BYTE n;			// 弾数(ｎ方向に発射)
	BYTE ns;		// 連射数(cmdのsビットがONのときだけ有効)
	BYTE v;			// 速度(下位６ビット)＆ランダム要素(上位２ビット)
	BYTE c;			// 弾の色＆形状
	char a;			// 加速度(速度とは単位が違うので注意)

	char vd;		// 角速度｜ホーミング率(BYTE にキャスト)

	BYTE rep;		// 繰り返し回数(回転、ｎ回ホーミング等)
	BYTE cmd;		// 弾コマンド＆エフェクト
	BYTE type;		// 弾の種類
	BYTE option;	// 弾の属性(バイブレーション,反射,炸裂,ボム)
} TAMA_CMD;



////弾データ構造体////
typedef struct{
	int x,y;		// 現在の<表示>座標
	int tx,ty;		// 振動系エフェクト使用時の演算用座標
	int vx,vy;		// 速度の(X,Y)成分

 	int  v;			// 速度
	int  v0;		// 初速度(回転系エフェクト等で使用)
	char a;			// 加速度

	BYTE d;			// 進行角
	WORD d16;		// 進行角(固定小数点 x256) -> ｎ％ホーミングでのみ使用する
	char vd;		// 角速度

	BYTE c;			// 弾の色＆形状

	BYTE rep;		// typeによる制御を行う回数
	BYTE type;		// 弾の種類(通常,加速,ホーミング2,回転3,落下,変更)
	BYTE option;	// 弾の属性(バイブレーション,反射,炸裂,ボム)
	BYTE effect;	// 実行中のエフェクト(なし,ロック,サークル,消去)

	WORD count;		// フレームカウンタ
	BYTE flag;		// 弾消去要請フラグ
} TAMA_DATA;



////弾の各種変数たち////
extern TAMA_CMD		TamaCmd;			// 標準・弾コマンド構造体
extern TAMA_DATA	Tama[TAMA_MAX];		// 弾の格納用構造体
extern WORD			Tama1Ind[TAMA_MAX];	// 小型弾の順番を維持するための配列
extern WORD			Tama2Ind[TAMA_MAX];	// 特殊弾の順番を維持するための配列
extern WORD			Tama1Now;			// 小型弾の弾数
extern WORD			Tama2Now;			// 特殊弾の弾数
extern WORD			Tama1Max;			// 小型弾の最大数
extern WORD			Tama2Max;			// 特殊弾の最大数



////弾関数////
void tama_set(void);					// 弾をセットする(難易度による変更は"有り")
void tama_setEX(void);					// 弾をセットする(難易度による変更は"無し")
void tama_setLine(void);				// 弾をセットする(ライン状に発射)
void tama_setExtra01(void);				// エキストラボス専用弾幕
void tama_move(void);					// 弾を動かす
void tama_draw(void);					// 弾を描画する
void tama_clear(void);					// 弾に消去エフェクトをセットする
void tamaind_set(WORD tama1);			// 弾の個数の割合をセットする(危険！)

DWORD tama2score(void);					// 弾を得点化する(Ret : 得点)
void  tama2item(BYTE n);				// 弾の 1/n をアイテム化する

BYTE __fastcall tama_dir(WORD i);					// 弾の進行方向をセットする
int  __fastcall tama_speed(WORD i);					// 弾の初速度をセットする
BYTE __fastcall tama_flag(void);					// 弾のフラグを初期化する

void __fastcall tamaTmove(TAMA_DATA *t);			// 弾の " Type " による移動を行う
void __fastcall tamaOmove(TAMA_DATA *t);			// 弾の "Option" による移動を行う
void __fastcall tamaEmove(TAMA_DATA *t);			// 弾の "Effect" による移動を行う


//// かすり用マクロ ////
extern FVOID evade_addEx(int x,int y,BYTE n);		// かすりゲージを上昇させる

IVOID TamaEvadeAdd(TAMA_DATA *t)
{
	if(t->flag&TF_EVADE) evade_addEx(t->x,t->y,0);
	else{
		t->flag |= TF_EVADE;
		evade_addEx(t->x,t->y,TAMA_EVADE);
	}
}

//// 弾コマンド用マクロ ////
_inline void TamaSetForm(BYTE cmd,BYTE option,BYTE type,BYTE c)
{
	TamaCmd.cmd    = cmd;
	TamaCmd.option = option;
	TamaCmd.type   = type;
	TamaCmd.c      = c;
}

_inline void TamaSTDForm(BYTE c)
{
	TamaSetForm(TC_WAY,TOP_NONE,T_NORM,c);
}

_inline void TamaSetDeg(BYTE d,BYTE dw)
{
	TamaCmd.d  = d;
	TamaCmd.dw = dw;
}

_inline void TamaSetNum(BYTE n,BYTE ns)
{
	TamaCmd.n  = n;
	TamaCmd.ns = ns;
}

_inline void TamaSetSpd(BYTE v,char a)
{
	TamaCmd.v = v;
	TamaCmd.a = a;
}

_inline void TamaSetXY(int x,int y)
{
	TamaCmd.x = x;
	TamaCmd.y = y;
}


#endif

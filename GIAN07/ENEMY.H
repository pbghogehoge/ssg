/*************************************************************************************************/
/*   ENEMY.H   敵の管理とか発生制御等                                                            */
/*                                                                                               */
/*************************************************************************************************/

#ifndef PBGWIN_ENEMY_H
#define PBGWIN_ENEMY_H		"ENEMY : Ver 0.41 : Update 2000/02/22"
//#pragma message(PBGWIN_ENEMY_H)

///// [ 更新履歴 ] /////

// 2000/10/17 : プレイランクの変数を参照するのを間違えていた(ConfigDat のほうを参照していた)
// 2000/03/22 : LLaser の処理を追加(実際の発射は構造体そのものに直接代入する事で行う)
// 2000/02/25 : 敵の当たり判定チェック用関数 enemy_damage() を追加
// 2000/02/22 : 敵のクリッピング範囲を変更した。

/* -> ここから下は古いよ
 * > 8/23 (20:56)  : (ox,oy) は使用しないものとする
 *                 : ECL_Head から始まるデータ <敵の数><開始アドレスx敵の数><ECLデータ>
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ECL.H"
#include "TAMA.H"
#include "LASER.H"


//// 敵定数 ////
#define ENEMY_MAX		50			// 敵の最大発生数

#define EF_DRAW			0x01		// 敵を描画するか
#define EF_CLIP			0x02		// 敵が画面外に出たとき消去するか
#define EF_DAMAGE		0x04		// 敵にダメージを与えられるか
#define EF_HITSB		0x08		// 敵と自機は接触するか
#define EF_RLCHG		0x10		// ＥＣＬ左右反転を有効にするか
#define EF_BOMB			0x20		// 敵が爆発中である
#define EF_DELETE		0x40		// 敵をこのターン中に消去する

#define ENEMY_BOMB_SPD	4

//// ホーミング定数 ////
#define HOMING_DUMMY	(500*64)	// 敵をホーミングしない場合のダミー値


////アニメーション定数////
#define ANIME_MAX		50			// アニメーションの種類
#define ANIMEPTN_MAX	16			// アニメーションパターンの最大値
#define ANM_NORM		0x00		// 普通のアニメーション
#define ANM_DEG			0x01		// 角度でアニメーションする
#define ANM_STOP		0x02		// 最終パターンで静止する


//// 割り込みベクタ構造体 ////
typedef struct{
	DWORD	vect;		// 割り込みベクタ(0 なら無効)
	int		value;		// 比較値
} INT_VECTOR;

//// 敵データ構造体 ////
typedef struct{
	int			x,y;		// 表示座標
	int			vx,vy;		// 速度の(x,y)成分 x64系

	int			v;			// 速度成分 x64系

	DWORD		hp;			// 残り体力(大きすぎるか？)
	DWORD		item;		// アイテムその他に使用か？
	DWORD		cmd;		// ECLコマンドの絶対アドレス(DOS版との大きな変更点)
	DWORD		count;		// 多目的フレームカウンタ
	DWORD		call_addr;	// RET 実行後にジャンプするアドレス

	DWORD		score;		// 得点(時間による得点変動対応か？)
	DWORD		evscore;	// かすり得点用

	DWORD		IntTimer;	// 割り込みようタイマー

	DWORD		GR[ECLREG_MAX];		// 変数用レジスタ
	INT_VECTOR	Vect[ECLVECT_MAX];	// 割り込みベクタ

	WORD		g_width;	// グラフィックの幅  /2*64(当たり判定にも使用)
	WORD		g_height;	// グラフィックの高さ/2*64(上に同じ)
	WORD		rep_c;		// REP 命令用カウンタ
	WORD		cmd_c;		// 実行中コマンドの繰り返し回数
	WORD		anm_c;		// アニメーションカウンタ

	BYTE		d;			// 進行角 256
	char		vd;			// 角速度 128
	BYTE		amp;		// 振幅   256
	BYTE		anm_ptn;	// 使用しているアニメーションパターン
	BYTE		anm_ptnEx;	// ダメージ中のアニメーションパターン
	char		anm_sp;		// アニメーションスピード
	BYTE		IsDamaged;	// ダメージを受けたか

	BYTE		flag;		// 敵状態フラグ(状況によってサイズ変更のこと)
	BYTE		tama_c;		// 弾発射用カウンタ
	BYTE		t_rep;		// 弾発射間隔

	BYTE		LLaserRef;	// 太レーザーの参照カウント

	TAMA_CMD	t_cmd;		// 弾発射用コマンド
	LASER_CMD	l_cmd;		// レーザー発射用コマンド
} ENEMY_DATA;

typedef struct{
	BYTE	mode;				// アニメーションモード
	BYTE	n;					// アニメーションパターン数
	int		width;				// 絵の幅
	int		height;				// 絵の高さ
	RECT	ptn[ANIMEPTN_MAX];	// アニメーションの存在する矩形領域
} ANIME_DATA;

//// 敵変数 ////
extern ENEMY_DATA	Enemy[ENEMY_MAX];
extern BYTE			*ECL_Head;
extern BYTE			*SCL_Head,*SCL_Now;
extern WORD			EnemyInd[ENEMY_MAX];
extern WORD			EnemyNow;
extern ANIME_DATA	Anime[ANIME_MAX];

extern int HomingX;			// ホーミング対象のＸ座標
extern int HomingY;			// ホーミング対象のＹ座標
extern int HomingFlag;		// 真ならホーミング実行


//// 敵制御関数 ////
extern void enemy_move(void);		// 敵を動かす
extern void enemy_draw(void);		// 敵を描画する
extern void enemyind_set(void);		// 敵の順序設定用配列の初期化をする
extern void enemy_clear(void);		// 雑魚を消滅させる

extern BOOL enemy_damage(int x,int y,int damage);	// 敵にダメージを与える
extern BOOL enemy_damage2(int x,int y,int damage);	// ｙ上方向無限Ver.敵にダメージを与える
extern void enemy_damage3(int x,int y,BYTE d);		// ナナメレーザーの当たり判定
extern void enemy_damage4(int damage);				// すべての敵にダメージを与える

extern void SetAnime24(int x,int y,int id);
extern void SetAnime32(int x,int y,int id,int n,BYTE mode);
extern void SetAnime40(int x,int y,int id);
extern void SetAnime48(int x,int y,int id);
extern void SetAnime80(int x,int y,int id);
extern RECT *Enemy_AnimeClip(int *x,int *y,ENEMY_DATA *e);
extern RECT *Enemy_HitGrpClip(int *x, int *y, ENEMY_DATA *e);
extern void EnemyAnimeMove(ENEMY_DATA *e);


// 敵データを初期化する(x,y は x64 で指定のこと) //
extern void InitEnemyDataX64(ENEMY_DATA *e,int x,int y,DWORD EclID);

// 敵データを初期化する(x,y は非x64(ランダム可能) で指定のこと) //
extern void InitEnemyDataSTD(ENEMY_DATA *e,short x,short y,DWORD EclID);

// 強制的に ECL ブロック間を移動する //
extern void EnemyECL_LongJump(ENEMY_DATA *e, DWORD EclID);

extern void UpdateHoming(ENEMY_DATA *e);		// ホーミング座標を更新する
extern void parse_ECL(ENEMY_DATA *e);			// 敵をＥＣＬに従って動かす
extern void CheckECLInterrupt(ENEMY_DATA *e);	// 割り込みジャンプを調べる
extern void InitECLInterrupt(ENEMY_DATA *e);	// 割り込みベクタの初期化


// Vivit ナナメレーザーの当たり判定 //
IBOOL LaserHITCHK(ENEMY_DATA *e, int ox, int oy, BYTE d)
{
	int		tx,ty,l,w;
	int		chkw;

	chkw = min(e->g_height, e->g_width) + 3*64;

	tx = e->x - ox;
	ty = e->y - oy;

	l =  cosl(d,tx) + sinl(d,ty);
	w = abs(-sinl(d,tx) + cosl(d,ty));

	if(l>0 && w<chkw) return TRUE;
	else              return FALSE;
}


#endif

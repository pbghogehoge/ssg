/*************************************************************************************************/
/*   LASER.H   レーザーに関する処理(反射,ショート)                                               */
/*                                                                                               */
/*************************************************************************************************/

#ifndef PBGWIN_LASER_H
#define PBGWIN_LASER_H		"LASEER : Ver 0.51 : Update 2000/02/17"
//#pragma message(PBGWIN_LASER_H)



///// [更新履歴] /////

// 2000/02/17 : 新しいシステムに移行開始。無限遠レーザーと完全に分離

/*-> ここからはちょっと古いよ(1999..)
 * (4/3)  10:36 開発開始
 * (4/6)  12:00 ついにポリゴン＆クリッピング関数が完成。描画はいつ出来るのか？
 * (4/7)  12:02 全てのレーザーを同じ構造体で扱う事にした
 * (4/8)   7:23 無限遠レーザーの制作
 * (4/9)   2:01 反射レーザーを一応打ち込み終わる
 * (4/9)   2:59 反射レーザー完成
 * (4/11) 14:05 ショート＆反射レーザーの当たり判定完成
 * (4/11) 15:17 リフレクターのヒットチェックを強化(バグは消えたが遅くなった)
 *
 * (9/23) 16:18 ライン描画、ＥＣＬ対応などが完了
 */


#include <windows.h>



////レーザー定数////
#define LASER_MAX			1000			// レーザーの最大発生本数
#define RT_MAX				10				// 反射物の最大個数
#define LS_ZSET				0x08			// レーザーのサボテン(自機)セット属性
#define SLASER_EVADE		3				// ショートレーザーのかすりポイント
#define LASER_EVADE_WIDTH	(12*64)			// レーザーのかすり許容範囲


////レーザーの種類定数////
#define LS_SHORT	0x00	// ショートレーザー
#define LS_REF		0x01	// 反射レーザー
#define LS_LONG		0x02	// 無限遠レーザー
#define LS_LONGY	0x03	// 縦無限遠レーザー(角度指定は無効 64固定)


////レーザーコマンド定数////
#define LC_WAY		0x00	// 扇状発射
#define LC_ALL		0x01	// 全方向発射
#define LC_RND		0x02	// 基本角セット有りランダム
#define LC_WAYZ		0x08	// 扇状発射＆サボテンセット
#define LC_ALLZ		0x09	// 全方向発射＆サボテンセット
#define LC_RNDZ		0x0a	// 基本角サボテンセットランダム


////レーザーフラグ定数(一部レーザーの種類に依存します)////
#define LF_NONE		0x00	// フラグ無し状態
#define LF_CLEAR	0x01	// レーザーが消滅中である

#define LF_SHOT		0x02	// レーザー発射中
#define LF_HIT		0x04	// レーザーヒット中(REF_OBJECTに対して)
#define LF_NMOVE	0x06	// レーザーの長さ変わらず(LF_SHOT|LF_HIT)

#define LF_DELETE	0x80	// レーザーを消去する(処理対象から外す)


////レーザー発動コマンド構造体////
typedef struct{
	int  x,y;		// 始点の座標
	int  v;			// レーザーの初速度

	int w;			// レーザーの太さ        (x64座標を使用する)
	int l;			// レーザーの長さ最終値  (x64座標を使用する)
	int l2;			// レーザーの発射位置補正(x64...)

	BYTE d;			// 発射角
	BYTE dw;		// 発射幅

	BYTE n;			// レーザーの本数
	BYTE c;			// レーザーの色

	char a;			// 加速度(つかうのかな???)
	BYTE cmd;		// レーザー発動コマンド(ほとんど弾と同じかも)
	BYTE type;		// ショート、無限遠など
	BYTE notr;		// 反射しないリフレクターの番号
} LASER_CMD;


////レーザー用構造体////
typedef struct{
	int x,y;		// 現在の始点
	int vx,vy;		// 速度の(X,Y)成分
	int lx,ly;		// 表示座標の加算値(長さ)
	int wx,wy;		// 表示座標の加算値(太さ)
	int v;			// 速度

	POINT p[4];		// 表示する座標

	char a;			// 加速度(つかうのか??)
	BYTE d;			// 進行方向

	int w,wmax;		// 太さ
	int l,lmax;		// 現在の長さ、長さの最終値
	int ltemp;		// 反射レーザー専用変数(発射＆ヒットの場合にのみ使用)

	WORD count;		// フレームカウンタ
	BYTE c;			// 色
	BYTE type;		// 種類
	BYTE flag;		// 消去要請フラグ等(エフェクト含む)
	BYTE notr;		// 反射しないリフレクターの番号
	BYTE evade;		// かすり用フラグ
} LASER_DATA;

/*
////反射物(鏡?) 構造体////
typedef struct{
	int		x,y;		// 反射物の中心座標

	DWORD	l;			// リフレクターの長さ(中心から先端まで,つまり全体でl*2)
	BYTE	d;			// 反射物の角度(0 <= d < 128)
} REFLECTOR;
*/

////レーザー関数////
extern void laser_set(void);		// レーザーをセットする(難易度変更"有り")
extern void laser_setEX(void);		// レーザーをセットする(難易度変更"無し")
extern int  llaser_set(WORD *ind);	// 無限遠レーザーをセットする(セットできた数を返すのだ)
extern void laser_move(void);		// レーザーを動かす
extern void laser_draw(void);		// レーザーを描画する
extern void laser_clear(void);		// レーザー全てに消去エフェクトをかける
extern void laserind_set(void);		// レーザー順序用配列の初期化


////レーザーの各種変数たち////
extern LASER_CMD	LaserCmd;				// 標準レーザーコマンド構造体
extern LASER_DATA	Laser[LASER_MAX];		// レーザー格納用構造体
extern WORD			LaserInd[LASER_MAX];	// レーザー順番維持用配列
extern WORD			LaserNow;				// レーザーの本数
//extern REFLECTOR	Reflector[RT_MAX];		// 反射物構造体
//extern WORD			ReflectorNow;			// 反射物の個数

#endif

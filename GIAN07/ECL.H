/*                                                                           */
/*   ECL.h   敵コントロール言語用の定数                                      */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_ECL_H
#define PBGWIN_ECL_H	"ECL : Ver 0.77 : Update 2000/11/27"
//#pragma message(PBGWIN_ECL_H)



///// [更新履歴] /////

// 2000/11/27 : STG4EFC を追加・それに関する定数を追加
// 2000/10/16 : JEQ を追加
// 2000/09/05 : HLASER を追加
// 2000/04/26 : LASER2 を追加
// 2000/03/22 : LLaser 命令を追加
// 2000/03/15 : 命令を大幅に追加 (割り込み系、弾消去、レジスタ比較)
// 2000/02/18 : システムのアップデート開始



#include <windows.h>



///// [ 定数 ] /////

// ＥＣＬコマンド最大数 //
#define ECL_CMDMAX	(ECL_CMD0+ECL_CMD1+ECL_CMD2+ECL_CMD45+ECL_CMD67+ECL_CMD8+ECL_CMD9+ECL_CMDA+ECL_CMDB)


// 0x0? : 制御用コマンド //
#define ECL_CMD0		14		//
#define ECL_SETUP		0x00	// 敵データ初期化
#define ECL_END			0x01	// 敵強制消滅
#define ECL_JMP			0x02	// 強制ジャンプ
#define ECL_LOOP		0x03	// ループ(２重は不可、CX は使わないの....)
#define ECL_CALL		0x04	// サブルーチンを呼ぶ
#define ECL_RET			0x05	// サブルーチンから復帰する
#define ECL_JHPL		0x06	// ＨＰが指定値より大きければジャンプ
#define ECL_JHPS		0x07	// ＨＰが指定値より小さければジャンプ
#define ECL_JDIF		0x08	// 難易度によるswitch
#define ECL_JDSB		0x09	// 進行角度がサボテンと一致したらジャンプ(誤差±４まで有効)
#define ECL_JFCL		0x0A	// フレームカウンタが大きければジャンプ
#define ECL_JFCS		0x0B	// フレームカウンタが小さければジャンプ
#define ECL_STI			0x0C	// 割り込みベクタをセットする(SeTInterrupt flag)
#define ECL_CLI			0x0D	// 割り込みを無効にする(CLearInterrupt flag)


// 0x1? : 移動用コマンド //
#define ECL_CMD1		16		//
#define ECL_NOP			0x10	// 何もしない
#define ECL_NOPSC		0x11	// スクロールに流される
#define ECL_MOV			0x12	// 移動する
#define ECL_ROL			0x13	// 回転移動
#define ECL_LROL		0x14	// 直進＆回転移動
#define ECL_WAVX		0x15	// 波移動Ｘ
#define ECL_WAVY		0x16	// 波移動Ｙ
#define ECL_MXA			0x17	// Ｘ絶対移動
#define ECL_MYA			0x18	// Ｙ絶対移動
#define ECL_MXYA		0x19	// ＸＹ絶対移動
#define ECL_MXS			0x1A	// Ｘサボテンセット移動
#define ECL_MYS			0x1B	// Ｙサボテンセット移動
#define ECL_MXYS		0x1C	// ＸＹサボテンセット移動
#define ECL_ACC			0x1D	// 加速or減速つき移動
#define ECL_ACCXYA		0x1E	// 減速付きＸＹ絶対セット
#define ECL_GRAX		0x1F	// 重力付きＸ反射移動(Ｙ>=GY_MAX ならば自動消滅)


// 0x2? : 数値セット用コマンド //
#define ECL_CMD2		15		//
#define ECL_DEGA		0x20	// 角度絶対セット
#define ECL_DEGR		0x21	// 角度相対セット
#define ECL_DEGX		0x22	// 角度ランダムセット
#define ECL_DEGS		0x23	// 角度サボテンセット
#define ECL_SPDA		0x24	// 速度絶対セット
#define ECL_SPDR		0x25	// 速度相対セット
#define ECL_XYA			0x26	// 座標絶対セット
#define ECL_XYR			0x27	// 座標相対セット
#define ECL_DEGXU		0x28	// 角度ランダムセット(上１２８度)
#define ECL_DEGXD		0x29	// 角度ランダムセット(下１２８度)
#define ECL_DEGEX		0x2A	// 角度特殊セット(EXDEGDと併用する)
#define ECL_XYS			0x2B	// 座標サボテンセット
#define ECL_DEGX2		0x2C	// 制限付き角度ランダム
#define ECL_XYRND		0x2D	// 制限付き座標ランダム
#define ECL_XYL			0x2E	// 長さ指定座標相対(極座標的に指定)


// 0x4? : 弾発射用コマンド //
#define ECL_CMD45		22		//
#define ECL_TAMA		0x40	// 弾発射
#define ECL_TAUTO		0x41	// 弾発射間隔をセットする(０：自動発射しない)
#define ECL_TXYR		0x42	// 弾発射位置の相対ずらし
#define ECL_TCMD		0x43	// 弾コマンド
#define ECL_TDEGA		0x44	// 弾発射角絶対指定
#define ECL_TDEGR		0x45	// 弾発射角相対指定
#define ECL_TNUMA		0x46	// 弾数絶対指定
#define ECL_TNUMR		0x47	// 弾数相対指定
#define ECL_TSPDA		0x48	// 弾初速度絶対指定
#define ECL_TSPDR		0x49	// 弾初速度相対指定
#define ECL_TOPT		0x4a	// 弾オプション指定
#define ECL_TTYPE		0x4b	// 弾の種類指定
#define ECL_TCOL		0x4c	// 弾の色または形状指定
#define ECL_TVDEG		0x4d	// 弾の角速度指定
#define ECL_TREP		0x4e	// 弾の繰り返し用
#define ECL_TDEGS		0x4f	// 弾発射角サボテンセット
#define ECL_TDEGE		0x50	// 弾発射角を自分の向きにセット
#define ECL_TAMA2		0x51	// 難易度変化なし弾発射
#define ECL_TCLR		0x52	// 全ての弾を消去する
#define ECL_TAMAL		0x53	// 弾をライン状に発射する
#define ECL_T2ITEM		0x54	// 弾の何割かをアイテム化する
#define ECL_TAMAEX		0x55	//	エキストラボス用弾幕発射コマンド


// 0x6? : レーザー発射用コマンド //
#define ECL_CMD67		18		//
#define ECL_LASER		0x60	// レーザー発射
#define ECL_LCMD		0x61	// レーザーコマンド
#define ECL_LLA			0x62	// レーザー長・絶対指定
#define ECL_LLR			0x63	// レーザー長・相対指定
#define ECL_LL2			0x64	// レーザー発射位置
#define ECL_LDEGA		0x65	// レーザー発射角絶対指定
#define ECL_LDEGR		0x66	// レーザー発射角相対指定
#define ECL_LNUMA		0x67	// レーザーの本数絶対指定
#define ECL_LNUMR		0x68	// レーザーの本数相対指定
#define ECL_LSPDA		0x69	// レーザーの速さ絶対指定
#define ECL_LSPDR		0x6a	// レーザーの速さ相対指定
#define ECL_LCOL		0x6b	// レーザーの色
#define ECL_LTYPE		0x6c	// レーザーの種類
#define ECL_LWA			0x6d	// レーザーの太さ絶対指定
#define ECL_LDEGS		0x6e	// レーザー発射角サボテンセット
#define ECL_LDEGE		0x6f	// レーザー発射角を自分の向きにセット
#define ECL_LXY			0x70	// レーザーの発射座標セット(太レーザー用？)
#define ECL_LASER2		0x71	// レーザー発射


// 0x8? : 太レーザー&ホーミング発射用コマンド(構造体セットは上の命令を使用する) //
#define ECL_CMD8		6		//
#define ECL_LLSET		0x80	// 太レーザーセット
#define ECL_LLOPEN		0x81	// 太レーザーオープン
#define ECL_LLCLOSE		0x82	// 太レーザークローズ(消去＆参照カウント減少)
#define ECL_LLCLOSEL	0x83	// 太レーザーライン状態へ
#define ECL_LLDEGR		0x84	// 太レーザー角度相対変更
#define ECL_HLASER		0x85	// ホーミングレーザー発動！！


// 0x9? : フラグセット用コマンド //
#define ECL_CMD9		10		//
#define ECL_DRAW_ON		0x90	// 描画する
#define ECL_DRAW_OFF	0x91	// 描画しない
#define ECL_CLIP_ON		0x92	// 画面外に出ても消さない
#define ECL_CLIP_OFF	0x93	// 画面外に出たら消す
#define ECL_DAMAGE_ON	0x94	// 無敵にする
#define ECL_DAMAGE_OFF	0x95	// 無敵にしない
#define ECL_HITSB_ON	0x96	// 自機に当たる
#define ECL_HITSB_OFF	0x97	// 自機に当たらない
#define ECL_RLCHG_ON	0x98	// 左右反転を有効にする
#define ECL_RLCHG_OFF	0x99	// 左右反転を無効にする


// 0xA? : 特殊コマンド //
#define ECL_CMDA		16		//
#define ECL_ANM			0xA0	// アニメーションを変更する
#define ECL_PSE			0xA1	// 効果音を鳴らす
#define ECL_INT			0xA2	// ボス用割り込みを発生させる...
#define ECL_EXDEGD		0xA3	// 特殊角セット初期化
#define ECL_ENEMYSET	0xA4	// 敵を雑魚指定でセットする
#define ECL_ENEMYSETD	0xA5	// 敵セット(角度指定有り)
#define ECL_HITXY		0xA6	// 敵の当たり判定を変更する
#define ECL_ITEM		0xA7	// アイテムの種類をセットする
#define ECL_STG4EFC		0xA8	// ４面ボス用同期エフェクト管理
#define ECL_ANMEX		0xA9	// ダメージ中のアニメーションを設定
#define ECL_BITLASER	0xAA	// ビットによるレーザーコマンド指定
#define ECL_BITATTACK	0xAB	// ビットによる攻撃指定
#define ECL_BITCMD		0xAC	// ビットコマンド送信
#define ECL_BOSSSET		0xAD	// ボスを発生させる
#define ECL_CEFC		0xAE	// 円エフェクトを発生させる
#define ECL_STG3EFC		0xAF	// ３面星エフェクト発動


// 0xB? : レジスタ使用コマンド([80x86命令ちっく]に) //
#define ECL_CMDB		15		//
#define ECL_MOVR		0xB0	// レジスタ<->構造体変数の代入
#define ECL_MOVC		0xB1	// レジスタ<- 定数(即値)の代入
#define ECL_ADD			0xB2	// 加算命令
#define ECL_SUB			0xB3	// 減算命令
#define ECL_SINL		0xB4	// sinl(Gr0,Gr1)
#define ECL_COSL		0xB5	// cosl(Gr0,Gr1)
#define ECL_MOD			0xB6	// Gr0 = Gr0 % Const
#define ECL_RND			0xB7	// Gr0 = rnd()
#define ECL_CMPR		0xB8	// Gr0,Gr1 の比較
#define ECL_CMPC		0xB9	// Gr0,Const の比較
#define ECL_JL			0xBA	// 比較結果が > ならばジャンプ
#define ECL_JS			0xBB	// 比較結果が < ならばジャンプ
#define ECL_INC			0xBC	// レジスタ＋１
#define ECL_DEC			0xBD	// レジスタ－１
#define ECL_JEQ			0xBE	// 比較結果が = ならばジャンプ


// ECL 定数 //

// 割り込み命令は、数字が小さいほど優先順位が高い //
#define ECLVECT_MAX			4		// 割り込みベクタ最大数
#define ECLVECT_BOSSLEFT	0x00	// ボス残り数割り込み
#define ECLVECT_HP			0x01	// 体力が指定値より小さいときに割り込み
#define ECLVECT_TIMER		0x02	// タイマー割り込み
#define ECLVECT_BITLEFT		0x03	// 残りビット数割り込み

#define ECLREG_MAX		8		// レジスタの本数
#define ECLCST_GR0		0		// ０番レジスタ
#define ECLCST_GR1		1		// １番レジスタ
#define ECLCST_GR2		2		// ２番レジスタ
#define ECLCST_GR3		3		// ３番レジスタ
#define ECLCST_GR4		4		// ４番レジスタ
#define ECLCST_GR5		5		// ５番レジスタ
#define ECLCST_GR6		6		// ６番レジスタ
#define ECLCST_GR7		7		// ７番レジスタ

#define ECLCST_LCMD_D	(128+0)		// レーザーコマンド(角度)
#define ECLCST_LCMD_DW	(128+1)		// レーザーコマンド(角度差)
#define ECLCST_LCMD_N	(128+2)		// レーザーコマンド(本数)
#define ECLCST_LCMD_C	(128+3)		// レーザーコマンド(色)
#define ECLCST_LCMD_L	(128+4)		// レーザーコマンド(長さ)
#define ECLCST_LCMD_V	(128+5)		// レーザーコマンド(速度)

#define ECLCST_TCMD_D	(128+6)		// 弾コマンド(角度)
#define ECLCST_TCMD_DW	(128+7)		// 弾コマンド(角度差)
#define ECLCST_TCMD_N	(128+8)		// 弾コマンド(個数)
#define ECLCST_TCMD_NS	(128+9)		// 弾コマンド(連射数)
#define ECLCST_TCMD_V	(128+10)	// 弾コマンド(速度)
#define ECLCST_TCMD_C	(128+11)	// 弾コマンド(色)
#define ECLCST_TCMD_A	(128+12)	// 弾コマンド(加速度)
#define ECLCST_TCMD_REP	(128+13)	// 弾コマンド(繰り返し)
#define ECLCST_TCMD_VD	(128+14)	// 弾コマンド(角速度)

#define ECLCST_ENEMY_X	(128+15)	// 敵のＸ座標
#define ECLCST_ENEMY_Y	(128+16)	// 敵のＹ座標
#define ECLCST_ENEMY_D	(128+17)	// 敵の角度

#define ECLCST_LLASERALL	0xff	// 全てのレーザーに適用する場合に指定する数

#define ECLINT_SNAKEON		0x00	// 蛇型セット
#define ECLINT_LBWING01		0x01	// ラスボスの蝶の羽モード
#define ECLINT_LBWING02		0x02	// ラスボスの鳥の羽モード
#define ECLINT_BITON5		0x03	// ビット装着(５つ)
#define ECLINT_BITON6		0x04	// ビット装着(６つ)
#define ECLINT_SHILD1		0x05	// ボム回避１
#define ECLINT_SHILD2		0x06	// ボム回避１


#endif

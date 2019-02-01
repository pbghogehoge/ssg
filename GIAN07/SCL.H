/*                                                                           */
/*   SCL.h   ＳＣＬ用定義ファイル                                            */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_SCL_H
#define PBGWIN_SCL_H	"SCL : Ver 0.11 : Update 2000/02/28"
//#pragma message(PBGWIN_SCL_H)



///// [更新履歴] /////

// 2000/03/14 : WAITEX,STAGECLEAR 命令を追加
// 2000/02/28 : ＢＯＳＳ命令を変更
// 2000/02/24 : ミディ関連の関数を追加
// 2000/02/18 : システムのアップデート開始


///// 特殊な命令の仕様について /////

// WAITEX <待ち条件(BYTE)>,<オプション(DWORD)>
// 待ち条件の BOSSHP は、主に背景エフェクトチェンジ等に用いること(状態推移には使用しない)


///// [ 定数 ] /////

// SCL 命令 //
#define SCL_TIME		0x00		// 次のイベントの発動時間
#define SCL_ENEMY		0x01		// 敵イベント
#define SCL_SSP			0x02		// スクロールスピードチェンジ
#define SCL_EFC			0x03		// エフェクトセット
#define SCL_END			0x04		// ＳＣＬ終了
#define SCL_BOSS		0x05		// ボス発生(引数は X(16),Y(16),BossID(8))


// SCL レベル２命令 //
#define SCL_MWOPEN			0x06	// メッセージウィンドウを開く
#define SCL_MWCLOSE			0x07	// メッセージウィンドウを閉じる
#define SCL_MSG				0x08	// メッセージを出力する
#define SCL_KEY				0x09	// キー入力待ち
#define SCL_NPG				0x0a	// 新しいページに変更する
#define SCL_FACE			0x0b	// 顔を表示する
#define SCL_MUSIC			0x0c	// 曲データをロードする
#define SCL_BOSSDEAD		0x0d	// ボス強制破壊(すなわち時間切れ)
#define SCL_LOADFACE		0x0e	// 顔グラをロードする(引数は、SurfaceID(BYTE),FileNo(BYTE))
#define SCL_WAITEX			0x0f	// ある条件が起こるまでＳＣＬをストップする
#define SCL_STAGECLEAR		0x10	// そのステージが終了することを意味する。次のステージへGO!
#define SCL_MAPPALETTE		0x11	// パレットをマップパーツ用のもので初期化する(For 8BitMode)
#define SCL_GAMECLEAR		0x12	// タイトルに戻る(ネームレジスト有)
#define SCL_DELENEMY		0x13	// 敵を強制消去(インデックス配列そのものを)する
#define SCL_ENEMYPALETTE	0x14	// 敵のパレットにする
#define SCL_STAFF			0x15	// スタッフＩＤセット
#define SCL_EXTRACLEAR		0x16	// エキストラステージクリア


// EFC 命令の引数 //
#define SEFC_WARN		0x00		// ワーニング音・開始
#define SEFC_WARNSTOP	0x01		// ワーニング音・停止
#define SEFC_MUSICFADE	0x02		// 曲フェードアウト実行(Level2)
#define SEFC_STG2BOSS	0x03		// ステージ２ボスのスクロール発動！！
#define SEFC_RASTERON	0x04		// ラスタースクロール開始(砂漠とか海底都市とかに使えるかも)
#define SEFC_RASTEROFF	0x05		// ラスタースクロール終了
#define SEFC_CFADEIN	0x06		// 円形フェードイン
#define SEFC_CFADEOUT	0x07		// 円形フェードアウト
#define SEFC_STG3BOSS	0x08		// ３面ボス雲
#define SEFC_STG3RESET	0x09		// ３面ボス雲リセット
#define SEFC_STG6CUBE	0x0a		// ６面ボス３Ｄキューヴ
#define SEFC_STG6RNDECL	0x0b		// ６面ボス偽ＥＣＬ羅列
#define SEFC_STG4ROCK	0x0c		// ４面岩
#define SEFC_STG4LEAVE	0x0d		// ４面岩を画面外に吐き出す
#define SEFC_WHITEIN	0x0e		// ホワイトイン
#define SEFC_WHITEOUT	0x0f		// ホワイトアウト
#define SEFC_LOADEX01	0x10		// エキストラボス１用画像をロード
#define SEFC_LOADEX02	0x11		// エキストラボス２用画像をロード
#define SEFC_STG6RASTER	0x12		// ６面ラスター


// WAITEX 命令の引数(Level2) //
#define SWAIT_BOSSLEFT	0x00		// ボスの残り数(OPT:ボスの数)
#define SWAIT_BOSSHP	0x01		// ボスのＨＰ総和が指定値より小さい(OPT:残りＨＰ)



#endif

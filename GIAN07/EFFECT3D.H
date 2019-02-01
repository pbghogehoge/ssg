/*                                                                           */
/*   Effect3D.h   ３Ｄエフェクトの処理                                       */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_EFFECT3D_H
#define PBGWIN_EFFECT3D_H	"EFFECT3D : Ver 0.01 : Update 2000/05/31"
//#pragma message(PBGWIN_EFFECT3D_H)



///// [更新履歴] /////
// 2000/05/31 : 開発開始



///// [ヘッダファイル] /////
#include "PBGUTY_X.h"
#include "Gian.h"



///// [ 定数 ] /////
#define STG4ROCK_STDMOVE	0		// 普通のスクロールね
#define STG4ROCK_ACCMOVE1	1		// 加速有りスクロール(1)
#define STG4ROCK_ACCMOVE2	2		// 加速有りスクロール(2)
#define STG4ROCK_3DMOVE		3		// ３Ｄ回転
#define STG4ROCK_LEAVE		4		// 一時的に岩を消去する
#define STG4ROCK_END		5		// エフェクト終了



///// [ 構造体 ] /////
typedef struct tagPoint3D{
	int		x,y,z;
} Point3D;

typedef struct tagLineList3D{
	int			ox,oy;	/* 頂点の座標の補正用 */
	POINT		*p;		/* 頂点の座標         */
	int			n;		/* 頂点数             */
	POINT		*Work;	/* ワークエリア       */

	BYTE		DegX,DegY,DegZ;		/* 各軸に対する回転角 */
} LineList3D;

typedef struct tagWireFrame3D{
	LineList3D	*LList;
	int			n;
} WireFrame3D;

typedef struct tagTriangle3D{
	Point3D	Current;
	Point3D	Speed;

	POINT	Grp[3];

	BYTE	deg_x,deg_y,deg_z;
	char	roll_x,roll_y,roll_z;
} Triangle3D;

typedef struct tagCircle3D{
	int		ox,oy;
	int		r;
	BYTE	deg;
	BYTE	n;
} Circle3D;

typedef struct tagDeg3D{
	int		dx;
	int		dy;
	int		dz;
} Deg3D;

typedef struct tagCube3D{
	Point3D		p;
	Deg3D		d;
	int			l;
} Cube3D;

typedef struct tagStar2D {
	int		x,y;
	int		vy;
} Star2D;

// 雲管理用構造体 //
typedef struct tagCloud2D {
	int		x, y;		// ｘ６４座標
	int		vy;			// 速度のｙ成分(ｙしかないけど)
	BYTE	type;		// 雲の種類
} Cloud2D;

// 非・汎用２Ｄ正方形ワイヤーフレーム //
typedef struct tagWFLine2D {
	int		ox,oy;		// 中心座標
	int		w;			// 正方形の一辺の長さ
	BYTE	d;			// 正方形の傾き角度
} WFLine2D;

// 偽ＥＣＬ羅列管理用構造体 //
typedef struct tagFakeECLString {
	int		SrcX, SrcY;		// 元画像の基準となるＸ＆Ｙ座標
	int		x,y;			// 現在の座標x64
	int		vx,vy;			// 現在の速度成分x64
} FakeECLString;

// 岩管理用構造体 //
typedef struct tagRock3D {
	int		x, y, z;		// 現在の座標
	int		vx, vy;			// 速度XY成分(2D-平面上)

	DWORD	count;			// カウンタ
	int		v;				// 速度

	char	a;				// 加速度
	BYTE	d;				// 角度(2D-平面上)
	BYTE	GrpID;			// グラフィックＩＤ
	BYTE	State;			// 現在の状態
} Rock3D;


///// [ 関数 ] /////
FVOID InitLineList3D(WireFrame3D *w);
FVOID MoveLineList3D(WireFrame3D *w);
FVOID DrawLineList3D(WireFrame3D *w);

FVOID InitWarning(void);
FVOID DrawWarning(void);
FVOID MoveWarning(BYTE count);
FVOID MoveWarningR(char count);

FVOID Init3DCube(void);
FVOID Draw3DCube(void);
FVOID Move3DCube(void);

FVOID InitTriangle(void);
FVOID MoveTriangle(void);
FVOID DrawTriangle(void);

FVOID InitCircle(void);
FVOID MoveCircle(void);
FVOID DrawCircle(void);
/*
FVOID InitStg3Cloud(void);		// ３面の雲の初期化を行う
FVOID MoveStg3Cloud(void);		// ３面の雲を動作させる
FVOID DrawStg3Cloud(void);		// ３面の雲を描画する
*/
FVOID InitEffectFakeECL(void);
FVOID MoveEffectFakeECL(void);
FVOID DrawEffectFakeECL(void);

FVOID InitStg4Rock(void);						// ステージ４の背景となる岩の集団
FVOID MoveStg4Rock(void);						//
FVOID DrawStg4Rock(void);						//
FVOID SendCmdStg4Rock(BYTE Cmd, BYTE Param);	// コマンド送信

FVOID InitStg6Raster();		// ６面ラスター初期化
FVOID MoveStg6Raster();		// ６面ラスター動作
FVOID DrawStg6Raster();		// ６面ラスター描画

FVOID InitStg3Star();		// ３面高速星初期化
FVOID MoveStg3Star();		// ３面高速星動作
FVOID DrawStg3Star();		// ３面高速星描画




__inline void Transform3D(Point3D *p, BYTE dx, BYTE dy, BYTE dz)
{
	static Point3D	temp;

	temp.y = p->y;
	temp.z = p->z;
	p->y = cosl(dx,temp.y) - sinl(dx,temp.z);
	p->z = sinl(dx,temp.y) + cosl(dx,temp.z);

	temp.x = p->x;
	temp.z = p->z;
	p->x =   cosl(dy,temp.x) + sinl(dy,temp.z);
	p->z = - sinl(dy,temp.x) + cosl(dy,temp.z);

	temp.x = p->x;
	temp.y = p->y;
	p->x = cosl(dz,temp.x) - sinl(dz,temp.y);
	p->y = sinl(dz,temp.x) + cosl(dz,temp.y);
}

__inline void ShiftRight6Bit(Point3D *o, Point3D *p)
{
	p->x = ((p->x+o->x)>>6) + 320;
	p->y = ((p->y+o->y)>>6) + 240;
}


#endif

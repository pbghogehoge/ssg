/*                                                                           */
/*   Effect3D.cpp   ３Ｄエフェクトの処理                                     */
/*                                                                           */
/*                                                                           */

#include "Effect3D.h"

#define TRIANGLE_MAX		50
#define CIRCLE_MAX			40
#define CUBE_MAX			8
#define STAR_MAX			40
#define FAKE_ECLSTR_MAX		80
#define ROCK_MAX			28
//#define CLOUD_MAX			10


Triangle3D	Tri[TRIANGLE_MAX];
Circle3D	Cir[CIRCLE_MAX];
Cube3D		Cube[CUBE_MAX];
Star2D		Star[STAR_MAX];
Rock3D		Rock[ROCK_MAX];
//Cloud2D		Cloud[CLOUD_MAX];

WFLine2D		WFLine;
FakeECLString	FakeECLStr[FAKE_ECLSTR_MAX];


POINT PList_W[11] = {
	{0,15},{15,66},{32,47},{48,66},
	{63,14},{52,11},{42,38},{32,26},
	{21,38},{11,10},{0,15}
};

POINT PList_A1[8] = {
	{96,12},{66,61},{75,67},{83,56},
	{107,56},{115,67},{125,61},{96,12},
};

POINT PList_A2[4] = {
	{96,34},{90,44},{101,44},{96,34}
};

POINT PList_R[15-1] = {
	{132,14},{132,64},{145,64},{145,27},
	{164,27},

	//{150,41},{150,43},
	{150,42},

	{171,66},
	{173,66},{181,57},{167,43},{180,29},
	{180,27},{170,14},{132,14}
};

POINT PList_N1[9] = {
	{189,12},{189,64},{201,64},{201,40},
	{239,66},{239,14},{227,14},{227,38},
	{189,12}
};

POINT PList_N2[9] = {
	{189,12},{189,64},{201,64},{201,40},
	{239,66},{239,14},{227,14},{227,38},
	{189,12}
};

POINT PList_I[5] = {
	{248,14},{248,64},{262,64},{262,14},
	{248,14},
};

POINT PList_G[17] = {
	{354,11},{328,22},{328,57},{354,68},
	{380,59},{380,34},{355,34},{354,45},
	{367,46},{367,51},{355,55},{342,50},
	{342,29},{354,24},{372,30},{377,19},
	{354,11}
};

POINT PWork_W[11],PWork_A1[8],PWork_A2[4],PWork_R[15-1],PWork_N1[9],PWork_N2[9],PWork_I[5],PWork_G[17];

/*
LineList3D	LList_G = {354,39,PList_G,17,PWork_G};
LineList3D	LList_I = {255,39,PList_I,5,PWork_I};
LineList3D	LList_N = {215,39,PList_N,9,PWork_N};
LineList3D	LList_R = {156,39,PList_R,15,PWork_R};
LineList3D	LList_A2 = {96,39,PList_A2,4,PWork_A2};
LineList3D	LList_A1 = {96,39,PList_A1,8,PWork_A1};
LineList3D	LList_W = {32,39,PList_W,11,PWork_W};
*/

LineList3D LList_Warning[8] = {
	{192,39,PList_W,11,PWork_W},
	{192,39,PList_A1,8,PWork_A1},
	{192,39,PList_A2,4,PWork_A2},
	{192,39,PList_R,15-1,PWork_R},
	{192,39,PList_N1,9,PWork_N1},
	{192,39,PList_I,5,PWork_I},
	{192-(296-215),39,PList_N2,9,PWork_N2},
	{192,39,PList_G,17,PWork_G}
};

WireFrame3D Warning = {LList_Warning,8};


static FVOID RollPoint(Point3D *p,BYTE dx,BYTE dy,BYTE dz);
static FVOID __Draw3DCube(Cube3D *c);		// 汎用３Ｄキューブ描画



FVOID InitWarning(void)
{
	static BOOL		bInitialized = FALSE;

	if(bInitialized) return;

	InitLineList3D(&Warning);
	bInitialized = TRUE;
}

FVOID DrawWarning(void)
{
	RECT src = {0,152+16,384,232+16};
	extern GRP GrTama;
	int	st,det;
	static int count;

	count+=8;

	if(Warning.LList[0].DegX==0){
		GrpLock();
		GrpSetAlpha((BYTE)(128+sinl(count,48)),ALPHA_NORM);
		GrpSetColor(5,0,0);
		GrpBoxA(129,40+6,128+384,60+6);
		GrpBoxA(129,60+(232-152)-4,128+384,80+(232-152)-4);
		GrpUnlock();
		GrpBlt(&src,128+1,60+1,GrTama);
	}
	else{
		GrpLock();

		if(Warning.LList[0].DegX<10){
			st=0;
			det=0;
		}
		else if(Warning.LList[0].DegX<20){
			st=-4;
			det=1;
		}
		else{
			//if(Warning.LList[0].DegX<40){
			st  = -8;
			det = 2;
		}

		GrpSetColor(1,1,5);
		MoveWarningR(st);
		DrawLineList3D(&Warning);
		GrpSetColor(2,2,5);
		MoveWarningR(det);
		DrawLineList3D(&Warning);
		GrpSetColor(3,3,5);
		MoveWarningR(det);
		DrawLineList3D(&Warning);
		GrpSetColor(4,4,5);
		MoveWarningR(det);
		DrawLineList3D(&Warning);
		GrpSetColor(5,5,5);
		MoveWarningR(det);
		DrawLineList3D(&Warning);

		GrpUnlock();
	}
}

FVOID MoveWarningR(char count)
{
	int		i;

	if(count==0) return;

	for(i=0; i<Warning.n; i++){
		Warning.LList[i].DegX += count*2;
		Warning.LList[i].DegY += count*1;
		Warning.LList[i].DegZ += count*4;
	}

	MoveLineList3D(&Warning);
}

FVOID MoveWarning(BYTE count)
{
	int		i;

	for(i=0; i<Warning.n; i++){
		Warning.LList[i].DegX = (count<64) ? (64-count)*2 : 0;
		Warning.LList[i].DegY = (count<64) ? (64-count)*1 : 0;
		Warning.LList[i].DegZ = (count<64) ? (64-count)*4 : 0;
	}

	MoveLineList3D(&Warning);
}

FVOID InitLineList3D(WireFrame3D *w)
{
	int		i,j;

	/* Convert x64 */
	for(j=0; j<(w->n); j++){
		for(i=0; i<(w->LList[j].n); i++){
			w->LList[j].p[i].x = (w->LList[j].p[i].x - w->LList[j].ox) * 64;
			w->LList[j].p[i].y = (w->LList[j].p[i].y - w->LList[j].oy) * 64;
		}
	}
}

FVOID MoveLineList3D(WireFrame3D *w)
{
	Point3D		temp;
	int			i,j;
	//static BYTE		count = 0;
	//BOOL	bEnableRoll = FALSE;

	//if(count) count--;
	//else bEnableRoll = TRUE;

	for(j=0; j<(w->n); j++){
/*
		if(bEnableRoll){
			w->LList[j].DegX+=2;
			w->LList[j].DegY+=1;
			w->LList[j].DegZ+=4;
			if(w->LList[j].DegX==0) count = 64;
		}
*/
		for(i=0; i<(w->LList[j].n); i++){
			temp.x = w->LList[j].p[i].x;
			temp.y = w->LList[j].p[i].y;
			temp.z = 0;
			RollPoint(&temp,w->LList[j].DegX,w->LList[j].DegY,w->LList[j].DegZ);

			w->LList[j].Work[i].x = temp.x;
			w->LList[j].Work[i].y = temp.y;
		}
	}
}

FVOID DrawLineList3D(WireFrame3D *w)
{
	int		i,j;
	int		x1,y1,x2,y2;

	for(j=0; j<(w->n); j++){
		for(i=0; i<(w->LList[j].n)-1; i++){
			x1 = (w->LList[j].Work[ i ].x >> 6) + 320;
			y1 = (w->LList[j].Work[ i ].y >> 6) + 100;
			x2 = (w->LList[j].Work[i+1].x >> 6) + 320;
			y2 = (w->LList[j].Work[i+1].y >> 6) + 100;
			GrpLine(x1,y1,x2,y2);
		}
	}
}


FVOID InitTriangle(void)
{
	int		i;

	for(i=0; i<TRIANGLE_MAX; i++){
/*
		Tri[i].Current.z = 64 + i*90*64/TRIANGLE_MAX;
		Tri[i].Current.x = (sinl(i*512/TRIANGLE_MAX,Tri[i].Current.z/32)+320)*64;
		Tri[i].Current.y = (cosl(i*512/TRIANGLE_MAX,Tri[i].Current.z/32)+240)*64;
*/
		Tri[i].Current.x = (rnd()%640)*64;
		Tri[i].Current.y = (rnd()%480)*64;
		Tri[i].Current.z = (rnd()%50+10)*64;

		Tri[i].Speed.x = 0;
		Tri[i].Speed.y = 0;
		Tri[i].Speed.z = 0;

		Tri[i].deg_x = rnd()%256;
		Tri[i].deg_y = rnd()%256;
		Tri[i].deg_z = rnd()%256;

		Tri[i].roll_x = rnd()%6 + 1;
		Tri[i].roll_y = rnd()%6 + 1;
		Tri[i].roll_z = rnd()%6 + 1;
	}

	//return;

	for(i=TRIANGLE_MAX/2; i<TRIANGLE_MAX; i++){
		Tri[i].Current.x = Tri[i-TRIANGLE_MAX/2].Current.x;
		Tri[i].Current.y = Tri[i-TRIANGLE_MAX/2].Current.y;
		Tri[i].Current.z = Tri[i-TRIANGLE_MAX/2].Current.z;

		Tri[i].Speed.x = 0;
		Tri[i].Speed.y = 0;
		Tri[i].Speed.z = 0;

		Tri[i].deg_x = Tri[i-TRIANGLE_MAX/2].deg_x;
		Tri[i].deg_y = Tri[i-TRIANGLE_MAX/2].deg_y;
		Tri[i].deg_z = Tri[i-TRIANGLE_MAX/2].deg_z+128;

		Tri[i].roll_x = Tri[i-TRIANGLE_MAX/2].roll_x;
		Tri[i].roll_y = Tri[i-TRIANGLE_MAX/2].roll_y;
		Tri[i].roll_z = Tri[i-TRIANGLE_MAX/2].roll_z;
	}
}

FVOID MoveTriangle(void)
{
	int		i,j;
	Point3D	temp;

	for(i=0; i<TRIANGLE_MAX; i++){
		Tri[i].deg_x += Tri[i].roll_x;
		Tri[i].deg_y += Tri[i].roll_y;
		Tri[i].deg_z += Tri[i].roll_z;

		for(j=0; j<3; j++){
			temp.x = cosl(j*256/3,Tri[i].Current.z);
			temp.y = sinl(j*256/3,Tri[i].Current.z);
			temp.z = 0;
			RollPoint(&temp,Tri[i].deg_x,Tri[i].deg_y,Tri[i].deg_z);
			Tri[i].Grp[j].x = (temp.x + Tri[i].Current.x)>>6;
			Tri[i].Grp[j].y = (temp.y + Tri[i].Current.y)>>6;
		}
	}
}

FVOID DrawTriangle(void)
{
	int		i;
	POINT	*p;

	GrpLock();

	for(i=0; i<TRIANGLE_MAX; i++){
		p = Tri[i].Grp;
		GrpSetColor(2,2,5);
		GrpLine(p[0].x,p[0].y,p[1].x,p[1].y);
		GrpSetColor(3,3,5);
		GrpLine(p[1].x,p[1].y,p[2].x,p[2].y);
		GrpSetColor(4,4,5);
		GrpLine(p[2].x,p[2].y,p[0].x,p[0].y);
	}

	GrpUnlock();
}

static FVOID RollPoint(Point3D *p,BYTE dx,BYTE dy,BYTE dz)
{
	Point3D		temp;

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


FVOID InitCircle(void)
{
	int		i;

	for(i=0; i<CIRCLE_MAX; i++){
		Cir[i].deg = rnd()%256;
		Cir[i].n   = 16;
		Cir[i].ox  = (rnd()%640)*64;
		Cir[i].oy  = (rnd()%480)*64;
		Cir[i].r   = (rnd()%30+30)*64;
	}
}

FVOID MoveCircle(void)
{
	int		i;

	for(i=0; i<CIRCLE_MAX; i++){
		Cir[i].deg += 3;
	}
}

FVOID DrawCircle(void)
{
	int		i,j;
	int		x,y,l;
	BYTE	deg;

	GrpLock();

	for(i=0; i<CIRCLE_MAX; i++){
		for(j=0; j<Cir[i].n; j++){
			deg = Cir[i].deg+j*256/Cir[i].n;
			if(deg<128) continue;
			x = (Cir[i].ox + cosl(deg,Cir[i].r))>>6;
			y = (Cir[i].oy + sinl(deg,Cir[i].r))>>6;
			l = j*3+(Cir[i].r>>7);

			GrpSetColor(4,4,5);
			GrpLine(x,y,x,y-l);
			y-=l;l -= l/4;
			GrpSetColor(3,3,5);
			GrpLine(x,y,x,y-l);
			y-=l;l -= l/4;
			GrpSetColor(2,2,5);
			GrpLine(x,y,x,y-l);
		}
	}
	for(i=0; i<CIRCLE_MAX; i++){
		for(j=0; j<Cir[i].n; j++){
			deg = Cir[i].deg+j*256/Cir[i].n;
			if(deg>128) continue;
			x = (Cir[i].ox + cosl(deg,Cir[i].r))>>6;
			y = (Cir[i].oy + sinl(deg,Cir[i].r))>>6;
			l = j*3+(Cir[i].r>>7);

			GrpSetColor(4,4,5);
			GrpLine(x,y,x,y-l);
			y-=l;l -= l/4;
			GrpSetColor(3,3,5);
			GrpLine(x,y,x,y-l);
			y-=l;l -= l/4;
			GrpSetColor(2,2,5);
			GrpLine(x,y,x,y-l);
		}
	}

	GrpUnlock();
}


FVOID Init3DCube(void)
{
	int		i;

	for(i=0; i<CUBE_MAX; i++){
		Cube[i].l = 30*64;
		Cube[i].d.dx = rnd();
		Cube[i].d.dy = rnd();
		Cube[i].d.dz = rnd();
		Cube[i].p.x = cosl(i*256/CUBE_MAX, 200*64);
		Cube[i].p.y = sinl(i*256/CUBE_MAX, 200*64);
		Cube[i].p.z = 0;
	}

	for(i=0; i<STAR_MAX; i++){
		Star[i].x = (rnd()%(640-256))+128;
		Star[i].y = -(rnd()%480);
		Star[i].vy = rnd()%10 + 10;
	}
}


FVOID Draw3DCube(void)
{
	int		i;
	RECT	rc;

	for(i=0; i<STAR_MAX; i++){
		BltSetRect(&rc, 136, 272, 16, 24);
		GrpBlt(&rc,Star[i].x,Star[i].y,GrTama);
	}

	GrpLock();
	for(i=0; i<CUBE_MAX; i++) __Draw3DCube(Cube+i);
	GrpUnlock();
}


FVOID Move3DCube(void)
{
	int				i;
	int				l,d2;
	static WORD		d;
	static WORD		dx,dy,dz;

	d+=64*4;

	dx+=32*4;
	dy-=16*4;

	d2 = sinl(d>>8, 512/CUBE_MAX);
	l = sinl(d>>7, 100*64)+(200-20)*64;


	for(i=0; i<CUBE_MAX; i++){
		Cube[i].l = 15*64 + (l>>4) + i*128;
		Cube[i].d.dx+=4;
		Cube[i].d.dy-=4;
		//Cube[i].p.x = cosl(i*256/CUBE_MAX+d2, l);
		//Cube[i].p.y = sinl(i*256/CUBE_MAX+d2, l);
		Cube[i].p.x = cosl(i*500/CUBE_MAX+d2, l);
		Cube[i].p.y = sinl(i*500/CUBE_MAX+d2, l);
		Cube[i].p.z = (i-CUBE_MAX/2)*64*40;
		Transform3D(&Cube[i].p,dx>>8,dy>>8,dz>>8);
	}

	for(i=0; i<STAR_MAX; i++){
		Star[i].y += Star[i].vy;
		if(Star[i].y > 480){
			Star[i].x  = (rnd()%(640-256))+128;
			Star[i].y  = 0;
			Star[i].vy = rnd()%10 + 10;
		}
	}
}


// 汎用３Ｄキューブ描画 //
static FVOID __Draw3DCube(Cube3D *c)
{
	// ３Ｄで透過色付きorアルファ比較ポリゴンを使えばもっともっと早いのだが... //
	// ８ビット対応のため、まぁ仕方が無いか... //

	BYTE		dx,dy,dz;
	int			x,y,z;
	int			l,l2;
	Point3D		p1,p2;
	Point3D		o;

	o  = c->p;
	l  = c->l;
	dx = c->d.dx;
	dy = c->d.dy;
	dz = c->d.dz;

	l2  = l;

	//GrpSetColor(1,1,2);
	GrpSetColor(1,1,3);
	for(x=-1; x<=1; x++){
		for(y=-1; y<=1; y++){
			p1.x = x * l;
			p1.y = y * l;
			p1.z =    -l2;
			Transform3D(&p1, dx, dy, dz);
			ShiftRight6Bit(&o,&p1);

			p2.x = x * l;
			p2.y = y * l;
			p2.z =     l2;
			Transform3D(&p2, dx, dy, dz);
			ShiftRight6Bit(&o,&p2);

			GrpLine(p1.x, p1.y, p2.x, p2.y);
		}
	}

	GrpSetColor(0,0,3);
	for(y=-1; y<=1; y++){
		for(z=-1; z<=1; z++){
			p1.x =    -l2;
			p1.y = y * l;
			p1.z = z * l;
			Transform3D(&p1, dx, dy, dz);
			ShiftRight6Bit(&o,&p1);

			p2.x =     l2;
			p2.y = y * l;
			p2.z = z * l;
			Transform3D(&p2, dx, dy, dz);
			ShiftRight6Bit(&o,&p2);

			GrpLine(p1.x, p1.y, p2.x, p2.y);
		}
	}

	//GrpSetColor(1,1,3);
	GrpSetColor(1,1,4);
	for(x=-1; x<=1; x++){
		for(z=-1; z<=1; z++){
			p1.x = x * l;
			p1.y =    -l2;
			p1.z = z * l;
			Transform3D(&p1, dx, dy, dz);
			ShiftRight6Bit(&o,&p1);

			p2.x = x * l;
			p2.y =     l2;
			p2.z = z * l;
			Transform3D(&p2, dx, dy, dz);
			ShiftRight6Bit(&o,&p2);

			GrpLine(p1.x, p1.y, p2.x, p2.y);
		}
	}
}


FVOID InitEffectFakeECL(void)
{
	int		i,v;
	BYTE	d;

	WFLine.d  = 0;
	WFLine.ox = 640*64/2;
	WFLine.oy = 480*64/2;
	WFLine.w  = 30;

	for(i=0; i<FAKE_ECLSTR_MAX; i++){
		d = rnd()%128;
		v = rnd()%(64*5) + 64*5;

		FakeECLStr[i].SrcX = (rnd()% 7) *  9 * 8;
		FakeECLStr[i].SrcY = (rnd()%16) * 16;
		FakeECLStr[i].x    = (28+rnd()%484)*64;
		FakeECLStr[i].y    = -(rnd()%640)*64;
		FakeECLStr[i].vx   = 0;//cosl(d, v);
		FakeECLStr[i].vy   = v;//sinl(d, v);
	}
}

FVOID MoveEffectFakeECL(void)
{
	int		i,v;
	BYTE	d;

	WFLine.ox = (WFLine.ox+1)%64;
	WFLine.oy = (WFLine.oy+62)%64;

	for(i=0; i<FAKE_ECLSTR_MAX; i++){
		FakeECLStr[i].x += FakeECLStr[i].vx;
		FakeECLStr[i].y += FakeECLStr[i].vy;

		if(FakeECLStr[i].y >= 480*64){
			d = rnd()%128;
			v = rnd()%(64*5) + 64*5;

			FakeECLStr[i].SrcX = (rnd()% 7) *  9 * 8;
			FakeECLStr[i].SrcY = (rnd()%16) * 16;
			FakeECLStr[i].x    = (28+rnd()%484)*64;
			FakeECLStr[i].y    = -(rnd()%640)*64;
			FakeECLStr[i].vx   = 0;//cosl(d, v);
			FakeECLStr[i].vy   = v;//sinl(d, v);
		}
	}
}

FVOID DrawEffectFakeECL(void)
{
	RECT	src;
	int		i,j;

	GrpLock();

//	GrpSetColor(3,2,0);		// 後半戦用
	GrpSetColor(0,2,0);
//	GrpSetColor(0,0,3);

	for(i=128-WFLine.ox/2; i<640-128; i+=32)
		GrpLine(i,0,i,480);

	for(j=WFLine.oy/2; j<480; j+=32)
		GrpLine(128,j,640-128,j);


//	GrpSetColor(5,3,0);		// 後半戦用
	GrpSetColor(0,3,0);
//	GrpSetColor(0,0,4);

	for(i=128-WFLine.ox; i<640-128; i+=64)
		GrpLine(i,0,i,480);

	for(j=-WFLine.oy; j<480; j+=64)
		GrpLine(128,j,640-128,j);

	GrpUnlock();

	for(i=0; i<FAKE_ECLSTR_MAX; i++){
		BltSetRect(&src, FakeECLStr[i].SrcX, FakeECLStr[i].SrcY, 72, 16);
		GrpBlt(&src, FakeECLStr[i].x>>6, FakeECLStr[i].y>>6, GrMap);
	}

	SetRect(&src, 0, 272, 416, 352);
	GrpBlt(&src, 128, 400, GrMap);
}

/*
// ３面の雲の初期化を行う //
FVOID InitStg3Cloud(void)
{
	Cloud2D		*p;		//Cloud[CLOUD_MAX];
	int			i;

	p = Cloud;

	for(i=0; i<CLOUD_MAX; i++, p++){
		if(rnd()&1) p->x = 128*64+(rnd()>>1)%(100*64);
		else        p->x = 512*64-(rnd()>>1)%(100*64);
		p->y    = ((i*680*64)/CLOUD_MAX)-200*64;	//GY_RND;

		p->type = (rnd()>>2)%5;
		if(p->type == 2) p->type = 5;

		p->vy   = rnd()%(64 * 6) + 64 * 11;
	}
}


// ３面の雲を動作させる //
FVOID MoveStg3Cloud(void)
{
	Cloud2D		*p;		//Cloud[CLOUD_MAX];
	int			i;

	p = Cloud;

	for(i=0; i<CLOUD_MAX; i++, p++){
		p->y += p->vy;

		if(p->y > (480+200)*64){
			if(rnd()&1) p->x = 128*64+(rnd()>>1)%(100*64);
			else        p->x = 512*64-(rnd()>>1)%(100*64);
			p->y    = -200*64;

			p->type = (rnd()>>2)%5;
			if(p->type == 2) p->type = 5;

			p->vy   = rnd()%(64 * 6) + 64 * 11;
		}
	}
}


// ３面の雲を描画する //
FVOID DrawStg3Cloud(void)
{
#define _RsetMacro(x, y, w, h)		{(x), (y), ((x)+(w)), ((y)+(h))}
	static RECT Data[6] = {
		_RsetMacro(  0, 288, 144, 160),			// Large_1
		_RsetMacro(144, 288, 144, 112),			// Large_2
		_RsetMacro(288, 288, 144, 176),			// Large_3

		//_RsetMacro(480, 288,  32,  48),			// Small_1
		_RsetMacro(144, 400,  32,  48),			// Small_2
		_RsetMacro(176, 400,  48,  32),			// Small_3
		_RsetMacro(224, 400,  48,  48),			// Small_4
	};
#undef _RsetMacro

	static POINT	Size[6] = {
		144/2, 160/2, 144/2, 112/2, 144/2, 176/2,		// Large
		 //32,  48,
		 32/2,  48/2,  48/2,  32/2,  48/2,  48/2		// Small
	};

	RECT		src;
	Cloud2D		*p;		//Cloud[CLOUD_MAX];
	int			i, j;
	int			x, y;

	p = Cloud;

	for(i=0; i<CLOUD_MAX; i++, p++){
		j   = p->type;
		x   = (p->x >> 6) - Size[j].x;
		y   = (p->y >> 6) - Size[j].y;
		src = Data[j];

		GrpBlt(&src, x, y, GrEnemy);
	}
}
*/

FVOID InitStg4Rock(void)
{
	int		i;
	int		id = 2;
	int		y  = 0;
	int		dy = 500*64/4;
	int		dy2 = dy/2;

	for(i=0; i<ROCK_MAX; i++){
		y = (i%4)*dy + (rnd()%dy2);
			//((380*64/16) * (i%(ROCK_MAX/16+1))) + rnd()%(380*64/16);

		Rock[i].x = (rnd()%(500*64)-250*64);		//
		Rock[i].y = -250*64-y;						// 上の方なのだ
		Rock[i].z = (rnd()%(500*64)-250*64);		//

		if(i == ROCK_MAX*5/8) id--;
		if(i == ROCK_MAX*7/8) id--;

		Rock[i].GrpID = id;

		Rock[i].vx    = 0;
		Rock[i].vy    = (4-Rock[i].GrpID)*16;
		Rock[i].v     = Rock[i].vy;

		Rock[i].count = 0;
		Rock[i].a     = 0;
		Rock[i].d     = 64;
		Rock[i].State = STG4ROCK_STDMOVE;
	}
}


FVOID MoveStg4Rock(void)
{
	int		i, y;
	int		dy = 500*64/4;
	int		dy2 = dy/2;
	Rock3D	*p = Rock;

	for(i=0; i<ROCK_MAX; i++, p++){
		p->count++;

		switch(p->State){
		case(STG4ROCK_STDMOVE):
			p->y += p->vy;
			if(p->y > (250+40)*64){
				p->x     = (rnd()%(500*64)-250*64);
				p->y     = (-250-40-rnd()%250)*64;
				p->vy    = (4-Rock[i].GrpID)*16;
				p->v     = p->vy;
			}
		break;

		case(STG4ROCK_ACCMOVE1):
			p->v += p->a;
			p->vy = p->v;
			p->y += p->vy;

			if(p->y > (250+40)*64){
				p->x     = (rnd()%(500*64)-250*64);
				p->y     = (-250-40-rnd()%250)*64;
				p->vy    = (4-Rock[i].GrpID)*32 * 3;
				p->v     = p->vy;
				p->a     = 0;
			}
		break;

		case(STG4ROCK_ACCMOVE2):
			p->v -= p->a;
			p->vy = p->v;
			p->y += p->vy;

			if(p->count > 60*2){
				if(p->y > (250+40)*64 || p->y < (-250-40)*64){
					p->x     = (rnd()%(500*64)-250*64);
					p->y     = (-250-40-rnd()%250)*64;
					p->vy    = (4-Rock[i].GrpID)*32;
					p->v     = p->vy;
				}

				p->vy    = (4-Rock[i].GrpID)*32;
				p->a     = 2;
				p->v     = p->vy;
				p->State = STG4ROCK_ACCMOVE1;

				break;
			}

			if(p->y > (250+40)*64 || p->y < (-250-40)*64){
				p->x     = (rnd()%(500*64)-250*64);
				p->y     = (250+40)*64+rnd()%250;
				p->vy    = -(4-Rock[i].GrpID)*32 * 3;
				p->v     = p->vy;
				p->a     = 0;
			}

/*			p->v += p->a;
			p->x += cosl(p->d, p->v);
			p->y += (p->vy + sinl(p->d, p->v));

			if(p->count > 60){
				if(p->y > (250+40)*64 || p->y < (-250-40)*64){
					//p->x     = (rnd()%(500*64)-250*64);
					y = (i%4)*dy + (rnd()%dy2);
					p->x = (rnd()%(500*64)-250*64);		//
					p->y = -250*64-y;						// 上の方なのだ

					p->vy    = (4-Rock[i].GrpID)*32 * 3;
					p->v     = p->vy;
					p->a     = 0;
					p->State = STG4ROCK_ACCMOVE1;
				}
				else{
					p->v = p->vy = (4-Rock[i].GrpID)*32 * 3;
					p->a = 0;
					p->State = STG4ROCK_ACCMOVE1;
				}

				break;
			}

			if(p->y > (250+40)*64 || p->y < (-250-40)*64){
				p->x     = (rnd()%(700*64)-350*64);
				p->y     = (250+40)*64;
				p->vy    = (4-Rock[i].GrpID)*32 * 3;
				p->v     = 10;
				p->a     = -4;
			}*/
		break;

		case(STG4ROCK_LEAVE):
			if(p->y > (500+40)*64) break;
			else p->y += p->vy;
		break;

		case(STG4ROCK_END):
			if(p->y > (500+40)*64) break;
			else p->y += (4-Rock[i].GrpID)*32 * 6;
		break;

		default:
		break;
		}
	}
}


FVOID DrawStg4Rock(void)
{
	static RECT src[3] ={{0, 224, 80, 288},{0, 288, 48, 336},{48, 288, 80, 320}};
	static int dx[3] = {80/2, 48/2, 32/2};
	static int dy[3] = {64/2, 48/2, 32/2};

	RECT	temp;
	Rock3D	*p = Rock;

	int		x,y,i;

	for(i=0; i<ROCK_MAX; i++, p++){
		temp = src[p->GrpID];
		x = (p->x + GX_MID)>>6;
		y = (p->y + GY_MID)>>6;
		GrpBlt(&temp, x-dx[p->GrpID], y-dy[p->GrpID], GrMap);
	}
}


FVOID SendCmdStg4Rock(BYTE Cmd, BYTE Param)
{
	int		i;
	Rock3D	*p = Rock;

	switch(Cmd){
	case(STG4ROCK_LEAVE):
		for(i=0; i<ROCK_MAX; i++, p++){
			p->State = STG4ROCK_LEAVE;
		}
	break;

	case(STG4ROCK_END):
		for(i=0; i<ROCK_MAX; i++, p++){
			p->State = STG4ROCK_END;
		}
	break;

	case(STG4ROCK_ACCMOVE1):
		for(i=0; i<ROCK_MAX; i++, p++){
			p->State = STG4ROCK_ACCMOVE1;
			p->a = p->v/24;//(3-p->GrpID)*3;
			p->count = 0;
		}
	break;

	case(STG4ROCK_ACCMOVE2):
		for(i=0; i<ROCK_MAX; i++, p++){
			p->State = STG4ROCK_ACCMOVE2;
			p->a = p->v/12;24;//(3-p->GrpID)*3;
			p->count = 0;
		}
/*		for(i=0; i<ROCK_MAX; i++, p++){
			p->State = STG4ROCK_ACCMOVE2;
			p->a = -4;
			p->count = 0;
			p->v = 10;
			p->d = Param;
		}
*/
	break;

	default:
	break;
	}
}



#define S6RASTER_MAX	28
#define S6STAR_MAX		60
#define S3STAR_MAX		180

typedef struct tagStg6Raster {
	int		x, y;		// 表示座標
	char	vy;			//
	BYTE	type;		// 種類(0-2)
	BYTE	deg;		// 基準角度
	BYTE	amp;		// 振幅
} Stg6Raster;

typedef struct tagStg6Star {
	int		x, y;
	int		vy;
} Stg6Star;

Stg6Raster	S6Ras[S6RASTER_MAX];
Stg6Star	S6Star[S3STAR_MAX];		// 兼用モノなのだ


// ６面ラスター初期化 //
FVOID InitStg6Raster()
{
	int		i;

	for(i=0; i<S6RASTER_MAX; i++){
		S6Ras[i].x = rnd()%(640-256) + 128;
		S6Ras[i].y = -rnd()%(480+160);//(480+240)-240;
		S6Ras[i].deg = rnd();
		S6Ras[i].type = i%3;
		S6Ras[i].amp = rnd()%80 + 70;
		S6Ras[i].vy = 2 + rnd()%3;
	}

	for(i=0; i<S6STAR_MAX; i++){
		S6Star[i].x = rnd()%(640-256) + 128;
		S6Star[i].y = rnd()%480;
		S6Star[i].vy = rnd()%20 + 8;
	}
}


// ６面ラスター動作 //
FVOID MoveStg6Raster()
{
	int		i;

	for(i=0; i<S6RASTER_MAX; i++){
		if(i&1) S6Ras[i].deg += 2;
		else    S6Ras[i].deg -= 2;

		S6Ras[i].y += S6Ras[i].vy;

		if(S6Ras[i].y > 480){
			S6Ras[i].x = rnd()%(640-256) + 128;
			S6Ras[i].y = -160;
			S6Ras[i].deg = rnd();
			S6Ras[i].amp = rnd()%80 + 70;
		}
	}

	for(i=0; i<S6STAR_MAX; i++){
		S6Star[i].y += S6Star[i].vy;

		if(S6Star[i].y > 480){
			S6Star[i].x  = rnd()%(640-256) + 128;
			S6Star[i].y -= 480;
			S6Star[i].vy = rnd()%20 + 8;
		}
	}
}


// ６面ラスター描画 //
FVOID DrawStg6Raster()
{
	static RECT Target[3] = {
		{608, 272, 640, 352}, {592, 160, 640, 272}, {576, 0, 640, 160},
	};

	RECT	src;
	int		i, j, h, w;
	int		x1, x2, dx, oy;

	for(i=0; i<S6STAR_MAX; i++){
		SetRect(&src, 624, 352, 624+16, 352+16);
		GrpBlt(&src, S6Star[i].x, S6Star[i].y, GrMap);
	}

	for(i=0; i<S6RASTER_MAX; i++){
		x1 = Target[S6Ras[i].type].left;
		x2 = Target[S6Ras[i].type].right;
		oy = Target[S6Ras[i].type].top;
		h  = Target[S6Ras[i].type].bottom - Target[S6Ras[i].type].top;
		w  = (x2 - x1)/2;
		for(j=0; j<h; j+=2){
			dx = sinl(S6Ras[i].deg + j, S6Ras[i].amp);
			SetRect(&src, x1, j+oy, x2, j+2);
			GrpBlt(&src, S6Ras[i].x+dx-w, S6Ras[i].y+j, GrMap);
		}
	}
}


// ３面高速星初期化 //
FVOID InitStg3Star()
{
	int		i;

	for(i=0; i<S3STAR_MAX; i++){
		S6Star[i].x = rnd()%(640-256) + 128;
		S6Star[i].y = rnd()%480;
		S6Star[i].vy = rnd()%20 + 8;
	}
}


// ３面高速星動作 //
FVOID MoveStg3Star()
{
	int		i;

	for(i=0; i<S3STAR_MAX; i++){
		S6Star[i].y += S6Star[i].vy;

		if(S6Star[i].y > 480){
			S6Star[i].x  = rnd()%(640-256) + 128;
			S6Star[i].y -= 480;
			S6Star[i].vy = rnd()%20 + 8;
		}
	}
}


// ３面高速星描画 //
FVOID DrawStg3Star()
{
	int		i;
	RECT	src;

	for(i=0; i<S3STAR_MAX; i++){
		SetRect(&src, 640-16, 0, 640, 16);
		GrpBlt(&src, S6Star[i].x, S6Star[i].y, GrMap);
	}
}

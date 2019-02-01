/*************************************************************************************************/
/*   ENEMY.C   敵の管理とか発生制御等                                                            */
/*                                                                                               */
/*************************************************************************************************/

#include "GIAN.H"
#include "ECL_LEN.H"

/*
 * ECLコマンドのアドレス更新には ECL_CmdLen[ECLコマンド定数] を使用する
 *
 */

// 左右反転用マクロ //
#define ABS_DEGRL(d)	((e->flag&EF_RLCHG) ? (128-(d)) : (d ))
#define ABS_VXRL(vx)	((e->flag&EF_RLCHG) ? (-(vx))   : (vx))
#define REL_DEGRL(d)	((e->flag&EF_RLCHG) ? (-(d ))   : (d ))


// ＥＣＬデバッグ用マクロ //
#ifdef _DEBUG
	#define ECL_DEBUG(s,param)				\
	{										\
		char _ECL_Debug[1000];				\
		sprintf(_ECL_Debug,s,param);		\
		GrpPut16(10,10+16*10,_ECL_Debug);	\
	}
#else
	#define ECL_DEBUG(s,param)
#endif


// 変数の実体 //
BYTE		*ECL_Head = NULL;
BYTE		*SCL_Head = NULL;
BYTE		*SCL_Now  = NULL;
ENEMY_DATA	Enemy[ENEMY_MAX];
WORD		EnemyInd[ENEMY_MAX];
WORD		EnemyNow = 0;
ANIME_DATA	Anime[ANIME_MAX];

int			HomingX;		// ホーミング対象のＸ座標
int			HomingY;		// ホーミング対象のＹ座標
int			HomingFlag;		// 真ならホーミング実行

BYTE		EnemyEXDEG;		// 特殊角度の現在値
BYTE		EnemyEXDEG_D;	// 特殊角度の増分


// 関数 //
static void _EnemyDrawBomb(int x,int y,DWORD count);
static void Indsort(WORD *ind,WORD *now);

static DWORD ID2Value(ENEMY_DATA *e,BYTE id);		// ECLCST_?? からその値に変換する



void UpdateHoming(ENEMY_DATA *e)
{
	int temp = Viv.y - e->y;

	if(temp<0) return;

	if(temp<HomingFlag){
		HomingFlag = temp;
		HomingX    = e->x;
		HomingY    = e->y;
	}
}

/*
_inline Debug(DWORD old,int id)
{
	RndBuf[id] += (random_ref-old);
}
*/

void enemy_move(void)
{
	int				i;//,chkx,chky;
	ENEMY_DATA		*e;
	//DWORD temp;

	if(BossNow==0) HomingFlag = HOMING_DUMMY;

	for(i=0;i<EnemyNow;i++){
		e = Enemy+(*(EnemyInd+i));
		e->IsDamaged = 0;
		if(!(e->flag & EF_BOMB)){
			// 通常の敵の処理 //
			CheckECLInterrupt(e);
			parse_ECL(e);

			// 弾発射モードによる分岐 //
			if(e->t_rep && e->hp){
				e->tama_c = (e->tama_c+1)%(e->t_rep);
				if(e->tama_c==0){
					TamaCmd = e->t_cmd;
					TamaCmd.x += e->x;
					TamaCmd.y += e->y;
					tama_set();
				}
			}
			//Debug(temp,22);

			// サボテンヒットチェック //
			if(HITCHK(e->x,Viv.x,e->g_width) && HITCHK(e->y,Viv.y,e->g_height) && Viv.muteki==0){
				// ここら辺で敵にダメージを与えるとおもしろいかも？ //
				if(e->flag&EF_HITSB) MaidDead();
			}

			// 範囲外チェック //
			if((e->y<GY_MIN-(e->g_height))||(e->y>GY_MAX+(e->g_height))||
				(e->x<GX_MIN-(e->g_width))||(e->x>GX_MAX+(e->g_width))){
				if((e->flag&EF_CLIP)==0){
					if(e->LLaserRef) LLaserForceClose(e);
					e->flag = EF_DELETE;
				}
			}
		}
		else if(e->count>=8*ENEMY_BOMB_SPD-1){
			e->flag = EF_DELETE;
		}

		// ホーミングの準備 //
		if((BossNow==0) && (e->flag&EF_DAMAGE)) UpdateHoming(e);

		// アニメーションの動作 //
		EnemyAnimeMove(e);

		e->count++;
	}

	Indsort(EnemyInd,&EnemyNow);
}

void enemy_draw(void)
{
	int			i,x,y;
	ENEMY_DATA	*e;
	//HRESULT		ddrval;
	RECT		*src;

	for(i=0;i<EnemyNow;i++){
		e = Enemy+(*(EnemyInd+i));

		// 敵を描画する(クリッピング＆幅、高さ処理を追加すること) //
		x = (e->x>>6);
		y = (e->y>>6);
		if(e->flag==EF_BOMB){
			_EnemyDrawBomb(x,y,e->count);
			continue;
		}

		if(e->flag&EF_DRAW){
			if((src=Enemy_AnimeClip(&x,&y,e))==NULL) continue;
			GrpBltX(src,x,y,GrEnemy);	// クリッピングなし

			if((e->anm_ptn != e->anm_ptnEx) && e->IsDamaged){
				x = (e->x>>6);
				y = (e->y>>6);
				if((src=Enemy_HitGrpClip(&x,&y,e))==NULL) continue;
				GrpBltX(src,x,y,GrEnemy);	// クリッピングなし
			}
		}
	}
}

// 雑魚を消滅させる //
extern void enemy_clear(void)
{
	int			i;
	ENEMY_DATA	*e;

	for(i=0;i<EnemyNow;i++){
		e = Enemy+(*(EnemyInd+i));
		if(e->flag == EF_BOMB) continue;

		if(e->flag & EF_DRAW){
			e->flag = EF_BOMB;
			e->hp    = 0;
			e->count = 0;
			if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
			SndPlayEX(SOUND_ID_BOMB,e->x,0);
		}
		else{
			// 描画しないタイプの敵の消去の仕方は他の場合と異なり、 //
			// 爆発アニメ・爆発音を再生しない                       //
			e->flag = EF_DELETE;
			e->hp    = 0;
			e->count = 0;
			if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
			// 爆発音の再生は行わない //
		}
	}

	Indsort(EnemyInd,&EnemyNow);
}

void enemyind_set(void)
{
	int i;

	for(i=0;i<ENEMY_MAX;i++){
		//memset(Enemy+i,0,sizeof(ENEMY_DATA));
		EnemyInd[i] = i;
	}

	EnemyNow = 0;
}

// 敵にダメージを与える //
extern BOOL enemy_damage(int x,int y,int damage)
{
	ENEMY_DATA		*e;
	int				i;

	if(BossDamage(x,y,damage)) return TRUE;

	for(i=0;i<EnemyNow;i++){
		e = Enemy+EnemyInd[i];
		if(HITCHK(x,e->x,e->g_width) && HITCHK(y,e->y,e->g_height) && (e->flag&EF_DAMAGE)){
			if(e->flag==EF_BOMB || !(e->flag&EF_DAMAGE)) continue;
			else{
				e->IsDamaged = ((e->count) & 1);
				if(e->hp<=damage){
					SndPlayEX(SOUND_ID_BOMB,e->x,0);
					if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
					PowerUp((BYTE)e->hp);			// パワーアップ
					e->hp    = 0;
					e->count = 0;
					e->flag  = EF_BOMB;
					score_add(e->score);
					if(e->item) ItemSet(e->x,e->y,e->item);
				}
				else{
					SndPlayEX(SOUND_ID_HIT,e->x,0);
					PowerUp(damage);	// ここでもパワーアップ
					e->hp -= damage;
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

// ｙ上方向無限Ver.敵にダメージを与える //
extern BOOL enemy_damage2(int x,int y,int damage)
{
	ENEMY_DATA		*e;
	int				i;
	BOOL			ret_val = FALSE;

	if(BossDamage2(x,y,damage)) ret_val = TRUE;

	for(i=0;i<EnemyNow;i++){
		e = Enemy+EnemyInd[i];
		if(HITCHK(x,e->x,e->g_width) && (y > e->y) && (e->flag&EF_DAMAGE)){
			if(e->flag==EF_BOMB || !(e->flag&EF_DAMAGE)) continue;
			else{
				e->IsDamaged = ((e->count) & 1);
				if(e->hp<=damage){
					SndPlayEX(SOUND_ID_BOMB,e->x,0);
					if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
					PowerUp((BYTE)e->hp);			// パワーアップ
					e->hp    = 0;
					e->count = 0;
					e->flag  = EF_BOMB;
					score_add(e->score);
					if(e->item) ItemSet(e->x,e->y,e->item);
				}
				else{
					SndPlayEX(SOUND_ID_HIT,e->x,0);
					PowerUp(damage);	// ここでもパワーアップ
					e->hp -= damage;
				}

				ret_val = TRUE;
			}
		}
	}

	return ret_val;
}

// ナナメレーザーの当たり判定 //
extern VOID enemy_damage3(int x,int y,BYTE d)
{
	ENEMY_DATA		*e;
	int				i;
	//BOOL			ret_val = FALSE;
	int				damage  = 8;

	BossDamage3(x,y,d);

	for(i=0;i<EnemyNow;i++){
		e = Enemy+EnemyInd[i];
		if(LaserHITCHK(e, x, y, d) && (e->flag&EF_DAMAGE)){
			if(e->flag==EF_BOMB || !(e->flag&EF_DAMAGE)) continue;
			else{
				e->IsDamaged = ((e->count) & 1);
				if(e->hp<=damage){
					SndPlayEX(SOUND_ID_BOMB,e->x,0);
					if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
					PowerUp((BYTE)e->hp);			// パワーアップ
					e->hp    = 0;
					e->count = 0;
					e->flag  = EF_BOMB;
					score_add(e->score);
					if(e->item) ItemSet(e->x,e->y,e->item);
				}
				else{
					SndPlayEX(SOUND_ID_HIT,e->x,0);
					PowerUp(damage);	// ここでもパワーアップ
					e->hp -= damage;
				}
			}
		}
	}
}


// すべての敵にダメージを与える /
extern void enemy_damage4(int damage)
{
	ENEMY_DATA		*e;
	int				i;

	BossDamage4(damage);

	for(i=0;i<EnemyNow;i++){
		e = Enemy+EnemyInd[i];
		if(e->flag&EF_DAMAGE){
			if(e->flag==EF_BOMB || !(e->flag&EF_DAMAGE)) continue;
			else{
				e->IsDamaged = ((e->count) & 1);
				if(e->hp<=damage){
					SndPlayEX(SOUND_ID_BOMB,e->x,0);
					if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
					PowerUp((BYTE)e->hp);			// パワーアップ
					e->hp    = 0;
					e->count = 0;
					e->flag  = EF_BOMB;
					score_add(e->score);
					if(e->item) ItemSet(e->x,e->y,e->item);
				}
				else{
					SndPlayEX(SOUND_ID_HIT,e->x,0);
					PowerUp(damage);	// ここでもパワーアップ
					e->hp -= damage;
				}

				//return TRUE;
			}
		}
	}

	//return FALSE;
}


// 敵データを初期化する(x,y は x64 で指定のこと) //
extern void InitEnemyDataX64(ENEMY_DATA *e,int x,int y,DWORD EclID)
{
	e->x = x;
	e->y = y;

	e->cmd = (*(DWORD *)(&ECL_Head[EclID]));

	e->call_addr = e->cmd;

	e->hp       = 0xffffffff;
	e->amp      = 0;
	e->anm_ptn  = 0;
	e->anm_ptnEx= 0;	// 追加 : 2000/11/27 (ダメージ中のアニメ)
	e->anm_sp   = 0;
	e->anm_c    = 0;
	e->count    = 0;
	e->evscore  = 0;
	e->d        = 64;
	e->flag     = EF_DAMAGE|EF_DRAW|EF_HITSB;

	e->IsDamaged = 0;

	e->tama_c   = (BYTE)rnd();//&0xff;
	e->t_rep    = 0;			// 弾の発射間隔(０：自動発射しない)
	e->g_width  = 0;
	e->g_height = 0;

	e->item     = ITEM_SCORE;

	e->rep_c    = 0;
	e->cmd_c    = 0;
	e->v        = 64;
	e->vd       = 0;
	e->vx       = cosl(e->d,e->v);
	e->vy       = sinl(e->d,e->v);

	e->LLaserRef = 0;

	e->t_cmd.c      = 0;
	e->t_cmd.cmd    = TC_WAY;
	e->t_cmd.d      = 64;
	e->t_cmd.n      = 1;
	e->t_cmd.option = TE_NONE;
	e->t_cmd.type   = T_NORM;
	e->t_cmd.v      = 3;
	e->t_cmd.x      = 0;
	e->t_cmd.y      = 0;

	e->t_cmd.dw     = 16;
	e->t_cmd.ns     = 1;
	e->t_cmd.rep    = 0;
	e->t_cmd.vd     = 0;

	e->l_cmd.l2   = 0;
	e->l_cmd.x    = 0;
	e->l_cmd.y    = 0;
	e->l_cmd.notr = 0xff;

	// 変数用レジスタの初期化 //
	e->GR[0] = e->GR[1] = e->GR[2] = e->GR[3] = 0;
	e->GR[4] = e->GR[5] = e->GR[6] = e->GR[7] = 0;

	// 割り込みベクタの初期化 //
	InitECLInterrupt(e);
}


// 強制的に ECL ブロック間を移動する //
extern void EnemyECL_LongJump(ENEMY_DATA *e, DWORD EclID)
{
	e->cmd = (*(DWORD *)(&ECL_Head[EclID]));

	e->call_addr = e->cmd;

	e->t_rep    = 0;			// 弾の発射間隔(０：自動発射しない)
	e->rep_c    = 0;
	e->cmd_c    = 0;
}


// 敵データを初期化する(x,y は非x64(ランダム可能) で指定のこと) //
extern void InitEnemyDataSTD(ENEMY_DATA *e,short x,short y,DWORD EclID)
{
	int		EnemyX = x;
	int		EnemyY = y;
/*
	e->x   = (*(short *)(&p[0]));	//((int)(*(short *)(&p[0])))*64;
	e->y   = (*(short *)(&p[2]));	//((int)(*(short *)(&p[2])))*64;
*/

	// ランダム配置に対応するぞ //
	EnemyX = (EnemyX==X_RNDV) ? GX_RND : (EnemyX<<6);
	EnemyY = (EnemyY==Y_RNDV) ? GY_RND : (EnemyY<<6);

	InitEnemyDataX64(e,EnemyX,EnemyY,EclID);
}

extern void SetAnime24(int x,int y,int id)
{
	int			i;
	ANIME_DATA	*a = Anime+id;

	a->width = a->height = 24;
	a->n     = 16;
	a->mode  = ANM_DEG;

	for(i=0;i<16;i++){
		a->ptn[i].top    = y;
		a->ptn[i].left   = x+(i*24);
		a->ptn[i].bottom = a->ptn[i].top +24;
		a->ptn[i].right  = a->ptn[i].left+24;
	}
}

void SetAnime32(int x,int y,int id,int n,BYTE mode)
{
	int			i;
	ANIME_DATA	*a = Anime+id;

	a->width = a->height = 32;
	a->n     = n;
	a->mode  = mode;

	for(i=0;i<n;i++){
		a->ptn[i].top    = y;
		a->ptn[i].left   = x+(i<<5);//x+i*32;
		a->ptn[i].bottom = a->ptn[i].top +32;
		a->ptn[i].right  = a->ptn[i].left+32;
	}
}

extern void SetAnime40(int x,int y,int id)
{
	int			i;
	ANIME_DATA	*a = Anime+id;

	a->width = a->height = 40;
	a->n     = 16;
	a->mode  = ANM_DEG;

	for(i=0;i<16;i++){
		a->ptn[i].top    = y;
		a->ptn[i].left   = x+i*40;
		a->ptn[i].bottom = a->ptn[i].top  + 40;
		a->ptn[i].right  = a->ptn[i].left + 40;
	}
}

extern void SetAnime48(int x,int y,int id)
{
	int			i;
	ANIME_DATA	*a = Anime+id;

	a->width = a->height = 48;
	a->n     = 16;
	a->mode  = ANM_DEG;

	for(i=0;i<16;i++){
		a->ptn[i].top    = y+(i/8)*48;
		a->ptn[i].left   = x+(i%8)*48;
		a->ptn[i].bottom = a->ptn[i].top  + 48;
		a->ptn[i].right  = a->ptn[i].left + 48;
	}
}

extern void SetAnime80(int x,int y,int id)
{
	int			i;
	ANIME_DATA	*a = Anime+id;

	a->width = a->height = 80;
	a->n     = 16;
	a->mode  = ANM_DEG;

	for(i=0;i<16;i++){
		a->ptn[i].top    = y+(i/8)*80;
		a->ptn[i].left   = x+(i%8)*80;
		a->ptn[i].bottom = a->ptn[i].top  + 80;
		a->ptn[i].right  = a->ptn[i].left + 80;
	}
}

void EnemyAnimeMove(ENEMY_DATA *e)
{
	ANIME_DATA		*a = Anime+(e->anm_ptn);

	switch(a->mode){
		case(ANM_NORM):
			if(e->anm_sp>0 && (e->count%e->anm_sp==0))
				e->anm_c = (e->anm_c+1)%(a->n);
			else if(e->anm_sp<0 && (e->count%(-e->anm_sp)==0))
				e->anm_c = (e->anm_c+a->n-1)%(a->n);
		break;

		// 逆方向は不可としておきましょうか... //
		case(ANM_STOP):
			if(e->anm_sp>0 && (e->count%e->anm_sp==0)){
				if(e->anm_c < (a->n-1)) e->anm_c++;
			}
		break;

		default:
		break;
	}
}

RECT *Enemy_AnimeClip(int *x,int *y,ENEMY_DATA *e)
{
	static RECT		temp;
	BYTE			d;
	ANIME_DATA		*a;

	a = Anime+(e->anm_ptn);

	// 座標セット //
	*x -= (a->width >>1);
	*y -= (a->height>>1);

	// 描画モード選択 //
	switch(a->mode){
		case(ANM_DEG):
			d    = e->d-64+8;
			temp = a->ptn[d>>4];
		break;

		case(ANM_NORM):case(ANM_STOP):default:
			temp = a->ptn[e->anm_c];
		break;
	}

	return GrpClip(&temp,x,y) ? &temp : NULL;
}

extern RECT *Enemy_HitGrpClip(int *x, int *y, ENEMY_DATA *e)
{
	static RECT		temp;
	BYTE			d;
	ANIME_DATA		*a;

	a = Anime+(e->anm_ptnEx);

	// 座標セット //
	*x -= (a->width >>1);
	*y -= (a->height>>1);

	temp = a->ptn[0];

	return GrpClip(&temp,x,y) ? &temp : NULL;
}

static void _EnemyDrawBomb(int x,int y,DWORD count)
{
	RECT		src;

/*
	switch(count/ENEMY_BOMB_SPD){
		case(0):case(1):
			BltSetRect(&src,520,104,8,8);
			x-=4;y-=4;
		break;

		case(2):case(3):
			BltSetRect(&src,528,104,16,16);
			x-=8;y-=8;
		break;

		case(4):case(5):
			BltSetRect(&src,544,104,24,24);
			x-=12;y-=12;
		break;

		case(6):
			BltSetRect(&src,568,104,32,32);
			x-=16;y-=16;
		break;

		case(7):
			BltSetRect(&src,600,104,48,48);
			x-=24;y-=24;
		break;
	}
*/
	src.top    = 296;
	src.left   = (count/ENEMY_BOMB_SPD)*48;
	src.bottom = 296      +48;
	src.right  = src.left +48;

	x-=24;
	y-=24;

	GrpBlt(&src,x,y,GrTama);
}

void parse_ECL(ENEMY_DATA *e)
{
	BYTE *cmd;
	BOOL bRetFlag;		// 実行クロック０命令の場合はFALSEにすること
	BYTE temp;
	DWORD	dwTemp;
	int		RegCmp;
	HLaserInfo		HInfo;

	const RECT rcDegX2 =
		{GX_MIN+150*64, GY_MIN+(GY_MID-GY_MIN-40*64)/3,
			GX_MAX-150*64, GY_MID-(GY_MID-GY_MIN-40*64)/3 - 40*64};
	WORD	BaseAngle;
	WORD	DeltaAngle;


	// こんなところにＧＯＴＯ用ラベルが！！ //
	ECL_HEAD:
		bRetFlag = TRUE;
		cmd = ECL_Head+(e->cmd);


	switch(*cmd){
		case(ECL_CEFC):
		{
			int		x, y;

			x = e->x + ((*(short *)(&cmd[1+0]))<<6);
			y = e->y + ((*(short *)(&cmd[1+2]))<<6);
			CEffectSet(x, y, cmd[1+2+2]);
			bRetFlag = FALSE;
		}
		break;

		case(ECL_XYRND):
			if(e->x > GX_MID){
				e->x =  X_MID * 64 - ( rnd()%(X_MAX - X_MIN - 100) ) * 32;
			}
			else{
				e->x =  X_MID * 64 + ( rnd()%(X_MAX - X_MIN - 100) ) * 32;
			}

			e->y = ( rnd()%(Y_MID - Y_MIN - 160) ) * 64 + (Y_MIN + 40) * 64;
			bRetFlag = FALSE;
		break;

		case(ECL_XYL):	// 1+2 Bytes Param
			e->x += cosl(e->d, (*(short *)(&cmd[1]))<<6);
			e->y += sinl(e->d, (*(short *)(&cmd[1]))<<6);
			bRetFlag = FALSE;
		break;

		case(ECL_STG4EFC):
			switch(cmd[1]){
				case(STG4ROCK_STDMOVE):	SendCmdStg4Rock(cmd[1], 0);		break;
				case(STG4ROCK_ACCMOVE1):SendCmdStg4Rock(cmd[1], 0);		break;
				case(STG4ROCK_ACCMOVE2):SendCmdStg4Rock(cmd[1], e->d);	break;
				case(STG4ROCK_3DMOVE):	SendCmdStg4Rock(cmd[1], 0);		break;
				case(STG4ROCK_LEAVE):	SendCmdStg4Rock(cmd[1], 0);		break;
				case(STG4ROCK_END):		SendCmdStg4Rock(cmd[1], 0);		break;
			}

			bRetFlag = FALSE;
		break;

		case(ECL_STG3EFC):
			ScrollCommand(SCMD_STG3STAR);
		break;

		case(ECL_ITEM):
			e->item = cmd[1];
		break;

		case(ECL_HITXY):		// 当たり判定を変更する
			e->g_width  = (*(WORD *)(&cmd[1]))<<6;
			e->g_height = (*(WORD *)(&cmd[3]))<<6;
			bRetFlag = FALSE;
		break;

		case(ECL_HLASER):		// ホーミングレーザーセット
			HInfo.c    = e->l_cmd.c;
			HInfo.d    = e->l_cmd.d;
			HInfo.dw   = e->l_cmd.dw;
			HInfo.n    = e->l_cmd.n;
			HInfo.type = e->l_cmd.type;
			HInfo.x    = e->x + e->l_cmd.x;
			HInfo.y    = e->y + e->l_cmd.y;
			HLaserSet(&HInfo);
		break;

		case(ECL_LLSET):		// 太レーザーセット
			LLaserCmd.c  = e->l_cmd.c;
			LLaserCmd.d  = e->l_cmd.d;
			LLaserCmd.dx = e->l_cmd.x;
			LLaserCmd.dy = e->l_cmd.y;
			LLaserCmd.e  = e;
			LLaserCmd.type = e->l_cmd.type;
			//LLaserCmd.type = (e->l_cmd.type==0) ? LLS_LONG : LLS_SETDEG;
			LLaserCmd.v = e->l_cmd.v;
			LLaserCmd.w = e->l_cmd.w;

			// 失敗した場合は、参照カウントを増やさない //
			if(LLaserSet(e->LLaserRef)) e->LLaserRef++;
			bRetFlag = FALSE;
		break;

		case(ECL_LLOPEN):		// 太レーザーオープン cmd,id
			LLaserOpen(e,cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_LLCLOSE):		// 太レーザークローズ(消去＆参照カウント減少) cmd,id
			LLaserClose(e,cmd[1]);
			if(cmd[1]==ECLCST_LLASERALL) e->LLaserRef =  0;
			else                         e->LLaserRef -= 1;		// ちょっとバグ有りなので注意
			bRetFlag = FALSE;
		break;

		case(ECL_LLCLOSEL):		// 太レーザーライン状態へ cmd,id
			LLaserLine(e,cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_LLDEGR):		// 太レーザー角度相対変更 cmd,id,deg
			LLaserDegR(e,(char)cmd[2],cmd[1]);	// 順番が逆だから注意ね
			bRetFlag = FALSE;
		break;

		case(ECL_SETUP):		// 敵の初期化
			ECL_DEBUG("ECL_SETUP",0);
			e->hp    = *(DWORD *)(&cmd[1]);
			e->score = *(DWORD *)(&cmd[1+4]);
			if(e->hp==0) BossKillAll();
			bRetFlag = FALSE;
		break;

		case(ECL_END):			// 敵の強制消滅
			ECL_DEBUG("ECL_END",0);
			if(e->LLaserRef) LLaserForceClose(e);	// レーザーの強制クローズ
			e->flag = EF_DELETE;	// 後で変更するように
		return;						// バグ防止(かも)

		case(ECL_JMP):			// ◎ＥＣＬ無条件ジャンプ(少々特殊な動作をします)
			ECL_DEBUG("ECL_JMP",0);
			e->cmd = *(DWORD *)(&cmd[1]);
		goto ECL_HEAD;

		case(ECL_LOOP):			// ◎一定区間を繰り返す
			ECL_DEBUG("ECL_LOOP : %d",e->rep_c);
			if(e->rep_c==0) e->rep_c = (*(WORD *)(&cmd[1+4]))+1;
			if((--e->rep_c)!=0){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_CALL):			// ＠サブルーチンを呼ぶ
			ECL_DEBUG("ECL_CALL",0);
			e->call_addr = e->cmd + ECL_CmdLen[ECL_CALL];
			e->cmd       = *(DWORD *)(&cmd[1]);
		goto ECL_HEAD;

		case(ECL_RET):			// ＠サブルーチンから復帰する
			ECL_DEBUG("ECL_RET",0);
			e->cmd = e->call_addr;
		goto ECL_HEAD;

		case(ECL_JHPL):			// ◎ＨＰが指定値より大きければジャンプ
			ECL_DEBUG("ECL_JHPL : %d",(DWORD)cmd[1]);
			if(e->hp>(*(DWORD *)(&cmd[1+4]))){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_JHPS):			// ◎ＨＰが指定値より小さければジャンプ
			ECL_DEBUG("ECL_JHPS : %d",(DWORD)cmd[1]);
			if(e->hp<(*(DWORD *)(&cmd[1+4]))){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_JDIF):			// 難易度によるジャンプ
			ECL_DEBUG("ECL_JDIF",0);
			switch(PlayRank.GameLevel){
				case(GAME_EASY):	e->cmd = *(DWORD *)(&cmd[1]);		break;
				default:
				case(GAME_NORMAL):	e->cmd = *(DWORD *)(&cmd[1+4]);		break;
				case(GAME_HARD):	e->cmd = *(DWORD *)(&cmd[1+8]);		break;
				case(GAME_LUNATIC):	e->cmd = *(DWORD *)(&cmd[1+12]);	break;
			}
		goto ECL_HEAD;

		case(ECL_JDSB):			// 自機と進行角が一致したらジャンプ
			ECL_DEBUG("ECL_JDSB",0);
			temp = abs(atan8(Viv.x-e->x,Viv.y-e->y) - (e->d));
			if(temp<4){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_JFCL):			// フレームカウンタが大きければジャンプ
			ECL_DEBUG("ECL_JFCL",0);
			if(e->count>(*(DWORD *)(&cmd[1+4]))){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_JFCS):			// フレームカウンタが小さければジャンプ
			ECL_DEBUG("ECL_JFCS",0);
			if(e->count<(*(DWORD *)(&cmd[1+4]))){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_STI):			// 割り込みベクタをセットする Addr(4),条件(1),比較値(4)
			switch(cmd[1+4]){
				case(ECLVECT_BITLEFT):
					e->Vect[ECLVECT_BITLEFT].vect  = (*(DWORD *)(&cmd[1]));
					e->Vect[ECLVECT_BITLEFT].value = (*(DWORD *)(&cmd[1+4+1]));
				break;

				case(ECLVECT_BOSSLEFT):
					e->Vect[ECLVECT_BOSSLEFT].vect  = (*(DWORD *)(&cmd[1]));
					e->Vect[ECLVECT_BOSSLEFT].value = (*(DWORD *)(&cmd[1+4+1]));
				break;

				case(ECLVECT_HP):
					e->Vect[ECLVECT_HP].vect  = (*(DWORD *)(&cmd[1]));
					e->Vect[ECLVECT_HP].value = (*(DWORD *)(&cmd[1+4+1]));
				break;

				case(ECLVECT_TIMER):
					e->Vect[ECLVECT_TIMER].vect  = (*(DWORD *)(&cmd[1]));
					e->Vect[ECLVECT_TIMER].value = (*(DWORD *)(&cmd[1+4+1]));
					e->IntTimer = 0;
				break;

				default:
					ECL_DEBUG("不正な割り込みベクタ %d へのアクセス",cmd[1+4]);
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_CLI):			// 割り込みベクタをクリアする
			switch(cmd[1]){
				case(ECLVECT_BITLEFT):
					e->Vect[ECLVECT_BITLEFT].vect  = 0;
				break;

				case(ECLVECT_BOSSLEFT):
					e->Vect[ECLVECT_BOSSLEFT].vect  = 0;
				break;

				case(ECLVECT_HP):
					e->Vect[ECLVECT_HP].vect  = 0;
				break;

				case(ECLVECT_TIMER):
					e->Vect[ECLVECT_TIMER].vect = 0;
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_NOP):			// ＠何もしない
			ECL_DEBUG("ECL_NOP : %d",e->cmd_c);
			if(e->cmd_c==0) e->cmd_c = (*(WORD *)(&cmd[1]))+1;
			if((--e->cmd_c)!=0) return;
			bRetFlag = FALSE;
		break;

		case(ECL_NOPSC):		// スクロールに流される
			ECL_DEBUG("ECL_NOPSC : %d",e->cmd_c);
			if(e->cmd_c==0) e->cmd_c = (*(WORD *)(&cmd[1]))+1;
			if((--e->cmd_c)!=0){
				// スクロールに流される処理を記述 //
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_T2ITEM):		// 弾の何％かをアイテム化する
			tama2item(cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_ACC):			// 加速移動
			ECL_DEBUG("ECL_ACC : %d",e->cmd_c);
			if(e->cmd_c == 0){
				// 初期化 //
				e->cmd_c = (*(WORD *)(&cmd[2]))+1;
				//e->vx    = cosl(e->d,e->v);
				//e->vy    = sinl(e->d,e->v);
			}
			if((--e->cmd_c)!=0){
				e->v += (char)cmd[1];
				e->x += cosl(e->d, e->v);
				e->y += sinl(e->d, e->v);

				return;
			}
			// 最後は、何もしない訳で... //
			bRetFlag = FALSE;
		break;

		case(ECL_ACCXYA):		// ＸＹ指定加速移動
			// ちょっと、待ってね //
		break;

		case(ECL_DEGX2):		// 制限付き角度ランダム
			if(e->y < rcDegX2.top){
				if(e->x < rcDegX2.left){
					// 左上 //
					BaseAngle  = 32-16;//0;
					DeltaAngle = 32;//64;
				}
				else if(e->x > rcDegX2.right){
					// 右上 //
					BaseAngle  = 96-16;//64;
					DeltaAngle = 32;//64;
				}
				else{
					// 上端 //
					//BaseAngle  = 24+(rnd()>>1)%(64-16)-16;//0;
					BaseAngle  = 32 + ((rnd()>>1)&1)*64 - 16;
					DeltaAngle = 32;//128;
				}
			}
			else if(e->y > rcDegX2.bottom){
				if(e->x < rcDegX2.left){
					// 左下 //
					BaseAngle  = -32-16;//192;
					DeltaAngle = 32;//64;
				}
				else if(e->x > rcDegX2.right){
					// 右下 //
					BaseAngle  = 128+32-16;//128;
					DeltaAngle = 32;//64;
				}
				else{
					// 下端 //
					BaseAngle  = 128+64-16;//128;
					DeltaAngle = 32;//128;
				}
			}
			else{
				if(e->x < rcDegX2.left){
					// 左側 //
					BaseAngle  = -16;//192;
					DeltaAngle = 32;//128;
				}
				else if(e->x > rcDegX2.right){
					// 右側 //
					BaseAngle  = 128-16;//64;
					DeltaAngle = 32;//128;
				}
				else{
					// 真ん中 //
					BaseAngle  = ((rnd()>>1)&1) ? (-16) : (128-16);
					DeltaAngle = 32;
				}
			}

			// 実際に角度を確定する //
			e->d = BaseAngle + (rnd()>>1)%DeltaAngle;

			bRetFlag = FALSE;
		break;

		case(ECL_MOV):			// ＠直線移動
			ECL_DEBUG("ECL_MOV : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1]))+1;
				e->vx    = cosl(e->d,e->v);
				e->vy    = sinl(e->d,e->v);
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				e->y += e->vy;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_ROL):			// ＠回転移動
			ECL_DEBUG("ECL_ROL : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+1]))+1;
				e->vd    = REL_DEGRL((char)cmd[1]);
			}
			if((--e->cmd_c)!=0){
				e->x += cosl(e->d,e->v);
				e->y += sinl(e->d,e->v);
				e->d += e->vd;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_LROL):			// ＠回転＆直線移動
			ECL_DEBUG("ECL_LROL : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+9]))+1;
				e->vx    = ABS_VXRL((*(int  *)(&cmd[1]   )));
				e->vy    = (*(int  *)(&cmd[1+4] ));
				e->vd    = REL_DEGRL((char)cmd[1+8]);
			}
			if((--e->cmd_c)!=0){
				e->x += (cosl(e->d,e->v)+e->vx);
				e->y += (sinl(e->d,e->v)+e->vy);
				e->d += e->vd;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_WAVX):			// ＠波Ｘ移動
			ECL_DEBUG("ECL_WAVX : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+6]))+1;
				e->vx    = ABS_VXRL(*(int  *)(&cmd[1]  ));
				e->vy    = e->y;
				e->amp   = (BYTE)cmd[1+4];
				e->vd    = (char)cmd[1+5];
				//e->d     = 0;
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				e->y  = e->vy + sinl(e->d,e->amp<<6);
				e->d += e->vd;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_WAVY):			// ＠波Ｙ移動
			ECL_DEBUG("ECL_WAVY : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+6]))+1;
				e->vy    = (*(int  *)(&cmd[1]  ));
				e->vx    = e->x;
				e->amp   = (BYTE)cmd[1+4];
				e->vd    = (char)cmd[1+5];
				//e->d     = 0;
			}
			if((--e->cmd_c)!=0){
				e->y += e->vy;
				e->x  = e->vx + sinl(e->d,e->amp<<6);
				e->d += e->vd;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MXA):		// Ｘ絶対移動
			ECL_DEBUG("ECL_MXA : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+2]))+1;
				e->vx    = (((*(WORD *)(&cmd[1]))<<6)-(e->x))/e->cmd_c;
				e->vy    = 0;
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MYA):		// Ｙ絶対移動
			ECL_DEBUG("ECL_MYA : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+2]))+1;
				e->vy    = (((*(WORD *)(&cmd[1]))<<6)-(e->y))/e->cmd_c;
				e->vx    = 0;
			}
			if((--e->cmd_c)!=0){
				e->y += e->vy;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MXYA):		// ＸＹ絶対移動
			ECL_DEBUG("ECL_MXYA : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1+4]))+1;
				e->vx    = (((*(WORD *)(&cmd[1+0]))<<6)-(e->x))/e->cmd_c;
				e->vy    = (((*(WORD *)(&cmd[1+2]))<<6)-(e->y))/e->cmd_c;
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				e->y += e->vy;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MXS):		// Ｘサボテンセット移動
			ECL_DEBUG("ECL_MXS : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1]))+1;
				e->vx    = ((Viv.x)-(e->x))/e->cmd_c;
				e->vy    = 0;
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MYS):		// Ｙサボテンセット移動
			ECL_DEBUG("ECL_MYS : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1]))+1;
				e->vx    = 0;
				e->vy    = ((Viv.y)-(e->y))/e->cmd_c;
			}
			if((--e->cmd_c)!=0){
				e->y += e->vy;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MXYS):		// ＸＹサボテンセット移動
			ECL_DEBUG("ECL_MXYS : %d",e->cmd_c);
			if(e->cmd_c==0){
				e->cmd_c = (*(WORD *)(&cmd[1]))+1;
				e->vx    = ((Viv.x)-(e->x))/e->cmd_c;
				e->vy    = ((Viv.y)-(e->y))/e->cmd_c;
			}
			if((--e->cmd_c)!=0){
				e->x += e->vx;
				e->y += e->vy;
				return;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_GRAX):		// 重力付きＸ反射移動(Y_MIN 含むけど...)
			// 注意：この命令から脱出する方法は割り込み以外に存在しない //
			if(e->cmd_c == 0){
				e->cmd_c = 9999;	// 非ゼロ値であれば、どのような値でも良い
				e->vx    = cosl(e->d, e->v);
				e->vy    = sinl(e->d, e->v);
				e->vd    = (char)cmd[1];		// 重力加速度!!
				e->flag |= EF_CLIP;				// クリッピング属性を自動的にセットする
			}
			else{
				e->x += e->vx;
				e->y += e->vy;
				e->vy += e->vd;

				// Ｘ方向のチェック //
				if((e->x) < GX_MIN || (e->x) > GX_MAX){
					e->vx = -(e->vx);	// 速度反転
					e->x += e->vx;
				}
				// Ｙ方向(上)のチェック //
				if((e->y) < GY_MIN){
					e->vy = -(e->vy);	// 速度を反転するのです
					e->y += e->vy;
				}
				// Ｙ方向(下)のチェック -> さよならですな //
				// この部分だけ、縦方向判定を広く取るのだ //
				if((e->y) > GY_MAX+(e->g_height)){
					e->flag = EF_DELETE;	// 消えておしまい
				}
			}
		return;

		case(ECL_DEGA):			// ＠角度絶対指定
			ECL_DEBUG("ECL_DEGA : %d",(BYTE)cmd[1]);
			e->d = ABS_DEGRL((BYTE)cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_DEGR):			// ＠角度相対指定
			ECL_DEBUG("ECL_DEGR : %d",(char)cmd[1]);
			e->d += REL_DEGRL((char)cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_DEGX):			// ＠角度ランダムセット
			ECL_DEBUG("ECL_DEGX",0);
			e->d = rnd() & 0xff;
			bRetFlag = FALSE;
		break;

		case(ECL_DEGXU):		// ＠角度ランダムセット(上)
			e->d = 128 + (rnd()&0x7f);
			bRetFlag = FALSE;
		break;

		case(ECL_DEGXD):		// ＠角度ランダムセット(下)
			e->d = rnd()&0x7f;
			bRetFlag = FALSE;
		break;

		case(ECL_DEGEX):
			e->d = EnemyEXDEG;
			EnemyEXDEG += EnemyEXDEG_D;
			bRetFlag = FALSE;
		break;

		case(ECL_DEGS):			// ＠角度自機セット
			ECL_DEBUG("ECL_DEGS",0);
			e->d = atan8(Viv.x-e->x,Viv.y-e->y);
			bRetFlag = FALSE;
		break;

		case(ECL_SPDA):			// ＠速度絶対指定
			ECL_DEBUG("ECL_SPDA",0);
			e->v = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_SPDR):			// ＠速度相対指定
			ECL_DEBUG("ECL_SPDR",0);
			e->v += (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_XYA):			// ＠座標絶対指定
			ECL_DEBUG("ECL_XYA",0);
			e->x = ((int)(*(short *)(&cmd[1]  )))<<6;
			e->y = ((int)(*(short *)(&cmd[1+2])))<<6;
			bRetFlag = FALSE;
		break;

		case(ECL_XYR):			// ＠座標相対指定
			ECL_DEBUG("ECL_XYR",0);
			e->x += (((int)(*(short *)(&cmd[1]  )))<<6);
			e->y += (((int)(*(short *)(&cmd[1+2])))<<6);
			bRetFlag = FALSE;
		break;

		case(ECL_XYS):
			e->x = Viv.x;
			e->y = Viv.y;
			bRetFlag = FALSE;
		break;

		case(ECL_TAMA):			// ＠弾発射
			TamaCmd = e->t_cmd;
			TamaCmd.x += e->x;
			TamaCmd.y += e->y;
			tama_set();
			bRetFlag = FALSE;
		break;

		case(ECL_TAMA2):		// ＠弾発射(難易度変化なし)
			TamaCmd = e->t_cmd;
			TamaCmd.x += e->x;
			TamaCmd.y += e->y;
			tama_setEX();
			bRetFlag = FALSE;
		break;

		case(ECL_TAMAL):		// ライン状に弾を発射する
			TamaCmd = e->t_cmd;
			TamaCmd.x += e->x;
			TamaCmd.y += e->y;
			tama_setLine();
			bRetFlag = FALSE;
		break;

		case(ECL_TAMAEX):
			TamaCmd = e->t_cmd;
			TamaCmd.x += e->x;
			TamaCmd.y += e->y;
			tama_setExtra01();
			bRetFlag = FALSE;
		break;

		case(ECL_TAUTO):		// 弾発射モード変更
			e->t_rep = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TXYR):			// ＠弾発射位置相対指定
			e->t_cmd.x = (*(short *)(&cmd[1]  ))<<6;
			e->t_cmd.y = (*(short *)(&cmd[1+2]))<<6;
			bRetFlag = FALSE;
		break;

		case(ECL_TCMD):			// ＠弾コマンド
			e->t_cmd.cmd = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TDEGA):		// ＠弾発射角絶対指定
			e->t_cmd.d  = (BYTE)cmd[1];
			e->t_cmd.dw = (BYTE)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TDEGR):		// ＠弾発射角相対指定
			e->t_cmd.d  += (char)cmd[1];
			e->t_cmd.dw += (char)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TDEGS):		// ＠弾発射角サボテンセット
			// 正確には、TamaCmd の x,y も使うべきだが...
			e->t_cmd.d = atan8(Viv.x-e->x,Viv.y-e->y);
			bRetFlag = FALSE;
		break;

		case(ECL_TDEGE):		// ＠弾発射角の同期をとる
			e->t_cmd.d = e->d;
			bRetFlag = FALSE;
		break;

		case(ECL_TNUMA):		// ＠弾発射数絶対指定
			e->t_cmd.n  = (BYTE)cmd[1];
			e->t_cmd.ns = (BYTE)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TNUMR):		// ＠弾発射数相対指定
			e->t_cmd.n  += (char)cmd[1];
			e->t_cmd.ns += (char)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TSPDA):		// ＠弾速度絶対指定
			e->t_cmd.v = (BYTE)cmd[1];
			e->t_cmd.a = (char)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TSPDR):		// ＠弾速度相対指定
			temp = e->t_cmd.v;
			e->t_cmd.v  = ((temp&0x3f)+(char)cmd[1])&0x3f;	// フラグを外して演算
			e->t_cmd.v |= (temp&0xc0);//(temp&0x3c);						// フラグを書き戻す
			e->t_cmd.a += (char)cmd[1+1];
			bRetFlag = FALSE;
		break;

		case(ECL_TOPT):			// ＠弾オプション
			e->t_cmd.option = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TTYPE):		// ＠弾タイプ
			e->t_cmd.type = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TCOL):			// ＠弾の色もしくは形状
			e->t_cmd.c = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TVDEG):		// ＠弾の角速度
			e->t_cmd.vd = (char)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TREP):			// ＠弾の REP 指定
			e->t_cmd.rep = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_TCLR):			// 敵弾を全消去(レーザー含む)
			BossClearCmd();	// この処理を何よりも優先させる(ビット消去等を含む)
			tama_clear();
			laser_clear();
			HLaserClear();
			enemy_clear();
			bRetFlag = FALSE;
		break;

		case(ECL_LASER):		// レーザー発射
			LaserCmd = e->l_cmd;
			LaserCmd.x += e->x;
			LaserCmd.y += e->y;
			laser_set();
			bRetFlag = FALSE;
		break;

		case(ECL_LASER2):		// レーザー発射
			LaserCmd = e->l_cmd;
			LaserCmd.x += e->x;
			LaserCmd.y += e->y;
			laser_setEX();
			bRetFlag = FALSE;
		break;

		case(ECL_LCMD):			// レーザーコマンドセット
			e->l_cmd.cmd = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_LLA):			// レーザー長・絶対指定
			e->l_cmd.l = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LLR):			// レーザー長・相対指定
			e->l_cmd.l += (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LL2):			// レーザー発射位置
			e->l_cmd.l2 = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LDEGA):		// レーザー発射角＆幅絶対指定
			e->l_cmd.d  = (BYTE)cmd[1];
			e->l_cmd.dw = (BYTE)cmd[2];
			bRetFlag = FALSE;
		break;

		case(ECL_LDEGR):		// レーザー発射角＆幅相対指定
			e->l_cmd.d  += (char)cmd[1];
			e->l_cmd.dw += (char)cmd[2];
			bRetFlag = FALSE;
		break;

		case(ECL_LDEGS):		// レーザー発射角サボテンセット
			// 正確には、LaserCmd の x,y も使うべきだが...
			e->l_cmd.d = atan8(Viv.x-e->x,Viv.y-e->y);
			bRetFlag = FALSE;
		break;

		case(ECL_LDEGE):		// レーザー発射角を自分の向きにセット
			e->l_cmd.d = e->d;
			bRetFlag = FALSE;
		break;

		case(ECL_LNUMA):		// レーザーの本数絶対指定
			e->l_cmd.n = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_LNUMR):		// レーザーの本数相対指定
			e->l_cmd.n += (char)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_LSPDA):		// レーザーの速度絶対指定
			e->l_cmd.v = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LSPDR):		// レーザーの速度相対指定
			e->l_cmd.v = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LCOL):			// レーザーの色
			e->l_cmd.c = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_LTYPE):		// レーザーの種類
			e->l_cmd.type = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_LWA):			// レーザーの太さ絶対指定
			e->l_cmd.w = (*(int *)(&cmd[1]));
			bRetFlag = FALSE;
		break;

		case(ECL_LXY):			// レーザーの発射位置指定
			e->l_cmd.x = (*(short *)(&cmd[1]  ))<<6;
			e->l_cmd.y = (*(short *)(&cmd[1+2]))<<6;
			bRetFlag = FALSE;
		break;

		case(ECL_DRAW_ON):		// ＠描画する
			ECL_DEBUG("ECL_DRAW_ON",0);
			bRetFlag = FALSE;
			e->flag |= EF_DRAW;
		break;

		case(ECL_DRAW_OFF):		// ＠描画しない
			ECL_DEBUG("ECL_DRAW_OFF",0);
			bRetFlag = FALSE;
			e->flag &= (~EF_DRAW);
		break;

		case(ECL_CLIP_ON):		// ＠画面外消去しない
			ECL_DEBUG("ECL_CLIP_ON",0);
			bRetFlag = FALSE;
			e->flag |= EF_CLIP;
		break;

		case(ECL_CLIP_OFF):		// ＠画面外消去する
			ECL_DEBUG("ECL_CLIP_OFF",0);
			bRetFlag = FALSE;
			e->flag &= (~EF_CLIP);
		break;

		case(ECL_DAMAGE_ON):	// ＠ダメージ有り
			ECL_DEBUG("ECL_DAMAGE_ON",0);
			bRetFlag = FALSE;
			e->flag |= EF_DAMAGE;
		break;

		case(ECL_DAMAGE_OFF):	// ＠ダメージ無し
			ECL_DEBUG("ECL_DAMAGE_OFF",0);
			bRetFlag = FALSE;
			e->flag &= (~EF_DAMAGE);
		break;

		case(ECL_HITSB_ON):		// ＠自機との当たり判定有り
			ECL_DEBUG("ECL_HITSB_ON",0);
			bRetFlag = FALSE;
			e->flag |= EF_HITSB;
		break;

		case(ECL_HITSB_OFF):	// ＠自機との当たり判定無し
			ECL_DEBUG("ECL_HITSB_OFF",0);
			bRetFlag = FALSE;
			e->flag &= (~EF_HITSB);
		break;

		case(ECL_RLCHG_ON):		// ＠左右反転有り(左側にいればセット)
			ECL_DEBUG("ECL_RLCHG_ON",0);
			bRetFlag = FALSE;
			if(e->x<GX_MID) e->flag |=   EF_RLCHG;
			else            e->flag &= (~EF_RLCHG);
		break;

		case(ECL_RLCHG_OFF):	// ＠左右反転無し
			ECL_DEBUG("ECL_RLCHG_OFF",0);
			bRetFlag = FALSE;
			e->flag &= (~EF_RLCHG);
		break;

		case(ECL_ANM):			// アニメーションセット
			// anm_ptnEx をセットするのは互換性を保つための配慮 //
			e->anm_ptn  = e->anm_ptnEx = (BYTE)cmd[1];
			e->anm_sp   = (char)cmd[2];
			//if(e->anm_sp==0) e->anm_sp=1;
			e->g_height = (Anime[e->anm_ptn].height<<5);
			e->g_width  = (Anime[e->anm_ptn].width <<5);
			//if(Anime[e->anm_ptn].height>32) e->g_height = (e->g_height<<1)/3;
			//if(Anime[e->anm_ptn].width>32)  e->g_width  = (e->g_width <<1)/3;
			e->anm_c    = 0;
			bRetFlag = FALSE;
		break;

		case(ECL_ANMEX):
			// 他のパラメータはいっさい変更しない //
			e->anm_ptnEx = (BYTE)cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_PSE):			// 効果音を鳴らす
			ECL_DEBUG("ECL_PSE",0);
			SndPlayEX(cmd[1],e->x,0);
			bRetFlag = FALSE;
		break;

		case(ECL_INT):			// ボス用割り込み発生
			BossINT(e,cmd[1]);
			bRetFlag = FALSE;
		break;					// cmd を動かさない

		case(ECL_BITATTACK):	// (ボス特権命令)ビットに攻撃パターンをセットする
			BossBitAttack(e,(*(DWORD *)(&cmd[1])));
			bRetFlag = FALSE;
		break;

		case(ECL_BITLASER):		// (ボス特権命令)ビットにレーザー系コマンドをセットする
			BossBitLaser(e, cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_BITCMD):
			BossBitCommand(e, cmd[1], (*(int *)(&cmd[2])));
			bRetFlag = FALSE;
		break;

		case(ECL_EXDEGD):		// 特殊角度増分変更
			EnemyEXDEG_D = cmd[1];
			bRetFlag = FALSE;
		break;

		case(ECL_ENEMYSET):		// 敵を雑魚指定で発生させる
		case(ECL_ENEMYSETD):	// ＋角度指定(レジスタ)
		{
			DWORD			n;
			ENEMY_DATA		*new_enemy;
			short			x,y;

			bRetFlag = FALSE;

			if(EnemyNow+1>=ENEMY_MAX) break;
			new_enemy = Enemy+ (*(EnemyInd+EnemyNow));
			EnemyNow++;

			x   = (e->x >>6) + (*(short *)(&cmd[1]));	//((int)(*(short *)(&p[0])))*64;
			y   = (e->y >>6) + (*(short *)(&cmd[3]));	//((int)(*(short *)(&p[2])))*64;

			// バグに注意注意！！ //
			if(cmd[0] == ECL_ENEMYSETD){
				n   = 4 + (((BYTE)cmd[6])<<2);
				InitEnemyDataSTD(new_enemy,x,y,n);
				new_enemy->d = (BYTE)ID2Value(e,cmd[5]);
			}
			else{
				n   = 4 + (((BYTE)cmd[5])<<2);
				InitEnemyDataSTD(new_enemy,x,y,n);
			}
		}
		break;

		case(ECL_BOSSSET):		// ボスを発生させる
			BossSetEx((e->x)>>6, (e->y)>>6, cmd[1]);
			bRetFlag = FALSE;
		break;

		case(ECL_MOVR):		// レジスタ<->構造体変数の代入
			dwTemp = ID2Value(e,cmd[2]);
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]] = dwTemp;
				break;

				case(ECLCST_LCMD_D):	e->l_cmd.d = (BYTE)dwTemp;break;	// レーザーコマンド(角度)
				case(ECLCST_LCMD_DW):	e->l_cmd.dw= (BYTE)dwTemp;break;	// レーザーコマンド(角度差)
				case(ECLCST_LCMD_N):	e->l_cmd.n = (BYTE)dwTemp;break;	// レーザーコマンド(本数)
				case(ECLCST_LCMD_C):	e->l_cmd.c = (BYTE)dwTemp;break;	// レーザーコマンド(色)
				case(ECLCST_LCMD_L):	e->l_cmd.l = dwTemp;break;			// レーザーコマンド(長さ)
				case(ECLCST_LCMD_V):	e->l_cmd.v = dwTemp;break;			// レーザーコマンド(速度)

				case(ECLCST_TCMD_D):	e->t_cmd.d  = (BYTE)dwTemp;break;	// 弾コマンド(角度)
				case(ECLCST_TCMD_DW):	e->t_cmd.dw = (BYTE)dwTemp;break;	// 弾コマンド(角度差)
				case(ECLCST_TCMD_N):	e->t_cmd.n  = (BYTE)dwTemp;break;	// 弾コマンド(個数)
				case(ECLCST_TCMD_NS):	e->t_cmd.ns = (BYTE)dwTemp;break;	// 弾コマンド(連射数)
				case(ECLCST_TCMD_V):	e->t_cmd.v  = (BYTE)dwTemp;break;	// 弾コマンド(速度)
				case(ECLCST_TCMD_C):	e->t_cmd.c  = (BYTE)dwTemp;break;	// 弾コマンド(色)
				case(ECLCST_TCMD_A):	e->t_cmd.a  = (char)dwTemp;break;	// 弾コマンド(加速度)

				case(ECLCST_TCMD_REP):
					//char buf[100];
					//sprintf(buf,"REP=%d [REG:%d]",dwTemp,cmd[2]);
					//DebugOut(buf);
					e->t_cmd.rep= (BYTE)dwTemp;
				break;	// 弾コマンド(繰り返し)

				case(ECLCST_TCMD_VD):	e->t_cmd.vd = (char)dwTemp;break;	// 弾コマンド(角速度)

				case(ECLCST_ENEMY_X):	e->x = (int)dwTemp;		break;		// 敵のＸ座標
				case(ECLCST_ENEMY_Y):	e->y = (int)dwTemp;		break;		// 敵のＸ座標
				case(ECLCST_ENEMY_D):	e->d = (BYTE)dwTemp;	break;		// 敵の角度

				default:
					ErrInsert("ナゾのレジスタ指定++");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_MOVC):		// レジスタ<- 定数(即値)の代入
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]] = (*(DWORD *)(&cmd[2]));
				break;

				default:	// レジスタ指定がおかしい
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_INC):		// レジスタ＋１
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]]++;
				break;

				default:	// レジスタ指定がおかしい
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_DEC):		// レジスタ－１
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]]--;
				break;

				default:	// レジスタ指定がおかしい
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_ADD):		// 加算命令(第２引数はレジスタでなくてもよい)
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]] += (int)ID2Value(e,cmd[2]);
				break;

				default:	// レジスタ指定がおかしいに
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_SUB):		// 減算命令(第２引数はレジスタでなくてもよい)
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]] -= (int)ID2Value(e,cmd[2]);
				break;

				default:	// レジスタ指定がおかしいに
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_SINL):		// sinl(Gr0,Gr1)
			if(cmd[1]<ECLREG_MAX && cmd[2]<ECLREG_MAX)
				e->GR[cmd[1]] = sinl((BYTE)e->GR[cmd[2]],e->GR[cmd[1]]);
			//else
				// 本当はswitch()で判別したいが...
			bRetFlag = FALSE;
		break;

		case(ECL_COSL):		// cosl(Gr0,Gr1)
			if(cmd[1]<ECLREG_MAX && cmd[2]<ECLREG_MAX)
				e->GR[cmd[1]] = cosl((BYTE)e->GR[cmd[2]],e->GR[cmd[1]]);
			//else
				// 本当はswitch()で判別したいが...
			bRetFlag = FALSE;
		break;

		case(ECL_MOD):		// Gr0 = Gr0 % Const
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					if((*(DWORD *)(&cmd[2]))!=0)
						e->GR[cmd[1]] %= (*(DWORD *)(&cmd[2]));
					//else
						// ゼロ除算エラー
					break;

				default:	// レジスタ指定がおかしい
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_RND):		// Gr0 = rnd()
			switch(cmd[1]){
				case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
				case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
					e->GR[cmd[1]] = (DWORD)rnd()*rnd();
				break;

				default:	// レジスタ指定がおかしい
					ErrInsert("ナゾのレジスタ指定");
				break;
			}
			bRetFlag = FALSE;
		break;

		case(ECL_CMPR):		// レジスタ～レジスタの比較(Reg0,Reg1)
			if(cmd[1]>=ECLREG_MAX || cmd[2]>=ECLREG_MAX) return;		// エラー
			RegCmp = ( (int)ID2Value(e,cmd[1]) - (int)ID2Value(e,cmd[2]) );
			bRetFlag = FALSE;
		break;

		case(ECL_CMPC):		// レジスタ～定数の比較(Reg,Const)
			if(cmd[1]>=ECLREG_MAX) return;		// エラー
			RegCmp = ( (int)ID2Value(e,cmd[1]) - (*(int *)&cmd[1+1]) );
			bRetFlag = FALSE;
		break;

		case(ECL_JL):		// 比較結果 > 0 ならばジャンプ
			if(RegCmp>0){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			else bRetFlag = FALSE;
		break;

		case(ECL_JS):		// 比較結果 < 0 ならばジャンプ
			if(RegCmp<0){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			else bRetFlag = FALSE;
		break;

		case(ECL_JEQ):		// 比較結果 == 0 ならばジャンプ
			if(RegCmp == 0){
				e->cmd = *(DWORD *)(&cmd[1]);
				goto ECL_HEAD;
			}
			else bRetFlag = FALSE;
		break;

		default:				// 未定義の命令が発生！！
			ECL_DEBUG("Unrecognizable Operation Code %02x",(BYTE)cmd[0]);
		return;
	}

	// 次の命令への準備(実行させたくないときは上の switch 内で return する) //
	e->cmd += ECL_CmdLen[*cmd];

	if(bRetFlag) return;
	else         goto ECL_HEAD;
}

// 割り込みジャンプを調べる //
extern void CheckECLInterrupt(ENEMY_DATA *e)
{
	int		i;

	for(i=0;i<ECLVECT_MAX;i++){
		if(e->Vect[i].vect==0) continue;	// 割り込みがかかっていない
		switch(i){
			case(ECLVECT_BITLEFT):	// ビット残り割り込み
				if(BossGetBitLeft() <= e->Vect[ECLVECT_BITLEFT].value){
					e->cmd = e->Vect[ECLVECT_BITLEFT].vect;
					e->cmd_c = 0;		// コマンド繰り返しカウンタ
					e->rep_c = 0;		// LOOP(旧REP)命令カウンタ
					e->t_rep = 0;		// 自動弾発射タイミング(0:自動発射せず)
					return;
				}
			break;

			case(ECLVECT_BOSSLEFT):	// ボス残り割り込み
				if(BossNow <= e->Vect[ECLVECT_BOSSLEFT].value){
					e->cmd = e->Vect[ECLVECT_BOSSLEFT].vect;
					e->cmd_c = 0;		// コマンド繰り返しカウンタ
					e->rep_c = 0;		// LOOP(旧REP)命令カウンタ
					e->t_rep = 0;		// 自動弾発射タイミング(0:自動発射せず)
					return;
				}
			break;

			case(ECLVECT_HP):	// HPL 割り込み
				//char buf[100];
				//sprintf(buf,"hp = %d",e->hp);
				//DebugOut(buf);
				if(e->hp <= e->Vect[ECLVECT_HP].value){
					e->cmd = e->Vect[ECLVECT_HP].vect;
					e->cmd_c = 0;		// コマンド繰り返しカウンタ
					e->rep_c = 0;		// LOOP(旧REP)命令カウンタ
					e->t_rep = 0;		// 自動弾発射タイミング(0:自動発射せず)
					//DebugOut("受理");
					return;
				}
			break;

			case(ECLVECT_TIMER):	// タイマー割り込み
				if(e->IntTimer > e->Vect[ECLVECT_TIMER].value){
					e->cmd = e->Vect[ECLVECT_TIMER].vect;
					e->cmd_c = 0;		// コマンド繰り返しカウンタ
					e->rep_c = 0;		// LOOP(旧REP)命令カウンタ
					e->t_rep = 0;		// 自動弾発射タイミング(0:自動発射せず)
					e->IntTimer = 0;	// この割り込み特有の初期化
					return;
				}
				else{
					e->IntTimer++;
				}
			break;

			default:
			break;
		}
	}
}

// 割り込みベクタの初期化 //
extern void InitECLInterrupt(ENEMY_DATA *e)
{
	int			i;

	for(i=0;i<ECLVECT_MAX;i++){
		e->Vect[i].vect = 0;
	}
}

// ECLCST_?? からその値に変換する //
static DWORD ID2Value(ENEMY_DATA *e,BYTE id)
{
	switch(id){
		// レジスタ指定の場合 //
		case(ECLCST_GR0):case(ECLCST_GR1):case(ECLCST_GR2):case(ECLCST_GR3):
		case(ECLCST_GR4):case(ECLCST_GR5):case(ECLCST_GR6):case(ECLCST_GR7):
		return (DWORD)e->GR[id];

		case(ECLCST_LCMD_D):	return (DWORD)e->l_cmd.d;		// レーザーコマンド(角度)
		case(ECLCST_LCMD_DW):	return (DWORD)e->l_cmd.dw;		// レーザーコマンド(角度差)
		case(ECLCST_LCMD_N):	return (DWORD)e->l_cmd.n;		// レーザーコマンド(本数)
		case(ECLCST_LCMD_C):	return (DWORD)e->l_cmd.c;		// レーザーコマンド(色)
		case(ECLCST_LCMD_L):	return (DWORD)e->l_cmd.l;		// レーザーコマンド(長さ)
		case(ECLCST_LCMD_V):	return (DWORD)e->l_cmd.v;		// レーザーコマンド(速度)

		case(ECLCST_TCMD_D):	return (DWORD)e->t_cmd.d;		// 弾コマンド(角度)
		case(ECLCST_TCMD_DW):	return (DWORD)e->t_cmd.dw;		// 弾コマンド(角度差)
		case(ECLCST_TCMD_N):	return (DWORD)e->t_cmd.n;		// 弾コマンド(個数)
		case(ECLCST_TCMD_NS):	return (DWORD)e->t_cmd.ns;		// 弾コマンド(連射数)
		case(ECLCST_TCMD_V):	return (DWORD)e->t_cmd.v;		// 弾コマンド(速度)
		case(ECLCST_TCMD_C):	return (DWORD)e->t_cmd.c;		// 弾コマンド(色)
		case(ECLCST_TCMD_A):	return (DWORD)e->t_cmd.a;		// 弾コマンド(加速度)
		case(ECLCST_TCMD_REP):	return (DWORD)e->t_cmd.rep;		// 弾コマンド(繰り返し)
		case(ECLCST_TCMD_VD):	return (DWORD)e->t_cmd.vd;		// 弾コマンド(角速度)

		case(ECLCST_ENEMY_X):	return (DWORD)e->x;			// 敵のＸ座標
		case(ECLCST_ENEMY_Y):	return (DWORD)e->y;			// 敵のＹ座標
		case(ECLCST_ENEMY_D):	return (DWORD)e->d;			// 敵の角度

		default:
		return 0;
	}
}

static void Indsort(WORD *ind,WORD *now)
{
	int i,temp,next;

	for(i=temp=next=0;i<(*now);i++){
		if(((Enemy+(*(ind+i)))->flag) & EF_DELETE){
			for(temp=i+1;temp<(*now);temp++){
				if(((Enemy+(*(ind+temp)))->flag & EF_DELETE)==0){
					next++;
					goto SEARCH_END;
				}
			}

			SEARCH_END:
			swap16(ind+i,ind+temp);
		}
		else next++;
	}

	*now = next;
}

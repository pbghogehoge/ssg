/*************************************************************************************************/
/*   ECLCDEF.H   ECLC用のコンバート情報の定義など                                                */
/*                                                                                               */
/*************************************************************************************************/

#include "..\\Gian07SrcFiles\\ECL.H"


#define CMDLEN_MAX 20

typedef enum{
	DT_U1,		// １バイトデータ
	DT_U2,		// ２バイトデータ
	DT_U4,		// ４バイトデータ
	DT_S1,		// １バイト符号有りデータ
	DT_S2,		// ２バイト符号有りデータ
	DT_S4,		// ４バイト符号有りデータ
	DT_JMP,		// ジャンプ命令のアドレス値
	DT_END		// コマンド終了
} ECLC_CMD;

typedef struct{
	char		*name;				// 置換前の文字列(コマンド)
	BYTE		cmd_val;			// コマンドの変換後の値
	ECLC_CMD	cmd[CMDLEN_MAX];	// コマンドの並び
} CONV_INFO;


// 記述するときの注意 : 繰り返し命令は符号無し２バイトとする //
CONV_INFO CMD_BUF[ECL_CMDMAX] = {
	{"SETUP",ECL_SETUP,DT_U4 ,DT_U4 ,DT_END},					// HP,SCORE
	{"END"  ,ECL_END  ,DT_END},
	{"JMP"  ,ECL_JMP  ,DT_JMP,DT_END},
	{"LOOP" ,ECL_LOOP ,DT_JMP,DT_U2 ,DT_END},
	{"CALL" ,ECL_CALL ,DT_JMP,DT_END},
	{"RET"  ,ECL_RET  ,DT_END},
	{"JHPL" ,ECL_JHPL ,DT_JMP,DT_U4 ,DT_END},
	{"JHPS" ,ECL_JHPS ,DT_JMP,DT_U4 ,DT_END},
	{"JDIF" ,ECL_JDIF ,DT_JMP,DT_JMP,DT_JMP,DT_JMP,DT_END},
	{"JDSB" ,ECL_JDSB ,DT_JMP,DT_END},
	{"JFCL" ,ECL_JFCL ,DT_JMP,DT_U4 ,DT_END},
	{"JFCS" ,ECL_JFCS ,DT_JMP,DT_U4 ,DT_END},
	{"STI"  ,ECL_STI  ,DT_JMP,DT_U1 ,DT_U4 ,DT_END},	// Addr,Flags,CmpValue
	{"CLI"  ,ECL_CLI  ,DT_U1 ,DT_END},					// Flags

	{"NOP"  ,ECL_NOP  ,DT_U2 ,DT_END},							// turn
	{"NOPSC",ECL_NOPSC,DT_U2 ,DT_END},							// turn
	{"MOV"  ,ECL_MOV  ,DT_U2 ,DT_END},							// turn
	{"ROL"  ,ECL_ROL  ,DT_S1 ,DT_U2 ,DT_END},					// vd,turn
	{"LROL" ,ECL_LROL ,DT_S4 ,DT_S4 ,DT_S1 ,DT_U2 ,DT_END},		// vx,vy,vd,turn
	{"WAVX" ,ECL_WAVX ,DT_S4 ,DT_U1 ,DT_S1 ,DT_U2 ,DT_END},		// vx,amp,vd,turn
	{"WAVY" ,ECL_WAVY ,DT_S4 ,DT_U1 ,DT_S1 ,DT_U2 ,DT_END},		// vy,amp,vd,turn
	{"MXA"  ,ECL_MXA  ,DT_S2 ,DT_U2 ,DT_END},					// x(Grp),turn
	{"MYA"  ,ECL_MYA  ,DT_S2 ,DT_U2 ,DT_END},					// y(Grp),turn
	{"MXYA" ,ECL_MXYA ,DT_S2 ,DT_S2 ,DT_U2 ,DT_END},			// x(Grp),y(Grp),turn
	{"MXS"  ,ECL_MXS  ,DT_U2 ,DT_END},							// turn
	{"MYS"  ,ECL_MYS  ,DT_U2 ,DT_END},							// turn
	{"MXYS" ,ECL_MXYS ,DT_U2 ,DT_END},							// turn

	{"DEGA" ,ECL_DEGA ,DT_U1 ,DT_END},
	{"DEGR" ,ECL_DEGR ,DT_S1 ,DT_END},
	{"DEGX" ,ECL_DEGX ,DT_END},
	{"DEGS" ,ECL_DEGS ,DT_END},
	{"SPDA" ,ECL_SPDA ,DT_U4 ,DT_END},
	{"SPDR" ,ECL_SPDR ,DT_S4 ,DT_END},
	{"XYA"  ,ECL_XYA  ,DT_S2 ,DT_S2 ,DT_END},
	{"XYR"  ,ECL_XYR  ,DT_S2 ,DT_S2 ,DT_END},
	{"DEGXU",ECL_DEGXU,DT_END},
	{"DEGXD",ECL_DEGXD,DT_END},
	{"DEGEX",ECL_DEGEX,DT_END},
	{"XYS"  ,ECL_XYS  ,DT_END},

	{"TAMA" ,ECL_TAMA ,DT_END},
	{"TAUTO",ECL_TAUTO,DT_U1 ,DT_END},			// rep
	{"TXYR" ,ECL_TXYR ,DT_S2 ,DT_S2 ,DT_END},	// x,y
	{"TCMD" ,ECL_TCMD ,DT_U1 ,DT_END},			// cmd
	{"TDEGA",ECL_TDEGA,DT_U1 ,DT_U1 ,DT_END},	// deg,deg_wid
	{"TDEGR",ECL_TDEGR,DT_S1 ,DT_S1 ,DT_END},	// deg,deg_wid
	{"TNUMA",ECL_TNUMA,DT_U1 ,DT_U1 ,DT_END},	// n,ns
	{"TNUMR",ECL_TNUMR,DT_S1 ,DT_S1 ,DT_END},	// n,ns
	{"TSPDA",ECL_TSPDA,DT_U1 ,DT_S1 ,DT_END},	// v,a
	{"TSPDR",ECL_TSPDR,DT_S1 ,DT_S1 ,DT_END},	// v,a (v はフラグ変化をしないものとする)
	{"TOPT" ,ECL_TOPT ,DT_U1 ,DT_END},			// option
	{"TTYPE",ECL_TTYPE,DT_U1 ,DT_END},			// type
	{"TCOL" ,ECL_TCOL ,DT_U1 ,DT_END},			// color
	{"TVDEG",ECL_TVDEG,DT_S1 ,DT_END},			// vd
	{"TREP" ,ECL_TREP ,DT_U1 ,DT_END},			// t_rep
	{"TDEGS",ECL_TDEGS,DT_END},					//
	{"TDEGE",ECL_TDEGE,DT_END},					//
	{"TAMA2",ECL_TAMA2,DT_END},
	{"TCLR" ,ECL_TCLR ,DT_END},

	{"LASER" ,ECL_LASER,DT_END},
	{"LCMD"  ,ECL_LCMD ,DT_U1 ,DT_END},			// cmd
	{"LLA"   ,ECL_LLA  ,DT_S4 ,DT_END},			// l
	{"LLR"   ,ECL_LLR  ,DT_S4 ,DT_END},			// l
	{"LL2"   ,ECL_LL2  ,DT_S4 ,DT_END},			// l2
	{"LDEGA" ,ECL_LDEGA,DT_U1 ,DT_U1 ,DT_END},	// d,dw
	{"LDEGR" ,ECL_LDEGR,DT_S1 ,DT_S1 ,DT_END},	// d,dw
	{"LNUMA" ,ECL_LNUMA,DT_U1 ,DT_END},			// n(速射はないでしょ...)
	{"LNUMR" ,ECL_LNUMR,DT_S1 ,DT_END},			// n
	{"LSPDA" ,ECL_LSPDA,DT_S4 ,DT_END},			// v(加速はないでしょ...)
	{"LSPDR" ,ECL_LSPDR,DT_S4 ,DT_END},			// v
	{"LCOL"  ,ECL_LCOL ,DT_U1 ,DT_END},			// color
	{"LTYPE" ,ECL_LTYPE,DT_U1 ,DT_END},			// type
	{"LWA"   ,ECL_LWA  ,DT_S4 ,DT_END},			// width
	{"LDEGS" ,ECL_LDEGS,DT_END},				//
	{"LDEGE" ,ECL_LDEGE,DT_END},				//
	{"LXY"   ,ECL_LXY  ,DT_S2 ,DT_S2 ,DT_END},	// x,y
	{"LASER2",ECL_LASER2,DT_END},

	{"LLSET"	,ECL_LLSET		,DT_END},
	{"LLOPEN"	,ECL_LLOPEN		,DT_U1,DT_END},				// LLaserID
	{"LLCLOSE"	,ECL_LLCLOSE	,DT_U1,DT_END},				// LLaserID
	{"LLCLOSEL"	,ECL_LLCLOSEL	,DT_U1,DT_END},				// LLaserID
	{"LLDEGR"	,ECL_LLDEGR		,DT_U1,DT_S1	,DT_END},	// LLaserID,Degree(Rel)

	{"DRAW_ON"   ,ECL_DRAW_ON   ,DT_END},
	{"DRAW_OFF"  ,ECL_DRAW_OFF  ,DT_END},
	{"CLIP_ON"   ,ECL_CLIP_ON   ,DT_END},
	{"CLIP_OFF"  ,ECL_CLIP_OFF  ,DT_END},
	{"DAMAGE_ON" ,ECL_DAMAGE_ON ,DT_END},
	{"DAMAGE_OFF",ECL_DAMAGE_OFF,DT_END},
	{"HITSB_ON"  ,ECL_HITSB_ON  ,DT_END},
	{"HITSB_OFF" ,ECL_HITSB_OFF ,DT_END},
	{"RLCHG_ON"  ,ECL_RLCHG_ON  ,DT_END},
	{"RLCHG_OFF" ,ECL_RLCHG_OFF ,DT_END},

	{"ANM"     ,ECL_ANM     ,DT_U1 ,DT_S1 ,DT_END},			// ptn,spd
	{"PSE"     ,ECL_PSE     ,DT_U1 ,DT_END},				// sound_no
	{"INT"     ,ECL_INT     ,DT_U1 ,DT_END},				// 割り込み番号
	{"EXDEGD"  ,ECL_EXDEGD  ,DT_U1 ,DT_END},				// EXDEG 用の角度増分
	{"ENEMYSET",ECL_ENEMYSET,DT_S2 ,DT_S2 ,DT_S1 ,DT_END},	// dx,dy,EnemyID

	{"MOVR",ECL_MOVR,DT_U1 ,DT_U1 ,DT_END},		// Reg/StID , Reg/StID
	{"MOVC",ECL_MOVC,DT_U1 ,DT_S4 ,DT_END},		// Reg , Const(32Bit)
	{"ADD" ,ECL_ADD ,DT_U1 ,DT_U1 ,DT_END},		// Reg0,Reg1  : Reg0 = Reg0+Reg1
	{"SUB" ,ECL_SUB ,DT_U1 ,DT_U1 ,DT_END},		// Reg0,Reg1  : Reg0 = Reg0-Reg1
	{"SINL",ECL_SINL,DT_U1 ,DT_U1 ,DT_END},		// Reg0,Reg1  : Reg0 = Reg0*sin[Reg1%256]
	{"COSL",ECL_COSL,DT_U1 ,DT_U1 ,DT_END},		// Reg0,Reg1  : Reg0 = Reg0*cos[Reg1%256]
	{"MOD" ,ECL_MOD ,DT_U1 ,DT_U4 ,DT_END},		// Reg0,Const : Reg0 = Reg0%(Const)
	{"RND" ,ECL_RND ,DT_U1 ,DT_END},			// Reg0       : Reg0 = rnd()
	{"CMPR",ECL_CMPR,DT_U1 ,DT_U1 ,DT_END},		// Reg0,Reg1     : Signed Compare Reg0,Reg1
	{"CMPC",ECL_CMPC,DT_U1 ,DT_S4 ,DT_END},		// Reg,Const(32) : Signed Compare Reg,Const
	{"JL"  ,ECL_JL  ,DT_JMP,DT_END},			// JMP if CmpResult > 0
	{"JS"  ,ECL_JS  ,DT_JMP,DT_END},			// JMP if CmpResult < 0
	{"INC" ,ECL_INC ,DT_U1 ,DT_END},			// Reg : Reg++
	{"DEC" ,ECL_DEC ,DT_U1 ,DT_END},			// Reg : Reg--
};

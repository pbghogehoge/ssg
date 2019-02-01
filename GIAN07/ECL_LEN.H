/*
 * 敵の命令長定義ファイル
 * 作成日時    01/31/01  01:34:57
 */


const BYTE ECL_CmdLen[256] = {
	    9,		// SETUP
	    1,		// END
	    5,		// JMP
	    7,		// LOOP
	    5,		// CALL
	    1,		// RET
	    9,		// JHPL
	    9,		// JHPS
	   17,		// JDIF
	    5,		// JDSB
	    9,		// JFCL
	    9,		// JFCS
	   10,		// STI
	    2,		// CLI
	    0,		//
	    0,		//
	    3,		// NOP
	    3,		// NOPSC
	    3,		// MOV
	    4,		// ROL
	   12,		// LROL
	    9,		// WAVX
	    9,		// WAVY
	    5,		// MXA
	    5,		// MYA
	    7,		// MXYA
	    3,		// MXS
	    3,		// MYS
	    3,		// MXYS
	    4,		// ACC
	    7,		// ACCXYA
	    2,		// GRAX
	    2,		// DEGA
	    2,		// DEGR
	    1,		// DEGX
	    1,		// DEGS
	    5,		// SPDA
	    5,		// SPDR
	    5,		// XYA
	    5,		// XYR
	    1,		// DEGXU
	    1,		// DEGXD
	    1,		// DEGEX
	    1,		// XYS
	    1,		// DEGX2
	    1,		// XYRND
	    3,		// XYL
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    1,		// TAMA
	    2,		// TAUTO
	    5,		// TXYR
	    2,		// TCMD
	    3,		// TDEGA
	    3,		// TDEGR
	    3,		// TNUMA
	    3,		// TNUMR
	    3,		// TSPDA
	    3,		// TSPDR
	    2,		// TOPT
	    2,		// TTYPE
	    2,		// TCOL
	    2,		// TVDEG
	    2,		// TREP
	    1,		// TDEGS
	    1,		// TDEGE
	    1,		// TAMA2
	    1,		// TCLR
	    1,		// TAMAL
	    2,		// T2ITEM
	    1,		// TAMAEX
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    1,		// LASER
	    2,		// LCMD
	    5,		// LLA
	    5,		// LLR
	    5,		// LL2
	    3,		// LDEGA
	    3,		// LDEGR
	    2,		// LNUMA
	    2,		// LNUMR
	    5,		// LSPDA
	    5,		// LSPDR
	    2,		// LCOL
	    2,		// LTYPE
	    5,		// LWA
	    1,		// LDEGS
	    1,		// LDEGE
	    5,		// LXY
	    1,		// LASER2
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    1,		// LLSET
	    2,		// LLOPEN
	    2,		// LLCLOSE
	    2,		// LLCLOSEL
	    3,		// LLDEGR
	    1,		// HLASER
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    1,		// DRAW_ON
	    1,		// DRAW_OFF
	    1,		// CLIP_ON
	    1,		// CLIP_OFF
	    1,		// DAMAGE_ON
	    1,		// DAMAGE_OFF
	    1,		// HITSB_ON
	    1,		// HITSB_OFF
	    1,		// RLCHG_ON
	    1,		// RLCHG_OFF
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    3,		// ANM
	    2,		// PSE
	    2,		// INT
	    2,		// EXDEGD
	    6,		// ENEMYSET
	    7,		// ENEMYSETD
	    5,		// HITXY
	    2,		// ITEM
	    2,		// STG4EFC
	    2,		// ANMEX
	    2,		// BITLASER
	    5,		// BITATTACK
	    6,		// BITCMD
	    2,		// BOSSSET
	    6,		// CEFC
	    1,		// STG3EFC
	    3,		// MOVR
	    6,		// MOVC
	    3,		// ADD
	    3,		// SUB
	    3,		// SINL
	    3,		// COSL
	    6,		// MOD
	    2,		// RND
	    3,		// CMPR
	    6,		// CMPC
	    5,		// JL
	    5,		// JS
	    2,		// INC
	    2,		// DEC
	    5,		// JEQ
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0,		//
	    0 		//
};

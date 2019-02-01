/*                                                                           */
/*   PRankCtrl.cpp   プレイランク管理                                        */
/*                                                                           */
/*                                                                           */

#include "PRankCtrl.h"

PlayRankInfo	PlayRank;



// 難易度の許容範囲内でプレイランクを増減する
FVOID PlayRankAdd(int n)
{
	// イージー 　　　0 ～ 24
	// ノーマル　　　16 ～ 40
	// ハード　　 　　32 ～ 48
	// ルナティック  40 ～ 64

	// 難易度を変化させる //
	if(GameStage == GRAPH_ID_EXSTAGE){
		if(n){
			if(n > 0) PlayRank.Rank += max(1, n/4);
			else      PlayRank.Rank += min(-1, n/10);
		}
	}
	else{
		PlayRank.Rank += n;
	}

	// この分岐に関しては、基本的にコンフィグの値に基づく //
	switch(ConfigDat.GameLevel){
		case(GAME_EASY):
			if     (PlayRank.Rank < 0)      PlayRank.Rank = 0;
			else if(PlayRank.Rank > 24*256) PlayRank.Rank = 24*256;

			if(PlayRank.Rank < 20*256) PlayRank.GameLevel = GAME_EASY;
			else                       PlayRank.GameLevel = GAME_NORMAL;
		break;

		case(GAME_NORMAL):
			if     (PlayRank.Rank < 16*256) PlayRank.Rank = 16*256;
			else if(PlayRank.Rank > 40*256) PlayRank.Rank = 40*256;

			if     (PlayRank.Rank < 20*256) PlayRank.GameLevel = GAME_EASY;
			else if(PlayRank.Rank < 36*256) PlayRank.GameLevel = GAME_NORMAL;
			else                            PlayRank.GameLevel = GAME_HARD;
		break;

		case(GAME_HARD):
			if     (PlayRank.Rank < 32*256) PlayRank.Rank = 32*256;
			else if(PlayRank.Rank > 48*256) PlayRank.Rank = 48*256;

			if     (PlayRank.Rank < 36*256) PlayRank.GameLevel = GAME_NORMAL;
			else if(PlayRank.Rank < 44*256) PlayRank.GameLevel = GAME_HARD;
			else                            PlayRank.GameLevel = GAME_LUNATIC;
		break;

		case(GAME_LUNATIC):
			if     (PlayRank.Rank < 40*256) PlayRank.Rank = 40*256;
			else if(PlayRank.Rank > 64*256) PlayRank.Rank = 64*256;

			if(PlayRank.Rank < 44*256) PlayRank.GameLevel = GAME_HARD;
			else                       PlayRank.GameLevel = GAME_LUNATIC;
		break;

		//case(GAME_EXTRA):
		//break;
	}
}


// 現在の難易度に応じてプレイランクを初期化
FVOID PlayRankReset(void)
{
	PlayRank.GameLevel = ConfigDat.GameLevel;

	switch(ConfigDat.GameLevel){
		case(GAME_EASY):		PlayRank.Rank = 12*256;		break;
		case(GAME_NORMAL):		PlayRank.Rank = 28*256;		break;
		case(GAME_HARD):		PlayRank.Rank = 40*256;		break;
		case(GAME_LUNATIC):		PlayRank.Rank = 52*256;		break;
		//case(GAME_EXTRA):		break;
	}
}

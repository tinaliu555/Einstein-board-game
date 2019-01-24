#ifndef NODE_HH
#define NODE_HH

#include <random>
#include <iostream>
// #include <math.h>
#include "mcts.h"
// class Nodes;

// float my_sqrt(const float x)  
// {
//   union
//   {
//     int i;
//     float x;
//   } u;
//   u.x = x;
//   u.i = (1<<29) + (u.i >> 1) - (1<<22); 

//   u.x = u.x + x/u.x;
//   u.x = 0.25f*u.x + x/u.x;

//   return u.x;
// } 
using namespace std;
#define const_C 8
class Node
{
public:
	Board board;
	int moveLen;
	int childrenIndex;
	int pIndex;
	int pMovIndex;
	bool flag;

	// unsigned long long int  wins;
	unsigned long long int visits;
	long long int score;
	long long int scoreSum2;
	long double averageScore;
	long double SD;

	bool isMaxNode;
	void initialize(const Board& state,bool isMax){
		board = state;
		childrenIndex = -1;
		// wins = 0;
		visits = 0;
		score = 0;
		scoreSum2 = 0;
		isMaxNode = isMax;
		flag = true;
	}
	void doMoveAndSetMove(int no, int dir){
		board.do_move(no,dir);
		setMoveLen();
	}
	void setMoveLen(){
		moveLen = 0;
		if(board.winner() != Color::other)
			return;
		for(int no=1; no<=6; ++no) for(int dir=1; dir<=3; ++dir) if(board.new_pos(no, dir)){
			moveLen++;
		}
		if(moveLen==0){
			moveLen = 1;
		}
	}
	void setpIndexs(const int index, const int moveIndex){
		pIndex = index;
		pMovIndex = moveIndex;
	}
	void setAvgScore(){
		averageScore = (long double)score/visits;
	}
	void setSD(){
		// cerr<<" scoreSum2:"<<scoreSum2<<"/ visits:"<<visits<<" - averageScore:"<<averageScore<<endl;
		// cerr<<"= "<<(long double)scoreSum2/visits<<" - "<< averageScore * averageScore<<endl;
		SD = sqrt((long double)scoreSum2/visits - averageScore * averageScore);
	}
	long double setUCT(const unsigned long long int N){
		// if(isMaxNode)
		// 	UCT_score = 1 - ((long double)wins/visits) + const_C * sqrt(log(N)/visits);
		// else
		// 	UCT_score = ((long double)wins/visits) + const_C * sqrt(log(N)/visits);
		if(isMaxNode)
			UCT_score = - ((long double)score/visits) + const_C * sqrt(log(N)/visits);
		else
			UCT_score = ((long double)score/visits) + const_C * sqrt(log(N)/visits);

		return UCT_score;
	}
	long double getUCT(){
		return UCT_score;
	}

	void updateWithPropagateWins(unsigned long long int  newVisits, unsigned long long int  newWins ,long long int newScores, long long int newScoreSum2){
		visits += newVisits;
		// wins += newWins;
		score += newScores;
		scoreSum2 += newScoreSum2;
		return;
	}

	void updateWithPropagate(unsigned long long int  newVisits,long long int newScores, long long int newScoreSum2){
		visits += newVisits;
		score += newScores;
		scoreSum2 += newScoreSum2;
		return;
	}

	Node& operator = (const Node&);
	// bool operator <(const Node& d) {
	// 	return UCT_score<d.UCT_score;
	// }
	void print(int i,int j){
		cerr<<"-------------All information of a Node------------["<<i<<","<<j<<"]"<<endl;
		cerr<<board<<endl;
		// cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<endl;
		// cerr<<"[wins]:"<<wins<<", [visits]:"<<visits<<", [nodeLevel]:"<<nodeLevel<<", [UCT_score]:"<<UCT_score<<endl<<endl;
		cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<", [visits]:"<<visits<<", [Score]:"<<score<<", [Score2]:"<<scoreSum2<<", [SD]:"<<SD<<", [isMaxNode]:"<<isMaxNode<<", [UCT_score]:"<<UCT_score<<endl<<endl;
		// cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<", [wins]:"<<wins<<", [visits]:"<<visits<<", [Score]:"<<score<<", [isMaxNode]:"<<isMaxNode<<", [UCT_score]:"<<UCT_score<<endl<<endl;
	}
	void printInfo(int i,int j){
		cerr<<"-------------All information of a Node------------["<<i<<","<<j<<"]"<<endl;
		// cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<endl;
		// cerr<<"[wins]:"<<wins<<", [visits]:"<<visits<<", [nodeLevel]:"<<nodeLevel<<", [UCT_score]:"<<UCT_score<<endl<<endl;
		// cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<", [wins]:"<<wins<<", [visits]:"<<visits<<", [isMaxNode]:"<<isMaxNode<<", [UCT_score]:"<<UCT_score<<endl<<endl;
		cerr<<"[moveLen]:"<<moveLen<<", [childrenIndex]:"<<childrenIndex<<", [pIndex]:"<<pIndex<<", [pMovIndex]:"<<pMovIndex<<", [visits]:"<<visits<<", [Score]:"<<score<<", [Score2]:"<<scoreSum2<<", [SD]:"<<SD<<", [isMaxNode]:"<<isMaxNode<<", [UCT_score]:"<<UCT_score<<endl<<endl;
	}

private:

	long double UCT_score;
};

#endif
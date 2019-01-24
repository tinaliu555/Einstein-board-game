#ifndef MCTS_HH
#define MCTS_HH

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include "Node.hh"

#include <omp.h>

#define maxCalculateTime 9.65
#define maxMTCSIterations 1000000
#define Iterations 750
#define sigmaE 1.5
#define Rd 1
using namespace std;


static Node allNode[30000][9];
int rootIndex = 0, rootIndex2 = 0;
int allNodePtr = 0;
unsigned long long int totalStimulate = 0;
double startTime = 0, limitEndTime;

int uniform_int_rand(int l, int r){
    static mt19937_64 Random_engine(random_device{}());
    uniform_int_distribution<int> Ulr(l, r);
    return Ulr(Random_engine);
}

void InitialValue(const Board& root_state, const int len){
	rootIndex = 0, rootIndex2 = 0;
	allNodePtr = 0;
	allNode[rootIndex][rootIndex2].initialize(root_state,true);
	allNode[rootIndex][rootIndex2].moveLen = len;
	allNode[rootIndex][rootIndex2].setpIndexs(-1,-1);
	return;
}

void Do_ProgressivePruning(const int CandidateIndex,const int CandidateLen, const bool isMaxNode){
	if(CandidateLen<=1)//this is the only one move left for its parent
		return;
	int isAbleToTestIndex[CandidateLen], TestLen = -1;

	//Update Avg & SD
	for(int i=0;i<CandidateLen;i++){
		if(!allNode[CandidateIndex][i].flag)
			continue;
		allNode[CandidateIndex][i].setAvgScore();
		allNode[CandidateIndex][i].setSD();
		if(allNode[CandidateIndex][i].SD < sigmaE)
		{
			TestLen++;
			isAbleToTestIndex[TestLen] = i;
		}
	}
	if(isMaxNode){
		double lowerBound = -10000, temp;
		for(int i=0;i<=TestLen;i++){
			temp = allNode[CandidateIndex][isAbleToTestIndex[i]].averageScore - allNode[CandidateIndex][isAbleToTestIndex[i]].SD * Rd;
			if(temp>lowerBound)
				lowerBound = temp;
		}
		for(int i=0;i<=TestLen;i++){
			if(allNode[CandidateIndex][isAbleToTestIndex[i]].averageScore + allNode[CandidateIndex][isAbleToTestIndex[i]].SD * Rd < lowerBound){
				allNode[CandidateIndex][isAbleToTestIndex[i]].flag = false;
				// cerr<<"Pruning in max node, lowerBound:"<<lowerBound<<endl;
				// allNode[CandidateIndex][isAbleToTestIndex[i]].print(CandidateIndex,isAbleToTestIndex[i]);
			}
		}
	}
	else{
		double upperBound = 10000, temp;
		for(int i=0;i<=TestLen;i++){
			temp = allNode[CandidateIndex][isAbleToTestIndex[i]].averageScore + allNode[CandidateIndex][isAbleToTestIndex[i]].SD * Rd;
			if(temp<upperBound)
				upperBound = temp;
		}
		for(int i=0;i<=TestLen;i++){
			if(allNode[CandidateIndex][isAbleToTestIndex[i]].averageScore - allNode[CandidateIndex][isAbleToTestIndex[i]].SD * Rd > upperBound){
				allNode[CandidateIndex][isAbleToTestIndex[i]].flag = false;
				// cerr<<"Pruning in min node, upperBound:"<<upperBound<<endl;
				// allNode[CandidateIndex][isAbleToTestIndex[i]].print(CandidateIndex,isAbleToTestIndex[i]);
			}
		}
	}
	
}

void compute_tree(const bool isFirst_ply)
{
	totalStimulate = 0;
	int thisIndex = rootIndex,thisIndex2 = rootIndex2;

	for(int mtcs=0; mtcs<maxMTCSIterations; mtcs++){
		// cerr<<"Round:"<<mtcs<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
		//selection
		thisIndex = rootIndex, thisIndex2 = rootIndex2;
		int childPos = allNode[thisIndex][thisIndex2].childrenIndex;
		int thisMoveLen = allNode[thisIndex][thisIndex2].moveLen;
		while(childPos!=-1){// no child
			//Progressive Pruning
			int notPruneNum = 0;
			for(int i=0;i<thisMoveLen;i++){
				if(allNode[childPos][i].flag)
					notPruneNum++;
			}
			Do_ProgressivePruning(childPos,notPruneNum,allNode[thisIndex][thisIndex2].isMaxNode);

			// double maxNum = allNode[childPos][0].setUCT(allNode[thisIndex][thisIndex2].visits), selNum;
			double maxNum = -numeric_limits<double>::max(), selNum;
			int maxIndex = -1;
			for(int i=0;i<thisMoveLen;i++){
				if(allNode[childPos][i].flag){
					selNum = allNode[childPos][i].setUCT(allNode[thisIndex][thisIndex2].visits);
					if(maxNum<selNum){
						maxNum = selNum;
						maxIndex = i;
					}
				}
			}
			if(maxIndex==-1)
			{
				maxIndex = 0;
			}
			thisIndex = childPos;
			thisIndex2 = maxIndex;
			childPos = allNode[thisIndex][thisIndex2].childrenIndex;
			thisMoveLen = allNode[thisIndex][thisIndex2].moveLen;
		}

		//expansion
		// cerr<<"expansion~~~~~~~~~~~~~~~~~~~"<<endl;
		unsigned long long int  newVisits;//, newWins=0;
		long long int newScore = 0, newScoreSum2 = 0;
		long long int tempScore;
		int pId, pId2;//, N = 0;
		if(thisMoveLen==0){//reach leaf
			// cerr<<"simulation~~~~~~~~~~~~~~~~it is a leaf"<<endl;
			if(isFirst_ply){
				if(allNode[thisIndex][thisIndex2].board.winner()==Color::red){
					// allNode[thisIndex][thisIndex2].wins += Iterations;
					// newWins = Iterations;
					tempScore = (6 - allNode[thisIndex][thisIndex2].board.get_num_i(1));
				}
				else
					tempScore = (- 6 + allNode[thisIndex][thisIndex2].board.get_num_i(0));
			}
			else{
				if(allNode[thisIndex][thisIndex2].board.winner()==Color::blue){
					// allNode[thisIndex][thisIndex2].wins += Iterations;
					// newWins = Iterations;
					tempScore = (6 - allNode[thisIndex][thisIndex2].board.get_num_i(0));
				}
				else
					tempScore = (- 6 + allNode[thisIndex][thisIndex2].board.get_num_i(1));
			}
			newScore = tempScore * Iterations;
			newScoreSum2 = (tempScore * tempScore) * Iterations;

			allNode[thisIndex][thisIndex2].score += newScore;
			allNode[thisIndex][thisIndex2].scoreSum2 += newScoreSum2;
			allNode[thisIndex][thisIndex2].visits += Iterations;
			newVisits = Iterations;
			totalStimulate += Iterations;
			pId = allNode[thisIndex][thisIndex2].pIndex;
			pId2 = allNode[thisIndex][thisIndex2].pMovIndex;
		}
		else{
			// cerr<<"simulation~~~~~~~~~~~~~~~~~~~start, thisMoveLen:"<<thisMoveLen<<endl;
			allNodePtr++;
			allNode[thisIndex][thisIndex2].childrenIndex = allNodePtr;

			int tempLen = 0;
			int moves[9][2];
			for(int no=1; no<=6; ++no) for(int dir=1; dir<=3; ++dir) if(allNode[thisIndex][thisIndex2].board.new_pos(no, dir)){
				moves[tempLen][0] = no;
				moves[tempLen][1] = dir;
				tempLen ++;
			}
			if(tempLen==0){
				moves[0][0] = 0; moves[0][1] = 0;
				tempLen = 1;
			}

			for(int i=0;i<thisMoveLen;i++){
				allNode[allNodePtr][i].initialize(allNode[thisIndex][thisIndex2].board,!allNode[thisIndex][thisIndex2].isMaxNode);
				allNode[allNodePtr][i].doMoveAndSetMove(moves[i][0],moves[i][1]);
				allNode[allNodePtr][i].setpIndexs(thisIndex,thisIndex2);
			}
			

			//simulation

			int randomNum;
			// cerr<<"simulation~~~~~~~~~~~~~~~~~~~allNodePtr:"<<allNodePtr<<endl;
			for(int i=0;i<thisMoveLen;i++){
				int ml[9][2];
				// unsigned long long int win=0;
				long long int score=0,scoreSum2=0;
				// int temp = 0;
				for(int j=0;j<Iterations;j++){
					auto simuNode = allNode[allNodePtr][i];

					while(simuNode.board.winner()==Color::other){
						int mlLen=0;
				        for(int no=1; no<=6; ++no) for(int dir=1; dir<=3; ++dir) if(simuNode.board.new_pos(no, dir)){
				            ml[mlLen][0] = no;
				            ml[mlLen][1] = dir;
				            mlLen++;
				        }
				        if(mlLen==0){
				            // ml[0][0] = 0; ml[0][1] = 0;
				            simuNode.board.do_move(0, 0);
				        }
				        else{
				        	randomNum = uniform_int_rand(0, mlLen-1);
				        	simuNode.board.do_move(ml[randomNum][0], ml[randomNum][1]);
				        }
					}
					long long int tempScore;
					if(isFirst_ply)
					{
						if(simuNode.board.winner()==Color::red){
							// win++;
							tempScore = 6 - simuNode.board.get_num_i(1);
						}
						else{
							tempScore =  - 6 + simuNode.board.get_num_i(0);
						}
					}
					else
					{
						if(simuNode.board.winner()==Color::blue){
							// win++;
							tempScore =  6 - simuNode.board.get_num_i(0);
						}
						else{
							tempScore =  - 6 + simuNode.board.get_num_i(1);
						}
						
					}
					score += tempScore;
					scoreSum2 += tempScore * tempScore;
				}
				allNode[allNodePtr][i].score += score;
				allNode[allNodePtr][i].scoreSum2 += scoreSum2;
				// allNode[allNodePtr][i].wins += win;
				allNode[allNodePtr][i].visits += Iterations;
				// newWins += win;
				newScore += score;
				newScoreSum2 += scoreSum2;
			}
			newVisits = Iterations * thisMoveLen;
			totalStimulate += newVisits;
			pId = thisIndex, pId2 = thisIndex2;
		}

		//propagation
			// cerr<<"Propagation!--------------------------------------------------"<<endl;
		while(pId>=rootIndex){

			int ppId = allNode[pId][pId2].pIndex, ppId2 = allNode[pId][pId2].pMovIndex;
			allNode[pId][pId2].updateWithPropagate(newVisits, newScore, newScoreSum2);
			pId = ppId, pId2 = ppId2;
		}
		//Check TIme
		double nowTime = ::omp_get_wtime();
		if(nowTime>limitEndTime){
			// cerr<<"Time's out ! mtcs:"<<mtcs<<", iteration:"<<totalStimulate<<", useTim:"<<(nowTime-startTime)<<endl;
			return;
		}
		
	}
	// cerr<<"Finish compute tree rootIndex:"<<rootIndex<<", rootIndex2:"<<rootIndex2<<", allNodePtr:"<<allNodePtr<<endl;
	return;
}

std::pair<int, int> compute_move(const Board& root_state, const bool isFirst_ply, const int findotherMoveIndex)
{
	// cerr<<"Start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
	startTime = ::omp_get_wtime();
	limitEndTime = startTime + maxCalculateTime;
	// auto moves = root_state.move_list();
	unsigned int thisMoveLen=0, moves[9][2];
	for(int no=1; no<=6; ++no) for(int dir=1; dir<=3; ++dir) if(root_state.new_pos(no, dir)){
		moves[thisMoveLen][0] = no;
		moves[thisMoveLen][1] = dir;
		thisMoveLen++;
	}
	if(thisMoveLen==0){
		moves[0][0] = 0; moves[0][1] = 0;
		thisMoveLen = 1;
	}

	if(findotherMoveIndex!=-1){
		int tempChild = allNode[rootIndex][rootIndex2].childrenIndex;
		
		if(tempChild==-1){
			// cerr<<"Special for the child not expend"<<endl;
			allNodePtr++;
			for(unsigned int i=0;i<thisMoveLen;i++){
				allNode[allNodePtr][i].initialize(allNode[rootIndex][rootIndex2].board,!allNode[rootIndex][rootIndex2].isMaxNode);
				allNode[allNodePtr][i].doMoveAndSetMove(moves[i][0],moves[i][1]);
				allNode[allNodePtr][i].setpIndexs(rootIndex,rootIndex2);
			}
			tempChild = allNodePtr;
		}
		rootIndex = tempChild;
		rootIndex2 = findotherMoveIndex;

	}else{//Special for the "first move" of the board
		InitialValue(root_state,thisMoveLen);
		if(!isFirst_ply){
			// cerr<<"Special for the first move of the board"<<endl;
			rootIndex = 1; rootIndex2 = 0;
			allNodePtr = 1;

			allNode[0][0].childrenIndex = 1;
			allNode[rootIndex][rootIndex2].initialize(root_state,true);
			allNode[rootIndex][rootIndex2].moveLen = thisMoveLen;
			allNode[rootIndex][rootIndex2].setpIndexs(0,0);
		}
	}

	
	// cerr<<"rootIndex:"<<rootIndex<<", rootIndex2:"<<rootIndex2<<endl;

	compute_tree(isFirst_ply);
	int childId = allNode[rootIndex][rootIndex2].childrenIndex;
	double maxNum = allNode[childId][0].setUCT(allNode[rootIndex][rootIndex2].visits), selNum;
	int maxIndex = 0;
	for(unsigned int i=1;i<thisMoveLen;i++){
		selNum = allNode[childId][i].setUCT(allNode[rootIndex][rootIndex2].visits);
		if(maxNum<selNum){
			maxNum = selNum;
			maxIndex = i;
		}
	}
	// cerr<<"Final answer~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<isFirst_ply<<endl;
	
	// cerr<<endl;
	// allNode[rootIndex][rootIndex2].print(rootIndex,rootIndex2);
	
	// for(unsigned int i=0;i<thisMoveLen;i++){
	// 	allNode[childId][i].print(childId,i);
	// }
	
	// cerr<<"Give answer time:"<<::omp_get_wtime()-startTime<<", iterations:"<<totalStimulate<<", allNodePtr:"<<allNodePtr<<endl;
	rootIndex = allNode[rootIndex][rootIndex2].childrenIndex;
	rootIndex2 = maxIndex;
	// cerr<<"change to rootIndex:"<<rootIndex<<", rootIndex2:"<<rootIndex2<<endl;
	return make_pair(moves[maxIndex][0],moves[maxIndex][1]);

}

#endif

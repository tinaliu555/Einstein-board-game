#include<algorithm>
#include<iostream>
#include"einstein.hh"
// #include"Node.hh"
#include"mcts.h"
using namespace std;

int main(){
    while(1){
        // cerr<<"Start new game------------------------"<<endl;
        char player;
        cin >> player;
        if(player == 'e') // end game
            break;
        char arr[7]{};
        for(int i=0; i<=5; ++i)
            cin >> arr[i];

        Board b(arr); // initialize the board
        
        int findotherMove = -1;
        unsigned int OtherMoveLen=0;
        int otherMoves[9][2];
        for(bool first_ply = (player=='f');; first_ply = false){
            if(!first_ply){
                char prev_no, prev_dir;
                cin >> prev_no >> prev_dir;
                if(prev_no == 'w' or prev_no == 'l') // win or lose
                    break;
                auto otherNo = prev_no-'0',otherDir = prev_dir-'0';
                b.do_move(otherNo, otherDir);

                for(unsigned int i=0;i<OtherMoveLen;i++){
                    if(otherNo==otherMoves[i][0] && otherDir==otherMoves[i][1])
                        findotherMove=i;
                }
                // cerr<<"otherNo:"<<otherNo<<", otherDir:"<<otherDir<<", findotherMove:"<<findotherMove<<", otherMoves.size():"<<(otherMoves.size())<<endl;
            }
            
            std::pair<int, int> move = compute_move(b,player=='f',findotherMove);
            b.do_move(move.first, move.second); // do a move
            cout << move.first << move.second << flush; // print the move
            // otherMoves = b.move_list();
            OtherMoveLen = 0;
            for(int no=1; no<=6; ++no) for(int dir=1; dir<=3; ++dir) if(b.new_pos(no, dir)){
                otherMoves[OtherMoveLen][0] = no;
                otherMoves[OtherMoveLen][1] = dir;
                OtherMoveLen++;
            }
            if(OtherMoveLen==0){
                otherMoves[0][0] = 0; otherMoves[0][1] = 0;
                OtherMoveLen = 1;
            }
        }
        
    }
    return 0;
}

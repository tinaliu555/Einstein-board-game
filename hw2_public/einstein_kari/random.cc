#include<iostream>
#include<random>
#include"einstein.hh"

using namespace std;

int uniform_int_rand(int l, int r){
    static mt19937_64 Random_engine(random_device{}());
    uniform_int_distribution<int> Ulr(l, r);
    return Ulr(Random_engine);
}

int main(){
    while(1){
        char player;
        cin >> player;
        if(player == 'e') // end game
            break;
        char arr[7]{};
        for(int i=0; i<=5; ++i)
            cin >> arr[i];
        Board board(arr); // initialize the board
        for(bool first_ply = (player=='f');; first_ply = false){
            if(!first_ply){
                char prev_no, prev_dir;
                cin >> prev_no >> prev_dir;
                if(prev_no == 'w' or prev_no == 'l') // win or lose
                    break;
                board.do_move(prev_no-'0', prev_dir-'0');
            }
            auto ml = board.move_list();
            // randomly choose a move
            auto move = ml[uniform_int_rand(0, static_cast<int>(ml.size())-1)];
            board.do_move(move.first, move.second); // do a move
            cout << move.first << move.second << flush; // print the move
        }
    }
    return 0;
}

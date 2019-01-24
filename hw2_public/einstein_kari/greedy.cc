#include<algorithm>
#include<iostream>
#include"einstein.hh"

using namespace std;

int main(){
    while(1){
        char player;
        cin >> player;
        if(player == 'e') // end game
            break;
        char arr[7]{};
        for(int i=0; i<=5; ++i)
            cin >> arr[i];
        Board b(arr); // initialize the board
        for(bool first_ply = (player=='f');; first_ply = false){
            if(!first_ply){
                char prev_no, prev_dir;
                cin >> prev_no >> prev_dir;
                if(prev_no == 'w' or prev_no == 'l') // win or lose
                    break;
                b.do_move(prev_no-'0', prev_dir-'0');
            }
            auto ml = b.move_list();
            if(ml.size() == 1ull){
                b.do_move(ml[0].first, ml[0].second);
                cout << ml[0].first << ml[0].second << flush;
                continue;
            }
            vector<tuple<int, int, int, int>> dist;
            for(auto const &move: ml){
                auto b2 = b;
                b2.do_move(move.first, move.second);
                int op_num = b2.num_cubes(b2.turn()); // # of opponent's cubes
                auto p = b2(b.turn(), move.first); // position of moved cubed
                int dinf = (player=='f')? max(5-p.x(), 5-p.y()): max(p.x()-1, p.y()-1);
                int d1 = (player=='f')? (10-p.x()-p.y()): (p.x()+p.y()-2);
                dist.emplace_back(b2.winner()==Color::other, op_num, dinf, d1);
            }
            auto move = ml[min_element(dist.begin(), dist.end())-dist.begin()];
            b.do_move(move.first, move.second); // do a move
            cout << move.first << move.second << flush; // print the move
        }
    }
    return 0;
}

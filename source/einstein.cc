// by Yunghsien Chung (hiiragi4000)

// This file defines some variables and functions declared in einstein.hh.
// You may want to give a fast skim over the output formats of each class.

#include"einstein.hh"

using namespace std;

extern array<string, 7> const Die6_icon{{"", "⚀", "⚁", "⚂", "⚃", "⚄", "⚅"}};

ostream &operator <<(ostream &os, Color const &c){
    return os << "rbd"[Enum2int(c)];
}

ostream &operator <<(ostream &os, Cube const &c){
    if(c.color() == Color::other){
        return os << "--";
    }
    os << c.color();
    if(c.no() == 7){
        return os << '?';
    }
    return os << c.no();
}

ostream &operator <<(ostream &os, Square const &p){
    if(p) return os << '(' << p.x() << ", " << p.y() << ')';
    return os << "--";
}

array<int, 6> const Board::init_pos_i{{0, 1, 2, 5, 6, 10}};
array<array<Square, 6>, 2> const Board::init_pos{{
    {Square(1, 1), Square(1, 2), Square(1, 3), Square(2, 1), Square(2, 2), Square(3, 1)},
    {Square(5, 5), Square(5, 4), Square(5, 3), Square(4, 5), Square(4, 4), Square(3, 5)}
}};
array<array<int, 4>, 2> const Board::dx{{{0, 0, 1, 1}, {0, 0, -1, -1}}}, Board::dy{{{0, 1, 0, 1}, {0, -1, 0, -1}}};

ostream &operator <<(ostream &os, Board const &board){
    for(int i=1; i<=5; ++i){
        for(int j=1; j<=5; ++j){
            os << board(i, j) << " \n"[j==5];
        }
    }
    return os;
}

array<string, 8> const Cube_icon{{"  ", "１", "２", "３", "４", "５", "６", "？"}};
ostream &operator <<(ostream &os, Board_gui const &board){
    os << "\033[m";
    Square ph = board.highlight_pos(), pd;
    int player = Enum2int(board(ph).color());
    int dir = board.highlight_dir();
    if(dir){
        pd = {ph.x()+Board::dx[player][dir], ph.y()+Board::dy[player][dir]};
    }
    for(int i=1; i<=5; ++i){
        for(int j=1; j<=5; ++j){
            Square pij(i, j);
            if(pij == pd){
                os << (player? "\033[1;33;46m": "\033[1;32;45m") << "＊";
            }else{
                if(board(i, j).color() == Color::red){
                    os << (pij==ph? "\033[30;42m": "\033[1;41m");
                }else if(board(i, j).color() == Color::blue){
                    os << (pij==ph? "\033[30;43m": "\033[1;44m");
                }else os << "\033[47m";
                os << Cube_icon[board(i, j).no()];
            }
            os << "\033[m";
        }
        os << '\n';
    }
    return os;
}

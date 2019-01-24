// This file is adapted from https://stackoverflow.com/questions/7469139
// You don't need to know how it works

#include<termios.h>
#include<iostream>

using namespace std;

static struct termios st_old, st_new;

// initialize new terminal i/o settings
static void initTermios(int echo){
    tcgetattr(0, &st_old); // grab old terminal i/o settings
    st_new = st_old; // make new settings same as old settings
    st_new.c_lflag &= ~ICANON; // disable buffered i/o
    if(echo){
        st_new.c_lflag |= ECHO; // set echo mode
    }else{
        st_new.c_lflag &= ~ECHO; // set no echo mode
    }
    tcsetattr(0, TCSANOW, &st_new); // use these new terminal i/o settings now
}

// restore old terminal i/o settings
static void resetTermios(){
    tcsetattr(0, TCSANOW, &st_old);
}

// read 1 character - echo defines echo mode
static char getch_(int echo){
    initTermios(echo);
    char c;
    if(cin.flags() & cin.skipws){
        cin >> noskipws >> c >> skipws;
    }else cin >> c;
    resetTermios();
    return c;
}

// read 1 character without echo
char getch(){
    return getch_(0);
}

// read 1 character with echo
char getche(){
    return getch_(1);
}

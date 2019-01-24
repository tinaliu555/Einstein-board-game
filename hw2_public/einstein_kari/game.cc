// by Yunghsien Chung (hiiragi4000)

// This file serves as a game/judge server.
// Red player always goes first.

#include<bits/stdc++.h>
#include"agent.hh"
#include"einstein.hh"
#include"getch.hh"

using namespace std;
using namespace chrono;
using namespace this_thread;

// clear n lines
#define CLR_LINES(n) do{\
    for(int cnt=(n); cnt--;){\
        cout << "\033[1F\033[2K";\
    }\
}while(0)
#define WRONG_ARG \
"Usage: game [-n np agents...] [-r round] [-s seed] [-g] [-l logfile]\n\n"\
"np: number of human players (0-2), 2 by default\n"\
"agents...: the (2-np) AIs\n"\
"round: number of rounds, 8/∞ when np=0/np≠0 by default, and can only be specified if np=0\n"\
"seed: random seed for the random part, std::random_device{}() by default\n"\
"-g: enable the GUI; can only be specified if np=0\n"\
"logfile: the file to record the game\n"
#define TIMEOUT 10
#define TLE "Time Limit Exceeded"
#define FOUL "Foul"
#define FPS 1

int main(int argc, char **argv) try{
    // parse argv
    int arg_np = -1, arg_nround = 0;
    string arg_agent[2], arg_log;
    bool arg_seed_set = false, arg_gui = false;
    decltype(Die6{}.seed()) arg_seed;
    for(int i=1; i<argc; ++i){
        if(!strcmp(argv[i], "-n")){
            if(arg_np>=0 || ++i==argc){
                throw runtime_error(WRONG_ARG);
            }
            if(!strcmp(argv[i], "0")){
                if(i+2 >= argc){
                    throw runtime_error(WRONG_ARG);
                }
                arg_np = 0;
                for(int j=0; j<=1; ++j){
                    arg_agent[j] = argv[++i];
                    if(find(arg_agent[j].begin(), arg_agent[j].end(), '/') == arg_agent[j].end()){
                        arg_agent[j] = "./" + arg_agent[j];
                    }
                }
            }else if(!strcmp(argv[i], "1")){
                if(++i == argc){
                    throw runtime_error(WRONG_ARG);
                }
                arg_np = 1;
                arg_agent[0] = argv[i];
                if(find(arg_agent[0].begin(), arg_agent[0].end(), '/') == arg_agent[0].end()){
                    arg_agent[0] = "./" + arg_agent[0];
                }
            }else if(!strcmp(argv[i], "2")){
                if(arg_nround){
                    throw runtime_error(WRONG_ARG);
                }
                arg_np = 2;
            }else throw runtime_error(WRONG_ARG);
        }else if(!strcmp(argv[i], "-r")){
            if(arg_np==2 || arg_nround || ++i==argc){
                throw runtime_error(WRONG_ARG);
            }
            arg_nround = atoi(argv[i]);
            if(arg_nround <= 0){
                throw runtime_error(WRONG_ARG);
            }
        }else if(!strcmp(argv[i], "-s")){
            if(arg_seed_set || ++i==argc){
                throw runtime_error(WRONG_ARG);
            }
            arg_seed_set = true;
            istringstream(argv[i]) >> arg_seed;
        }else if(!strcmp(argv[i], "-g")){
            arg_gui = true;
        }else if(!strcmp(argv[i], "-l")){
            if(!arg_log.empty() || ++i==argc){
                throw runtime_error(WRONG_ARG);
            }
            arg_log = argv[i];
        }else throw runtime_error(WRONG_ARG);
    }
    if(arg_np == -1) arg_np = 2;
    if(arg_nround && arg_np){
        throw runtime_error(WRONG_ARG);
    }
    if(arg_nround==0 && arg_np==0){
        arg_nround = 8;
    }
    if(!arg_seed_set){
        arg_seed = Die6{}.seed();
    }
    if(arg_gui && arg_np){
        throw runtime_error(WRONG_ARG);
    }

    ofstream flog;
    if(!arg_log.empty()){
        flog.open(arg_log);
    }else flog.open("/dev/null");
    if(!flog){
        throw runtime_error("can't open the log file");
    }
    Coin coin(arg_seed); // determine which player goes first
    auto random_arrangement = [arg_seed]() -> string{
        static mt19937_64 generator(arg_seed);
        string arr = "123456";
        shuffle(arr.begin(), arr.end(), generator);
        return arr;
    };
    auto timer = [](bool reset = false) -> double{
        static int cnt = 0;
        static decltype(steady_clock::now()) tick, tock;
        if(!cnt || reset){
            tick = steady_clock::now();
            cnt = 1;
            return 0;
        }
        tock = steady_clock::now();
        cnt = 0;
        return duration_cast<duration<double>>(tock-tick).count();
    };

    if(arg_np == 0){ // 2 AIs
        flog << "Player 1 = " << arg_agent[0] << '\n';
        flog << "Player 2 = " << arg_agent[1] << '\n';
        flog << "Random Seed = " << arg_seed << '\n';
        flog << "==========\n" << flush;
        if(arg_gui){
            cout << "\033[1;36m" << Die6_icon[Die6(arg_seed)()] << "Einstein Würfelt Nicht! (Kari)" << "\033[m\n\n";
            cout << "\033[1mPlayer 1 = " << arg_agent[0] << "\033[m\n";
            cout << "\033[1mPlayer 2 = " << arg_agent[1] << "\033[m\n";
            cout << "\033[1;32mRandom Seed = " << arg_seed << "\033[m\n\n";
        }
        int first_player = coin()-1, score[2]{};
        Agent agent[2]{Agent(arg_agent[0]), Agent(arg_agent[1])};
        for(int round=1; round<=arg_nround; ++round, first_player^=1){
            // announce the winner to the two AIs, kill and restart the loser if necessary
            auto announce = [&agent, &score, &arg_agent, &flog, &arg_gui, &first_player, &round, &arg_nround](int winner, size_t winner_len, size_t loser_len, string const &status = "") -> void{
                agent[winner].write(string(winner_len, 'w'));
                ++score[winner];
                if(loser_len){
                    agent[winner^1].write(string(loser_len, 'l'));
                }else agent[winner^1] = Agent(arg_agent[winner^1]);
                if(!status.empty()){
                    flog << "Player " << ((winner+1)^3) << ": " << status << '\n';
                }
                flog << "Player " << (winner+1) << " Wins.\n";
                flog << "Player 1's Score: " << score[0] << '\n';
                flog << "Player 2's Score: " << score[1] << '\n';
                flog << "==========\n" << flush;
                if(arg_gui){
                    if(!status.empty()){
                        cout << "\033[1;32mPlayer " << ((winner+1)^3) << ": " << status << "\033[m\n";
                    }
                    cout << (winner==first_player? "\033[1;31m": "\033[1;34m");
                    cout << "Player " << (winner+1) << " Wins.\033[m\n";
                    sleep_for(milliseconds(1000));
                    if(round < arg_nround){
                        CLR_LINES(14+!status.empty());
                    }else{
                        cout << "\n\033[1mPlayer 1's Score: " << score[0] << "\033[m\n";
                        cout << "\033[1mPlayer 2's Score: " << score[1] << "\033[m\n\n";
                    }
                }
            };
            if(arg_gui){
                cout << "\033[1;35mRound " << round << "\033[m\n\n";
            }
            string init_arr = random_arrangement();
            Board_gui board(init_arr);
            flog << "Red (First) = Player " << (first_player+1) << '\n';
            flog << "Blue (Second) = Player " << ((first_player+1)^3) << "\n\n";
            flog << dynamic_cast<Board&>(board) << '\n' << flush;
            if(arg_gui){
                cout << "\033[1;31mRed (First) = Player " << (first_player+1) << "\033[m\n";
                cout << "\033[1;34mBlue (Second) = Player " << ((first_player+1)^3) << "\033[m\n\n";
                cout << "\033[1mInitial Arrangement:\033[m\n\n" << board << "\033[m\n";
                sleep_for(milliseconds(1000/FPS));
            }
            agent[first_player].write('f'+init_arr);
            agent[first_player^1].write('s'+init_arr);
            for(int player=first_player, no=0, dir=0, init=1; ; player^=1){
                if(arg_gui){
                    CLR_LINES(8);
                    cout << "\033[1mPlayer " << (player+1) << "'s Turn:\033[m\n\n";
                    cout << board << '\n';
                }
                if(!init){
                    string send{static_cast<char>(no+'0'), static_cast<char>(dir+'0')};
                    if(!agent[player].write(send)){
                        announce(player^1, 2, 0, FOUL);
                        break;
                    }
                }else init = 0;
                timer();
                string receive = agent[player].read(2, TIMEOUT);
                auto dt = timer();
                if(receive.empty()){
                    announce(player^1, 2, 0, dt>TIMEOUT? TLE: FOUL);
                    break;
                }
                no = receive[0]-'0';
                dir = receive[1]-'0';
                if(!board.check_move(no, dir)){
                    announce(player^1, 2, 0, FOUL);
                    break;
                }
                board.do_move(no, dir);
                flog << Cube(static_cast<Color>(player!=first_player), no) << ", " << dir << '\n' << flush;
                if(arg_gui){
                    CLR_LINES(6);
                    cout << board << '\n';
                    sleep_for(milliseconds(1000/FPS));
                }
                if(board.winner() != Color::other){ // player 'player' wins
                    announce(player, 2, 2);
                    break;
                }
            }
        }
        for(int i=0; i<=1; ++i){
            agent[i].write('e');
        }
        sleep_for(milliseconds(500));
        return 0;
    }

    cout << "\033[1;36m" << Die6_icon[Die6(arg_seed)()] << "Einstein Würfelt Nicht! (Kari)" << "\033[m\n\n";
    cout << "\033[1;32mRandom Seed = " << arg_seed << "\033[m\n\n";
    flog << "Random Seed = " << arg_seed << '\n';
    flog << "==========\n" << flush;
    static Color const color[3] = {Color::other, Color::red, Color::blue};
    // do a move of a human player
    auto do_human_move = [&flog](Board_gui &board, int player) -> pair<int, int>{
        cout << "\033[1;33mChange: Arrow Keys\033[m\n";
        cout << "\033[1;33mConfirm: Z\033[m\n\n\n";
        cout << board << "\n\n";
        auto ml = board.move_list();
        if(ml.at(0).first == 0){
            board.do_move(0, 0);
            return {0, 0};
        }
        board.highlight(ml.at(0).first);
        board.set_hldir(ml.at(0).second);
        CLR_LINES(7);
        cout << board << "\n\n";
        int human_no, human_dir;
        for(int stage=0, iml=0; !stage;){
            char c = getch();
            switch(c){
            case 'Z': case 'z':
                ++stage;
                board.do_move(ml[iml].first, ml[iml].second);
                tie(human_no, human_dir) = ml[iml];
                flog << Cube(color[player], human_no) << ", " << human_dir << '\n' << flush;
                break;
            case '\033':
                c = getch();
                if(c != '['){
                    cout << '\a'; continue;
                }
                c = getch();
                if(!strchr("ABCD", c)){
                    cout << '\a'; continue;
                }
                iml = (iml+((c=='B'||c=='C')? 1: ml.size()-1))%ml.size();
                board.highlight(ml[iml].first);
                board.set_hldir(ml[iml].second);
                break;
            default:
                cout << '\a'; continue;
            }
            CLR_LINES(7);
            cout << board << "\n\n";
        }
        return {human_no, human_dir};
    };
    auto play_again = []() -> bool{
        cout << "\033[1;35mPlay Again?  \033[37mYes(Z) No(X)\033[m\n\n";
        while(1){
            char c = getch();
            if(c=='Z' || c=='z'){
                CLR_LINES(2);
                break;
            }else if(c=='X' || c=='x'){
                cout << "\033[1;36mThanks for Playing!\033[m\n\n";
                return false;
            }
            cout << '\a';
        }
        return true;
    };
    if(arg_np == 1){ // Human v.s. AI
        Agent agent(arg_agent[0]);
        for(int human=coin();; human^=3){
            flog << "Red (First) = " << (human==1? "You": arg_agent[0]) << '\n';
            flog << "Blue (Second) = " << (human==1? arg_agent[0]: "You") << "\n\n" << flush;
            string init_arr = random_arrangement();
            Board_gui board(init_arr);
            agent.write("fs"[human==1] + init_arr);
            auto ai_exception = [&board, &flog, &arg_agent, &play_again, &agent](string const &status) -> void{
                CLR_LINES(13);
                cout << "\033[1;31mAI Exception: " << status << "\033[m\n\n\n\n\n\n" << board << "\n\n";
                cout << "\033[1mYou Win!\033[m\n\n";
                flog << "AI Exception: " << status << "\nYou Win.\n";
                flog << "==========\n" << flush;
                agent = Agent(arg_agent[0]);
                if(!play_again()){
                    agent.write('e');
                    sleep_for(milliseconds(500));
                    exit(0);
                }
                CLR_LINES(15);
            };
            for(int player=1, human_no=0, human_dir=0, init=1; board.winner()==Color::other; player^=3, init=0){
                if(player == human){
                    cout << "\033[1mYour Turn:\033[m\n\n";
                    tie(human_no, human_dir) = do_human_move(board, player);
                }else{
                    cout << "\033[1mAI's Turn:\033[m\n\n\n";
                    cout << "\033[1;33mNow Searching ...\033[m\n\n\n" << board << "\n\n";
                    sleep_for(milliseconds(1000/FPS));
                    if(!init){
                        string send{static_cast<char>(human_no+'0'), static_cast<char>(human_dir+'0')};
                        if(!agent.write(send)){
                            ai_exception(FOUL);
                            break;
                        }
                    }
                    timer();
                    string receive = agent.read(2, TIMEOUT);
                    double dt = timer();
                    if(receive.empty()){
                        ai_exception(dt>TIMEOUT? TLE: FOUL);
                        break;
                    }
                    if(!board.check_move(receive[0]-'0', receive[1]-'0')){
                        ai_exception(FOUL);
                        break;
                    }
                    Cube cube(static_cast<Color>(player==2), receive[0]-'0');
                    int no = receive[0] - '0';
                    int dir = receive[1] - '0';
                    board.do_move(no, dir);
                    flog << Cube(static_cast<Color>(player==2), no) << ", " << dir << '\n' << flush;
                    CLR_LINES(7);
                    cout << board << "\n\n";
                }
                if(board.winner() != Color::other){
                    if(player == human){
                        agent.write("ll");
                    }else{
                        agent.write("ww");
                    }
                    cout << "\033[1m" << (player==human? "You Win!": "You Lose!") << "\033[m\n\n";
                    flog << (player==human? "You Win.\n": "AI Wins.\n");
                    flog << "==========\n" << flush;
                    if(!play_again()){
                        agent.write('e');
                        sleep_for(milliseconds(500));
                        exit(0);
                    }
                    CLR_LINES(2);
                }
                CLR_LINES(13);
            }
        }
    }else{ // 2 human players
        while(1){
            string init_arr = random_arrangement();
            Board_gui board(init_arr);
            for(int player=1; board.winner()==Color::other; player^=3){
                cout << (player==1? "\033[1;31m": "\033[1;34m");
                cout << "Player " << player << "'s Turn:\033[m\n\n";
                do_human_move(board, player);
                if(board.winner() != Color::other){
                    cout << (board.winner()==Color::red? "\033[1;31m": "\033[1;34m");
                    cout << "You Win!\033[1m\n\n";
                    flog << (board.winner()==Color::red? "Red": "Blue") << " Wins.\n";
                    flog << "==========\n" << flush;
                    if(!play_again()){
                        return 0;
                    }
                    CLR_LINES(2);
                }
                CLR_LINES(13);
            }
        }
    }

}catch(exception &e){ // a bug or something alike
    cerr << e.what() << '\n';
    return 1;
}

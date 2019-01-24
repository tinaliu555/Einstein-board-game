// by Yunghsien Chung (hiiragi4000)

// This file defines the class of the communication between judge and AI.
// In most cases you don't need to know how it works

#ifndef AGENT_HH
#define AGENT_HH

#include<sys/wait.h>
#include<unistd.h>
#include<cstdio>
#include<algorithm>
#include<chrono>
#include<stdexcept>
#include<string>
#include<utility>
#include<vector>

// Important: Don't use this class in a multithreaded program!
class Agent final{
private:
    static void catch_sigpipe(int sig){}
    static bool init;
    mutable int pid = -1;
    int rfd, wfd;
public:
    // a process should belong to exactly 1 Agent
    Agent(Agent const&) = delete;
    Agent &operator =(Agent const&) = delete;
    Agent(Agent &&a){
        *this = std::move(a);
    }
    Agent &operator =(Agent &&a){
        if(pid != a.pid){
            wait();
            pid = a.pid, rfd = a.rfd, wfd = a.wfd;
            a.pid = -1;
        }
        return *this;
    }
    ~Agent(){
        wait();
    }
    Agent() noexcept = default;
    explicit Agent(std::string const &exe_file){
        // if it's the first use of Agent, set the signal handler to ignore SIGPIPE
        if(init){
            typename ::sigaction catcher;
            catcher.sa_handler = catch_sigpipe;
            catcher.sa_flags = 0;
            if(::sigemptyset(&catcher.sa_mask) == -1){
                throw std::runtime_error("Agent::Agent(): ::sigemptyset() failed");
            }
            if(::sigaction(SIGPIPE, &catcher, nullptr) == -1){
                throw std::runtime_error("Agent::Agent(): ::sigaction() failed");
            }
            init = false;
        }
        int cpid, rfdp[2], wfdp[2];
        if(::pipe(rfdp)==-1 || ::pipe(wfdp)==-1){
            throw std::runtime_error("Agent::Agent(): ::pipe() failed");
        }
        cpid = ::fork();
        if(cpid == -1){
            throw std::runtime_error("Agent::Agent(): ::fork() failed");
        }
        if(!cpid){
            if(::close(rfdp[0])==-1 || ::close(wfdp[1])==-1){
                throw std::runtime_error("Agent::Agent(): ::close() failed");
            }
            if(::dup2(rfdp[1], 1)==-1 || ::dup2(wfdp[0], 0)==-1){
                throw std::runtime_error("Agent::Agent(): ::dup2() failed");
            }
            if(::close(rfdp[1])==-1 || ::close(wfdp[0])==-1){
                throw std::runtime_error("Agent::Agent(): ::close() failed");
            }
            if(!std::freopen("/dev/null", "w", stderr)){
                throw std::runtime_error("Agent::Agent(): ::freopen() failed");
            }
            if(::execlp(exe_file.c_str(), exe_file.c_str(), nullptr)==-1){
                throw std::runtime_error("Agent::Agent(): ::execlp() failed");
            }
        }
        if(::close(rfdp[1])==-1 || ::close(wfdp[0])==-1){
            throw std::runtime_error("Agent::Agent(): ::close() failed");
        }
        pid = cpid;
        rfd = rfdp[0];
        wfd = wfdp[1];
    }
    bool write(char c) const{
        if(pid == -1){
            return false;
        }
        int dn = ::write(wfd, &c, 1);
        return dn == 1;
    }
    bool write(std::string const &s) const{
        if(pid == -1){
            return false;
        }
        std::size_t offset = 0;
        while(offset < s.size()){
            int dn = ::write(wfd, s.c_str()+offset, s.size()-offset);
            if(dn <= 0){
                return false;
            }
            offset += dn;
        }
        return true;
    }
    // read size characters within timeout_sec seconds
    std::string read(std::size_t size, int timeout_sec) const{
        if(pid == -1){
            return "";
        }
        if(timeout_sec < 0){
            timeout_sec = 0;
        }
        int timeout_us = 1000000*timeout_sec + 200000;
        std::string result;
        std::vector<char> buf(size+1);
        while(size){
            ::fd_set fds;
            FD_ZERO(&fds);
            FD_SET(rfd, &fds);
            ::timeval timeout{timeout_us/1000000, timeout_us%1000000};
            auto t1 = std::chrono::steady_clock::now();
            if(::select(rfd+1, &fds, nullptr, nullptr, &timeout) == -1){
                throw std::runtime_error("Agent::read(): ::select() failed");
            }
            auto t2 = std::chrono::steady_clock::now();
            int dt = 1e6*std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count();
            timeout_us -= dt;
            if(timeout_us < 100000){
                return "";
            }
            int dn = ::read(rfd, buf.data(), size);
            switch(dn){
            case -1:
                throw std::runtime_error("Agent::read(): ::read() failed");
            case 0:
                return "";
            default:
                buf[dn] = 0;
                result += buf.data();
                std::fill(buf.data(), buf.data()+dn, 0);
                size -= dn;
            }
        }
        return result;
    }
    // wait the child process within timeout_sec seconds
    void wait(int timeout_sec = 0) const{
        if(pid == -1) return;
        if(sleep > 0){
            ::sleep(timeout_sec);
        }
        if(::close(rfd)==-1 || ::close(wfd)==-1){
            throw std::runtime_error("Agent::wait(): ::close() failed");
        }
        ::kill(pid, SIGKILL);
        if(::waitpid(pid, nullptr, 0) == -1){
            throw std::runtime_error("Agent::wait(): ::waitpid() failed");
        }
        pid = -1;
    }
};

#endif

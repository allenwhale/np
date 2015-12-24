#ifndef __SOCKS4__H__
#define __SOCKS4__H__
#define SOCKS4_PORT 2345
#define BUF_SIZE 1024*8
#include <bits/stdc++.h>
#include "socks4_req.h"
using namespace std;

class Socks4Server{
    public:
        int sockfd;
        int port;
        Socks4Server(int);
        int Connect();
        void Run();
        void Handler(int);
        void ConnectHandler(int, const Socks4Request &);
        void BindHandler(int, const Socks4Request &);
};


#endif

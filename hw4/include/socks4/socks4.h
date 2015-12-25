#ifndef __SOCKS4__H__
#define __SOCKS4__H__
#define SOCKS4_PORT 2345
#define BUF_SIZE 1024*8

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "socks4_req.h"
#include "socks4_firewall.h"
using namespace std;

class Socks4Server{
    public:
        int sockfd;
        int port;
        string socks_conf;
        Socks4Firewall firewall;
        Socks4Server(int,const string &);
        int Connect();
        int ReadConf();
        void Run();
        void Handler(int, const sockaddr_in &);
        void ConnectHandler(int, const Socks4Request &);
        void BindHandler(int, const Socks4Request &);
};


#endif

#ifndef __SERVER__H__
#define __SERVER__H__
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
using namespace std;
#define BUF_SIZE 1024

class Server{
    public:
        int                 sockfd;
        sockaddr_in			dest;
        int					port;
        int					connection;
        set<string>         cgi;
        Server(int, int);
        void SetCGI(const set<string>&);
        bool Connect();
        void Run();
        void RequestHandler(int);
};
#endif

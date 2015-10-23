#ifndef __SERVER__H__
#define __SERVER__H__
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>

class Server{
public:
    int					sockfd, fifofd, epfd;
    sockaddr_in			dest;
    int					port;
    int					connection;
	std::string			fifo;
	//std::set<int>		client_pid;
    Server(int, int);
    int Connect();
    void Run();
    int ServerProcess();
    int StdinProcess();
    int ClientProcess(int);
	void CloseClient(int);
	int Read(int, std::string&);
    void Close();
};
#endif

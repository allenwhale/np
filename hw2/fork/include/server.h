#ifndef __SERVER__H__
#define __SERVER__H__
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include "client.h"
#include "msgq.h"

#define MAX_CLIENT_SIZE 128
#define MAX_GLOBAL_PIPE 128
class Server{
public:
    int					sockfd, fifofd, epfd;
    sockaddr_in			dest;
    int					port;
    int					connection;
    int                 shm_id;
    Client              clients[MAX_CLIENT_SIZE];
    char                fifo[256];
    Message             msg[MAX_CLIENT_SIZE];
    bool                global_pipe[MAX_GLOBAL_PIPE];
	//std::set<int>		client_pid;
    Server(int, int);
    int Connect();
    void Run();
    int ServerProcess();
    int StdinProcess();
    int ClientProcess(int);
	void CloseClient(int);
    int InsertNewClient(int, sockaddr_in);
    int GetClient(int);
    bool CheckNickname(const char*);
    int Tell(int, int, const std::string&);
    int Yell(int, const std::string&);
	int Read(int, std::string&);
    int Send(int, const char*);
    int SendAll(const char*);
    void Close();
};
#endif

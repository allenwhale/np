#ifndef __SERVER__H__
#define __SERVER__H__
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/stdc++.h>
//#include "client.h"
#include "command.h"
class Client;
class Server{
public:
    int					sockfd, fifofd, epfd;
    sockaddr_in			dest;
    int					port;
    int					connection;
	std::string			fifo;
    std::vector<Client*> clients;
    std::map<int, Client*> fd2client;
    std::map<int, Client*> id2client;
    std::set<std::string>   nickname;
    PipeLine            global_pipe;


	//std::set<int>		client_pid;
    Server(int, int);
    int Connect();
    void Run();
    int ServerProcess();
    int StdinProcess();
    int ClientProcess(int);
	void CloseClient(int);
    Client* InsertNewClient(int, std::string, int);
	int Read(int, std::string&);
    int Send(int, const char*);
    int SendAll(const char *);
    void Close();
};
#endif

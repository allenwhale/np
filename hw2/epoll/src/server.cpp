#include <sys/types.h>
#include <sys/socket.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include "function.h"
#include "server.h"
#include "command.h"
#include "shell.h"
#include "client.h"
#define PORT 1234
std::set<int> client_pid;
Server::Server( int _port, int _connection): port(_port), connection(_connection), fifo("/tmp/server_fifo"){
	unlink(fifo.c_str());
}

int Server::Connect(){
	if((epfd=epoll_create(MAX_EPOLL_SIZE)) < 0){
		perror("epoll create failed");
		return -1;
	}
	if(mkfifo(fifo.c_str(), 0660) < 0){
		perror("fifo create failed");
		return -1;
	}
	if((fifofd=open(fifo.c_str(), O_RDONLY|O_NONBLOCK)) < 0){
		perror("fifo open failed");
		return -1;
	}
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) <= 0){
        perror("socket creation failed");
        return -1;
    }
    memset(&dest, 0 , sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int)) < 0){
        perror("setsocketopt failed");
        return -1;
    }
    if(bind(sockfd, (sockaddr*)&dest, sizeof(dest)) < 0){
        perror("bind failed");
        return -1;
    }
    if(listen(sockfd, connection) < 0){
        perror("listen failed");
        return -1;
    }
	epoll_add_event(epfd, sockfd, EPOLLIN);
	epoll_add_event(epfd, fifofd, EPOLLIN);
	epoll_add_event(epfd, STDIN_FILENO, EPOLLIN);
    return 0;
}
int Server::ServerProcess(){
    int fd;
    sockaddr_in cliaddr;
    int addrlen = sizeof(cliaddr); 
    if((fd = accept(sockfd, (sockaddr*)&cliaddr, (socklen_t*)&addrlen)) < 0){
		return -1;
	}
    std::string ip = inet_ntoa(cliaddr.sin_addr);
    ip = "CGILAB";
    int port = cliaddr.sin_port;
    port = 511;
    int old_stdin = dup(STDIN_FILENO);
    int old_stdout = dup(STDOUT_FILENO);
    int old_stderr = dup(STDERR_FILENO);
	epoll_add_event(epfd, fd, EPOLLIN);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    Client *client = InsertNewClient(fd, ip, port);
    char msg[100];
    sprintf(msg, "*** User \'%s\' entered from %s/%d. ***\n", client->nickname.c_str(), client->ip.c_str(), client->port);
    client->shell = new Shell();
    SendAll(msg);
    client->shell->Prompt();
    dup2(old_stdin, STDIN_FILENO);
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);
    close(old_stdin);
    close(old_stdout);
    close(old_stderr);
}
Client* Server::InsertNewClient(int fd, std::string ip, int port){
    int id = 0;
    for(auto client: clients){
        if(client->id != id+1) break;
        id = client->id;
    }
    clients.push_back(new Client(id+1, fd, ip, port));
    fd2client[fd] = clients.back();
    id2client[clients.back()->id] = clients.back();
    std::sort(clients.begin(), clients.end(), [](Client *a, Client *b){return a->id < b->id;});
    return fd2client[fd];
}
int Server::ClientProcess(int fd){
    int old_stdin = dup(STDIN_FILENO);
    int old_stdout = dup(STDOUT_FILENO);
    int old_stderr = dup(STDERR_FILENO);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    int res = fd2client[fd]->shell->Run(fd2client[fd], this);
    dup2(old_stdin, STDIN_FILENO);
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);
    close(old_stdin);
    close(old_stdout);
    close(old_stderr);
    if(res == SHELL_EXIT)CloseClient(fd);
	return 0;
}

void Server::CloseClient(int fd){
    for(int i=0;i<(int)clients.size();i++){
        if(clients[i]->fd == fd){
            char msg[256];
            sprintf(msg, "*** User '%s' left. ***\n", clients[i]->nickname.c_str());
            SendAll(msg);
            fd2client.erase(fd);
            id2client.erase(clients[i]->id);
            nickname.erase(clients[i]->nickname);
            clients.erase(clients.begin()+i);
            break;
        }
    }
	epoll_del_event(epfd, fd, EPOLLIN);
	close(fd);
}
int Server::StdinProcess(){
	std::string buf;
	if(Read(STDIN_FILENO, buf) <= 0) return -1;
	if(buf == "exit") Close();
	return 0;
}
void Server::Run(){
	epoll_event events[MAX_EPOLL_SIZE];
	int res;
	while(true){
		res = epoll_wait(epfd, events, MAX_EPOLL_SIZE, -1);
		for(int i=0;i<res;i++){
			int fd = events[i].data.fd;
			if(fd == sockfd) ServerProcess();
			else if(fd == STDIN_FILENO) StdinProcess();
			else{
                if(fd2client.find(fd) != fd2client.end()){
                    ClientProcess(fd);
                }else{
                }      
            }
		}
	}
}
int Server::Read(int fd, std::string& str){
	char buf[BUFFER_SIZE];
	int res;
	if((res=read(fd, buf, BUFFER_SIZE)) <= 0)
		return -1;
	if(buf[res-1] == '\n') buf[res-1] = 0;
	str = buf;
	return res;
}
int Server::Send(int fd, const char *msg){
    return write(fd, msg, strlen(msg));
}
int Server::SendAll(const char *msg){
    for(auto client: clients)
        Send(client->fd, msg);
    return 0;
}
void Server::Close(){
	for(auto pid: client_pid)
		printf("close %d\n", pid), kill(pid, SIGINT), waitpid(pid, NULL, 0);
	close(sockfd);
	unlink(fifo.c_str());
	exit(0);
}
void zombie_handler(int sig){
	int status, pid = waitpid(-1, &status, WNOHANG);
	if(pid!=-1&&WIFEXITED(status))
		client_pid.erase(pid);
}
int main(){
	Server server(PORT, MAX_EPOLL_SIZE);
	signal(SIGCHLD, zombie_handler);
	if(server.Connect() != 0)return 0;
	server.Run();
	return 0;
}

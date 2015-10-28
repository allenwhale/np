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
	int pid = fork();
	if(pid == 0){
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		Shell sh;
		sh.Run();
		exit(0);
	}else if(pid > 0){
		client_pid.insert(pid);
		close(fd);
		return 0;
	}else{
		return -1;
	}
}
int Server::ClientProcess(int fd){
	return 0;
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
			else if(fd == fifofd) ClientProcess(fd);
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
void Server::CloseClient(int fd){
	printf("%d leaves\n", fd);
	epoll_del_event(epfd, fd, EPOLLIN);
	close(fd);
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

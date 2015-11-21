#include <sys/types.h>
#include <sys/socket.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/shm.h>
#include "function.h"
#include "server.h"
#include "command.h"
#include "sem.h"
#include "shell.h"
#include "msgq.h"
#define PORT 1234
std::set<int> client_pid;
std::set<int> client_id;
Server::Server( int _port, int _connection): port(_port), connection(_connection){
    memset(global_pipe, false, sizeof(global_pipe));
}

int Server::Connect(){
	if((epfd=epoll_create(MAX_EPOLL_SIZE)) < 0){
		perror("epoll create failed");
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
    int id = InsertNewClient(fd, cliaddr);
    char _msg[256] = {0};
    sprintf(_msg, "*** User \'%s\' entered from %s/%d. ***\n", clients[id].nickname, clients[id].ip, clients[id].port);
	int pid = fork();
	if(pid == 0){
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		Shell sh;
        printf("%s", _msg);
        sh.Prompt();
		sh.Run(id, shm_id);
		exit(0);
	}else if(pid > 0){
        int sem = shm_sem_open(msg[id].key);
        shm_sem_wait(sem);
        msg[id].Init();
        shm_sem_post(sem);
        shm_sem_close(sem);
        for(int i=1;i<MAX_CLIENT_SIZE;i++)
            if(clients[i].used && i != id)
                msg[i].Append(_msg);
        client_pid.insert(pid);
        close(fd);
		return 0;
	}else{
		return -1;
	}
}
int Server::InsertNewClient(int fd, sockaddr_in cliaddr){
    int id = -1;
    for(int i=1;i<MAX_CLIENT_SIZE;i++){
        if(clients[i].used == false){
            id = i;
            break;
        }
    }
    clients[id].set_nickname("(no name)");
    clients[id].fd = fd;
    clients[id].used = true;
    char *ip = inet_ntoa(cliaddr.sin_addr);
    memset(clients[id].ip, 0, sizeof(clients[id].ip));
    memcpy(clients[id].ip, ip, strlen(ip));
    clients[id].port = cliaddr.sin_port;
    return id;
}
bool Server::CheckNickname(const char *nickname){
    int len = strlen(nickname);
    for(int i=1;i<MAX_CLIENT_SIZE;i++){
        if(clients[i].used){
            if(strlen(clients[i].nickname) == len && memcmp(nickname, clients[i].nickname, len) == 0)
                return false;
        }
    }
    return true;
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
			else ClientProcess(fd);
		}
	}
}
int Server::Read(int fd, std::string& str){
	char buf[BUFFER_SIZE] = {0};
    flock(fd, LOCK_EX);
	int res = read(fd, buf, BUFFER_SIZE);
    printf("ret of fd %d %d\n", fd, res);    
    flock(fd, LOCK_UN);
	if(res <= 0) return -1;
	if(buf[res-1] == '\n') buf[res-1] = 0;
	str = buf;
	return res;
}
void Server::CloseClient(int id){
    int fd = clients[id].fd;
    char msg[256] = {0};
    printf("id %d fd %d\n", id, clients[id].fd);
    sprintf(msg, "*** User '%s' left. ***\n", clients[id].nickname);
    SendAll(msg);
    clients[id].used = false;
    client_id.erase(id);
	epoll_del_event(epfd, fd, EPOLLIN);
	int res = close(fd);
    printf("close %d %d\n", id, res);
}
void Server::Close(){
	for(auto pid: client_pid)
		printf("close %d\n", pid), kill(pid, SIGINT), waitpid(pid, NULL, 0);
	close(sockfd);
    shmdt(this);
	exit(0);
}
int Server::Send(int fd, const char *msg){
    return write(fd, msg, strlen(msg));
}
int Server::SendAll(const char *msg){
    for(int i=1;i<MAX_CLIENT_SIZE;i++){
        if(clients[i].used){
            Send(clients[i].fd, msg);
        }
    }
    return 0;
}
void zombie_handler(int sig){
	int status, pid = waitpid(-1, &status, WNOHANG);
	if(pid!=-1&&WIFEXITED(status))
		client_pid.erase(pid);
}
int main(){
    srand(time(0));
    key_t key = ftok("/tmp/np", 0);
    int server_shm_id = shmget(key, sizeof(Server), 0666|IPC_CREAT);    
    if(server_shm_id == -1){
        perror("shared memory error");
        return 1;
    }
    Server *shm = (Server*)shmat(server_shm_id, (void*)0, 0);
    Server *server = new(shm) Server(PORT, MAX_EPOLL_SIZE);
    server->shm_id = server_shm_id;
    for(int i=0;i<MAX_CLIENT_SIZE;i++){
        server->msg[i].key = ftok("/tmp/np", i + MAX_CLIENT_SIZE);
        shm_sem_create(server->msg[i].key, 1);
    }
	signal(SIGCHLD, zombie_handler);
	if(server->Connect() != 0)return 0;
	server->Run();
	return 0;
}

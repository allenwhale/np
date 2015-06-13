#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"
#include <time.h>
#include <algorithm>
#include <vector>

Server::Server( int _port,
                int _timeout,
                int _connection): 
                port(_port), 
                timeout(_timeout), 
                connection(_connection){
    tv.tv_sec = 0;
    tv.tv_usec = timeout;
}

int Server::Connect(){
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) <= 0){
        fprintf(stderr, "socket creation failed\r\n");
        return -1;
    }
    memset(&dest, 0 , sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    if(setsockopt(  sockfd, 
                    SOL_SOCKET, SO_REUSEADDR,
                    (const char*)&opt, sizeof(int)) < 0){
        fprintf(stderr, "setsocketopt failed\r\n");
        return -1;
    }
    if(bind(sockfd, (sockaddr*)&dest, sizeof(dest)) < 0){
        fprintf(stderr, "bind failed\r\n");
        return -1;
    }
    if(listen(sockfd, connection) < 0){
        fprintf(stderr, "listen failed\r\n");
        return -1;
    }
    return 0;
}

void Server::SetCliFd(int fd){
    clifd.push_back(fd);
    clinick[fd] = std::to_string(fd);
}

void Server::SetFdNick(int fd, const char *nick){
    clinick[fd] = std::string(nick);
}

void Server::SetFd(){
    FD_ZERO(&infdset);
    FD_SET(sockfd, &infdset);
    FD_SET(0, &infdset);
    for(auto fd: clifd){
        FD_SET(fd, &infdset);
    }
}

void Server::Run(){
    int ret, maxfd;
    while(1){
        SetFd();
        maxfd = 0;
        for(auto fd: clifd){
            maxfd = std::max(maxfd, fd);
        }
        maxfd = std::max(std::max(maxfd, sockfd), 0) + 1;
        ret = select(maxfd, &infdset, NULL, NULL, &tv);
        if(ret == 0){
            usleep(1000);
        }else if(ret < 0){
            usleep(1000);
        }else{
            ServerProcess();
            ret = StdinProcess();
            if(ret == 1){
                return;
            }
            ClientProcess();
        }
    }
}

int Server::ServerProcess(){
    if(!FD_ISSET(sockfd, &infdset)) return 0;
    int clifd;
    sockaddr_in cliaddr;
    int addrlen = sizeof(cliaddr); 
    clifd = accept(sockfd, (sockaddr*)&cliaddr, (socklen_t*)&addrlen);
    if(clifd <= 0){
        fprintf(stderr, "error client\r\n");
        return -1;
    }
    printf("%d.%d.%d.%d\r\n",
            int(cliaddr.sin_addr.s_addr&0xFF),
            int((cliaddr.sin_addr.s_addr&0xFF00)>>8),
            int((cliaddr.sin_addr.s_addr&0xFF0000)>>16),
            int((cliaddr.sin_addr.s_addr&0xFF000000)>>24));
    char msg[BUFFER_SIZE+1];
    sprintf(msg, "Hello\r\n");
    send(clifd, msg, strlen(msg), 0);
    SetCliFd(clifd);
    return 0;
}

int Server::StdinProcess(){
    if(!FD_ISSET(0, &infdset)) return 0;
    char cmd[BUFFER_SIZE+1];
    int ret = read(0, cmd, BUFFER_SIZE);
    if(ret <= 0){
        fprintf(stderr, "stdin failed\r\n");
        return -1;
    }
    cmd[ret-1] = 0;
    printf("cmd: %s\r\n", cmd);
    if(strcmp(cmd, "exit") == 0){
        return 1;
    }
    return 0;
}

int Server::ClientProcess(){
    std::vector<int> alive;
    char buffer[BUFFER_SIZE+1];
    char sendbuffer[BUFFER_SIZE+1];
    char cmd[BUFFER_SIZE];
    int ret;
    char *ptr;
    for(auto fd: clifd){
        if(FD_ISSET(fd, &infdset)){
            ret = read(fd, buffer, BUFFER_SIZE);
            if(ret <= 0){
                char msg[BUFFER_SIZE+1];  
                sprintf(msg, "[%s] leaves\r\n", clinick[fd].c_str());
                printf(msg);
                SendAll(msg, strlen(msg), std::vector<int>{fd});
                continue;
            }
            buffer[ret] = 0;
            printf("%s", buffer);
            if(buffer[0] == '/'){
                ptr=strtok(buffer+1, " ");
                sscanf(ptr, "%s", cmd);
                ptr = strtok(NULL, " ");
                if(strcmp(cmd, "nick") == 0){
                    sscanf(ptr, "%s", cmd);
                    SetFdNick(fd, cmd);
                }
                goto END;
            }
            sprintf(sendbuffer ,"[%s]: %s", clinick[fd].c_str(), buffer);
            SendAll(sendbuffer, strlen(sendbuffer), std::vector<int>());
END:;
            alive.push_back(fd);
        }else{
            alive.push_back(fd);
        }
    }
    clifd = alive;
    return 0;
}

int Server::SendAll(const char *msg, int len, const std::vector<int> &except){
    for(auto fd: clifd){
        if(std::find(except.begin(), except.end(), fd) == except.end())
            int ret = write(fd, msg, len);
    }
    return 0;
}

void Server::Close(){
    close(sockfd);
}

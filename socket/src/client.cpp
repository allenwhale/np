#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "client.h"
#define BUFFER_SIZE 2048

Client::Client(const char *_ip, int _port, int _timeout):port(_port), timeout(_timeout){
    memcpy(ip, _ip, sizeof(char)*strlen(_ip));
    tv.tv_sec = 0;
    tv.tv_usec = _timeout;
}

int Client::Connect(){
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        fprintf(stderr, "socket create failed\r\n");
    }
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    const char *ptr = ip;
    dest.sin_addr.s_addr = inet_addr(ip);
    //inet_aton(ip, &dest.sin_addr);
    if(connect(sockfd, (sockaddr*)&dest, sizeof(dest)) < 0){
        fprintf(stderr, "connect error\r\n");
        return -1;
    }
    return 0;
}

void Client::SetFd(){
    FD_ZERO(&infdset);
    FD_SET(0, &infdset);
    FD_SET(sockfd, &infdset);

}

void Client::Run(){
    int ret;
    while(1){
        SetFd();
        int maxfd = sockfd + 1;
        ret = select(maxfd, &infdset, NULL, NULL, &tv);
        if(ret == 0){
            usleep(1000);
        }else if(ret < 0){
            usleep(1000);
        }else{
            ret = StdinProcess();
            if(ret == 1){
                return;
            }
            ret = ServerProcess();
        }
    }
}

int Client::StdinProcess(){
    if(!FD_ISSET(0, &infdset))return 0;
    char buffer[BUFFER_SIZE+1];
    int ret;
    ret = read(0, buffer, BUFFER_SIZE);
    if(ret <= 0){
        fprintf(stderr, "stdin failed\r\n");
        return -1;
    }
    buffer[ret] = 0;
    ret = write(sockfd, buffer, strlen(buffer));
    if(ret <= 0){
        fprintf(stderr, "send msg error\r\n");
        return -1;
    }
    return 0;
}

int Client::ServerProcess(){
    if(!FD_ISSET(sockfd, &infdset))return 0;
    char buffer[BUFFER_SIZE+1];
    int ret;
    ret = read(sockfd, buffer, BUFFER_SIZE);
    if(ret <= 0){
        fprintf(stderr, "server error\r\n");
        return -1;
    }
    buffer[ret] = 0;
    printf("%s", buffer);
    return 0;
}

void Client::Close(){
    close(sockfd);
}

#include "server.h"
#include "http.h"
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
using namespace std;

Server::Server(int _port, int _conn): port(_port), connection(_conn) {}
void Server::SetCGI(const set<string> &_cgi){
    cgi = _cgi;
}

bool Server::Connect(){
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
    return 0;
}

void Server::Run(){
    while(1){
        int cli_fd;
        sockaddr_in cli_addr;
        int addr_len = sizeof(cli_addr);
        if((cli_fd = accept(sockfd, (sockaddr*)&cli_addr, (socklen_t*)&addr_len)) < 0){
            perror("accept error");
            exit(1);
        }
        int pid = fork();
        if(pid < 0){
            perror("fork error");
            exit(1);
        }else if(pid == 0){
            close(sockfd);
            dup2(cli_fd, STDIN_FILENO);
            dup2(cli_fd, STDOUT_FILENO);
            RequestHandler(cli_fd);
            fprintf(stderr, "done\n");
            exit(0);
        }else{
            close(cli_fd);
        }
    }
}

void Server::RequestHandler(int fd){
    char buf[BUF_SIZE] ={0};
    int ret;
    if((ret = read(fd, buf, BUF_SIZE)) <= 0){
        perror("");
        exit(1);
    }
    while(isspace(buf[ret - 1]))
        buf[--ret];
    if(strncmp(buf, "GET", 3) && strncmp(buf, "get", 3))
        exit(1);
    char *ptr = strtok(buf, " ");
    ptr = strtok(NULL, " ");
    string path = "./" + string(ptr + 1);
    for(int i=0;i<path.size()-1;i++)
        if(path[i] == '.' && path[i+1] == '.')
            exit(1);
    string filename, query;
    if(path.find('?') == -1){
        filename= path.substr(path.find_last_of('/') + 1);
        query = "";
    }else{
        query = path.substr(path.find('?') + 1);
        path = path.substr(0, path.find('?'));
        filename= path.substr(path.find_last_of('/') + 1);
    }
    string ext;
    if(filename.find('.') != -1)
        ext = filename.substr(filename.find_last_of('.'));
    else ext = "";
    fprintf(stderr, "path %s\n", path.c_str());
    fprintf(stderr, "filename %s\n", filename.c_str());
    fprintf(stderr, "query %s\n", query.c_str());
    fprintf(stderr, "ext %s\n", ext.c_str());
    if(cgi.find(ext) == cgi.end()){
        fprintf(stderr, "html\n");
        printf("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
        FILE *f = fopen(path.c_str(), "r");
        memset(buf, 0, sizeof(buf));
        while(fgets(buf, BUF_SIZE, f)){
            printf("%s", buf);
            fprintf(stderr, "%s\n", buf);
        }
        fclose(f);
    }else{
        setenv("QUERY_STRING", query.c_str(), 1);
        execl(path.c_str(), filename.c_str(), NULL);
    }
}

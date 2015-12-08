#include "client.h"
#include "hw3.h"
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
using namespace std;

void set_nonblock(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

void run(const args_map &args){
    fd_set rfds, wfds, n_rfds, n_wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&n_rfds);
    FD_ZERO(&n_wfds);
    int max_cli_fd = 0;
    int clients_fd[10];
    memset(clients_fd, -1, sizeof(clients_fd));
    int clients_status[10];
    string batch[10];
    FILE *f_batch[10];
    printf("start\n");
    int conn = 0;
    for(int i=1;i<=5;i++){
        if(args.find("h" + to_string(i)) != args.end() && args.at("h" + to_string(i)) != ""){
            batch[i] = args.at("f" + to_string(i));
            f_batch[i] = fopen(args.at("f" + to_string(i)).c_str(), "r");
            printf("fd %p\n", f_batch[i]);
            if(f_batch[i] == NULL){
                continue;
            }
            hostent *host = gethostbyname(args.at("h" + to_string(i)).c_str());
            sockaddr_in addr;
            int port = stoi(args.at("p" + to_string(i)));
            int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
            set_nonblock(cli_fd);
            bzero(&addr, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr = *(in_addr*)host->h_addr;
            addr.sin_port = htons(port);
            connect(cli_fd, (sockaddr*)&addr, sizeof(addr));
            //sleep(1);
            printf("cli %d\n", cli_fd);
            max_cli_fd = max(max_cli_fd, cli_fd);
            clients_fd[i] = cli_fd;
            clients_status[i] = C_READING;
            FD_SET(cli_fd, &n_rfds);
            FD_SET(cli_fd, &n_wfds);
            conn ++;
        }
    }
    while(conn){
        memcpy(&rfds, &n_rfds, sizeof(n_rfds));
        memcpy(&wfds, &n_wfds, sizeof(n_wfds));
        if(select(max_cli_fd + 1, &rfds, &wfds, NULL, NULL) < 0)
            exit(1);
        for(int i=0;i<=5;i++){
            if(clients_fd[i] != -1){
                if(clients_status[i] == C_READING && FD_ISSET(clients_fd[i], &rfds)){
                    char t_buf[BUF_SIZE] = {0};
                    if(read(clients_fd[i], t_buf, BUF_SIZE) <= 0){
                        printf("read cli close %d %d\n", i, clients_fd[i]);
                        close(clients_fd[i]);
                        FD_CLR(clients_fd[i], &n_wfds);
                        FD_CLR(clients_fd[i], &n_rfds);
                        clients_fd[i] = -1;
                        conn --;
                        continue;
                    }
                    clients_status[i] = has_prompt(t_buf) ? C_WRITING : C_READING;
                    string buf = html_encode(t_buf);
                    echo_msg(i, buf);
                }else if (clients_status[i] == C_WRITING && FD_ISSET(clients_fd[i], &wfds)){
                    char buf[BUF_SIZE] = {0};
                    if(fgets(buf, BUF_SIZE, f_batch[i]) == NULL){
                        printf("fgets cli close %d %d\n", i, clients_fd[i]);
                        close(clients_fd[i]);
                        FD_CLR(clients_fd[i], &n_wfds);
                        FD_CLR(clients_fd[i], &n_rfds);
                        clients_fd[i] = -1;
                        conn --;
                        continue;
                    }
                    echo_msg(i, "<b>" + html_encode(buf) + "</b>");
                    if(write(clients_fd[i], buf, strlen(buf)) <= 0){
                        printf("write cli close %d %d\n", i, clients_fd[i]);
                        close(clients_fd[i]);
                        FD_CLR(clients_fd[i], &n_wfds);
                        FD_CLR(clients_fd[i], &n_rfds);
                        clients_fd[i] = -1;
                        conn --;
                        continue;
                    }
                    clients_status[i] = C_READING;
                }
            }
        }
    }
    printf("done\n");
    for(int i=1;i<=5;i++){
        if(clients_fd[i] != -1){
            printf("close %d %d\n", i, clients_fd[i]);
            close(clients_fd[i]);
            perror("");
        }
    }
}

string html_encode(const char *buf){
    string res = "";
    int len = strlen(buf);
    for(int i=0;i<len;i++){
        res += escape(buf[i]);
    }
    return res;
}

string escape(char c){
    if(c == '\n') return "<br>";
    if(c == '\r') return "";
    if(c == ' ') return "&nbsp;";
    if(c == '\'') return "\\\'";
    if(c == '\"') return "\\\"";
    if(c == '<') return "&lt;";
    if(c == '>') return "&gt;";
    return string(1, c);
}

bool has_prompt(const char *buf){
    int len = strlen(buf);
    for(int i=0;i<len;i++){
        if(buf[i] == '%')
            return true;
    }
    return false;
}

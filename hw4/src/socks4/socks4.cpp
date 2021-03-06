#include "socks4.h"
#include "socks4_req.h"
#include "socks4_firewall.h"
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
using namespace std;
Socks4Server::Socks4Server(int _port, const string &_conf): port(_port), socks_conf(_conf){}
int Socks4Server::Connect(){
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        perror("socket creation failed");
        return -1;
    }
    sockaddr_in dest;
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
    if(listen(sockfd, 5) < 0){
        perror("listen failed");
        return -1;
    }
    return ReadConf();
}

int Socks4Server::ReadConf(){
    FILE *f = fopen(socks_conf.c_str(), "r");
    if(f == NULL) return -1;
    char op[10], mode[5], ip[32];
    int bits;
    while(~fscanf(f, "%s %s %[^/]/%d", op, mode, ip, &bits)){
        if(mode[0] == 'c'){
            firewall.Add(CONNECT_MODE, {ip, bits});
        }else if(mode[0] == 'b'){
            firewall.Add(BIND_MODE, {ip, bits});
        }
    }
    f = fopen("src_socks.conf", "r");
    while(~fscanf(f, "%s %s %[^/]/%d", op, mode, ip, &bits)){
        if(mode[0] == 'c'){
            src_firewall.Add(CONNECT_MODE, {ip, bits});
        }else if(mode[0] == 'b'){
            src_firewall.Add(BIND_MODE, {ip, bits});
        }
    }
    return 0;
}

void Socks4Server::Run(){
    firewall.Show();
    while(true){
        sockaddr_in cliaddr;
        int addr_len = sizeof(cliaddr);
        int clifd;
        if((clifd = accept(sockfd, (sockaddr*)&cliaddr, (socklen_t*)&addr_len)) < 0)
            exit(3);
        int pid = fork();
        if(pid == 0){
            close(sockfd);
            Handler(clifd, cliaddr);
            exit(0);
        }else{
            close(clifd);
        }
    }
}

void Socks4Server::Handler(int clifd, const sockaddr_in &cliaddr){
    char buf[BUF_SIZE]  = {0};
    int n = read(clifd, buf, BUF_SIZE);
    if(n < 8) exit(0);
    Socks4Request req(buf, cliaddr);
    if(firewall.Check((req.cd == 0x01) ? 0 : 1, req.dst_ip) == false){
        req.PrintMsg(buf);
        buf[1] = 0x5B;
        write(clifd, buf, 8);
    }else if(src_firewall.Check((req.cd == 0x01) ? 0 : 1, req.src_ip) == false){
        req.PrintMsg(buf);
        buf[1] = 0x5B;
        write(clifd, buf, 8);
    }else{
        req.permit = 1;
        req.PrintMsg(buf);
        if(req.cd == 0x01)
            ConnectHandler(clifd, req);
        else if(req.cd == 0x02)
            BindHandler(clifd, req);
    }
}

void Socks4Server::ConnectHandler(int clifd, const Socks4Request &req){
    char buf[BUF_SIZE] = {0};
    buf[0] = 0;
    buf[1] = 0x5A;
    buf[2] = (unsigned char)(req.dst_port / 256);
    buf[3] = (unsigned char)(req.dst_port % 256);
    for(int i=4;i<=7;i++)
        buf[i] = (unsigned char)req.get_dst_ip(i - 4);
    write(clifd, buf, 8);
    int dstfd = socket(AF_INET, SOCK_STREAM, 0);
    if(dstfd <= 0)
        exit(3);
    sockaddr_in dst_addr;
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = req.dst_ip;
    dst_addr.sin_port = htons(req.dst_port);
    if(connect(dstfd, (sockaddr*)&dst_addr, sizeof(dst_addr)) < 0){
        exit(3);
    }
    fd_set rfds, n_rfds;
    FD_ZERO(&rfds); FD_ZERO(&n_rfds);
    FD_SET(clifd, &rfds); FD_SET(dstfd, &rfds);
    FD_SET(clifd, &n_rfds); FD_SET(dstfd, &n_rfds);
    int max_fd = max(clifd, dstfd) + 1;
    vector<int> fds = {clifd, dstfd};
    int i=0;
    bool select_end = false;
    while(true){
        memcpy(&rfds, &n_rfds, sizeof(n_rfds));
        if(select(max_fd, &rfds, NULL, NULL, NULL) < 0){
            break;
        }
        for(int i=0;i<(int)fds.size();i++){
            if(FD_ISSET(fds[i], &rfds)){
                int n = read(fds[i], buf, BUF_SIZE);
                if(n <= 0){
                    FD_CLR(fds[i], &n_rfds);
                    select_end = true;
                    break;
                }
                req.PrintMsg(buf);
                n = write(fds[i^1], buf, n);
            }
        }
        if(select_end) break;
    }
    close(clifd); close(dstfd);
}

void Socks4Server::BindHandler(int clifd, const Socks4Request &req){
    srand(clock());
    int bindfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bindfd <= 0){
        exit(3);
    }
    int opt = 1;
    setsockopt(bindfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int)); 
    sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(INADDR_ANY);
    if(bind(bindfd, (sockaddr*)&bind_addr, sizeof(bind_addr)) < 0){
        exit(3);
    }
    sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if(getsockname(bindfd, (sockaddr*)&sa, &sa_len) < 0)
        exit(3);
    int bind_port = ntohs(sa.sin_port);
    if(listen(bindfd, 20) < 0){
        exit(3);
    }
    char buf[BUF_SIZE] = {0};
    buf[0] = 0;
    buf[1] = 0x5A;
    buf[2] = (unsigned char)(bind_port / 256);
    buf[3] = (unsigned char)(bind_port % 256);
    write(clifd, buf, 8);
    int dstfd;
    sockaddr_in dst_addr;
    socklen_t dst_len = sizeof(dst_addr);
    if((dstfd = accept(bindfd, (sockaddr*)&dst_addr, &dst_len)) < 0){
        exit(3);
    }
    write(clifd, buf, 8);
    fd_set rfds, n_rfds;
    FD_ZERO(&rfds); FD_ZERO(&n_rfds);
    FD_SET(clifd, &rfds); FD_SET(dstfd, &rfds);
    FD_SET(clifd, &n_rfds); FD_SET(dstfd, &n_rfds);
    int max_fd = max(clifd, dstfd) + 1;
    vector<int> fds = {clifd, dstfd};
    int select_end = false;
    while(true){
        memcpy(&rfds, &n_rfds, sizeof(rfds));
        if(select(max_fd, &rfds, NULL, NULL, NULL) < 0){
            break;
        }
        for(int i=0;i<(int)fds.size();i++){
            if(FD_ISSET(fds[i], &rfds)){
                memset(buf, 0, sizeof(buf));
                int n = read(fds[i], buf, BUF_SIZE);
                if(n <= 0){
                    FD_CLR(fds[i], &n_rfds);
                    select_end = true;
                    break;
                }
                req.PrintMsg(buf);
                n = write(fds[i^1], buf, n);
            }
        }
        if(select_end) break;
    }
    close(bindfd);
    close(dstfd);
    close(clifd);
}

void zombie_handler(int sig){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(){
    srand(time(0));
    signal(SIGCHLD, zombie_handler);
    Socks4Server socks4(SOCKS4_PORT, "socks4.conf");
    socks4.Connect();
    socks4.Run();
    return 0;
}

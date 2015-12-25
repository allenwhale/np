#include "socks4_req.h"
#include <bits/stdc++.h>
using namespace std;

Socks4Request::Socks4Request(const char *buf, const sockaddr_in &cliaddr): permit(0){
    vn = (unsigned char)buf[0];
    cd = (unsigned char)buf[1];
    dst_port = ((unsigned char)buf[2] << 8) | (unsigned char)buf[3];
    dst_ip = 0;
    for(int i=4;i<=7;i++)
        dst_ip |= (unsigned char)buf[i] << ((i - 4) * 8);
    user_id = string(buf + 8);
    src_ip = cliaddr.sin_addr.s_addr;
    src_port = cliaddr.sin_port;
}

int Socks4Request::get_dst_ip(int x) const {
    return (dst_ip >> (x * 8)) & 255;
}

string Socks4Request::get_dst_ip() const {
    in_addr addr;
    addr.s_addr = dst_ip;
    return string(inet_ntoa(addr));
}

string Socks4Request::get_src_ip() const {
    in_addr addr;
    addr.s_addr = src_ip;
    return string(inet_ntoa(addr));
}

void Socks4Request::PrintMsg(const string &t_buf) const {
    string buf = t_buf;
    printf("----- MSG START -----\n");
    printf("VN: %d\n", vn);
    printf("CD: %d(%s)\n", cd, (cd == 0x01) ? "Connect Mode" : "Bind Mode");
    printf("DST: %s(%d)\n", get_dst_ip().c_str(), dst_port);
    printf("SRC: %s(%d)\n", get_src_ip().c_str(), src_port);
    printf("REPLY: %d(%s)\n", permit, permit ? "GRANTED" : "DENIED");
    if((int)buf.size() > 100)
        buf.resize(100);
    printf("CONTENT:\n%s\n", buf.c_str());
    printf("----- MSG   END -----\n");
}

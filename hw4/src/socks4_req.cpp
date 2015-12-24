#include "socks4_req.h"
#include <bits/stdc++.h>
using namespace std;

Socks4Request::Socks4Request(const char *buf){
    vn = (unsigned char)buf[0];
    cd = (unsigned char)buf[1];
    dst_port = ((unsigned char)buf[2] << 8) | (unsigned char)buf[3];
    dst_ip = 0;
    for(int i=4;i<=7;i++)
        dst_ip |= (unsigned char)buf[i] << ((i - 4) * 8);
    user_id = string(buf + 8);
}

int Socks4Request::get_dst_ip(int x) const {
    return (dst_ip >> (x * 8)) & 255;
}

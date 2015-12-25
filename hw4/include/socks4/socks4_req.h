#ifndef __SOCKS4__REQ__H__
#define __SOCKS4__REQ__H__
#include <bits/stdc++.h>
#include <sys/types.h>
#include <arpa/inet.h>
using namespace std;
class Socks4Request{
    public:
        int vn, cd, dst_port, dst_ip, src_ip, src_port;
        int permit;
        string user_id;
        Socks4Request(const char *, const sockaddr_in &);
        int get_dst_ip(int) const;
        string get_src_ip() const;
        string get_dst_ip() const;
        void PrintMsg(const string &) const;
};
#endif

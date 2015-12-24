#ifndef __SOCKS4__REQ__H__
#define __SOCKS4__REQ__H__
#include <bits/stdc++.h>
using namespace std;
class Socks4Request{
    public:
        int vn, cd, dst_port, dst_ip;
        string user_id;
        Socks4Request(const char *);
        int get_dst_ip(int) const;
};
#endif

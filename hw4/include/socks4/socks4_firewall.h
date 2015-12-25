#ifndef __SOCKS4__FIREWALL__H__
#define __SOCKS4__FIREWALL__H__
#include <bits/stdc++.h>
using namespace std;
class Subnet{
    public:
        unsigned int mask;
        int bits;
        Subnet(const char *, int);
        bool Check(int);
};
#define CONNECT_MODE 0
#define BIND_MODE 1
class Socks4Firewall{
    public:
        vector<Subnet> whitelist[2];
        void Add(int, const Subnet &); 
        bool Check(int, int);
        void Show();
};
#endif

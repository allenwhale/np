#include "socks4_firewall.h"
#include <arpa/inet.h>
Subnet::Subnet(const char *ip, int _bits): bits(_bits) {
    mask = inet_addr(ip);
}

bool Subnet::Check(int ip){
    ip ^= mask;
    for(int i=0;i<bits;i++,ip >>= 1)
        if(ip & 1) return false;
    return true;
}

void Socks4Firewall::Add(int id, const Subnet &subnet){
    whitelist[id].push_back(subnet);
}

void Socks4Firewall::Show(){
    const char *name[] = {"Connect Mode", "Bind Mode"};
    for(int i=0;i<2;i++){
        printf("%s\n", name[i]);
        for(auto subnet: whitelist[i]){
            in_addr addr;
            addr.s_addr = subnet.mask;
            printf("%s/%d\n", inet_ntoa(addr), subnet.bits);
        }
    }
}

bool Socks4Firewall::Check(int id, int ip){
    if(whitelist[id].size() == 0) return true;
    for(auto subnet: whitelist[id]){
        if(subnet.Check(ip)) return true;
    }
    return false;
}

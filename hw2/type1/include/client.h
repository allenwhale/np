#ifndef __CLIENT__H__
#define __CLIENT__H__
#include <bits/stdc++.h>
//#include "shell.h"
class Shell;
class Client{
    public:
        bool used;
        int id, fd;
        std::string nickname;
        std::string ip;
        int port;
        Shell *shell;
        Client();
        Client(int, int, std::string, int);

};
#endif

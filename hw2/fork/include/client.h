#ifndef __CLIENT__H__
#define __CLIENT__H__
#include "msgq.h"
class Client{
    public:
        int fd;
        bool used;
        char nickname[256];
        char ip[64];
        int port;
        Client();
        void set_nickname(const char*);

};
#endif

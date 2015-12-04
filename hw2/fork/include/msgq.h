#ifndef __MSGQ__H__
#define __MSGQ__H__
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#define MAX_MESSAGE_SIZE 16
class Message{
    public:
        char msg[MAX_MESSAGE_SIZE][1024];
        int rear;
        key_t key;
        Message();
        void Init();
        void Flush();
        void Append(const char*);
};
#endif

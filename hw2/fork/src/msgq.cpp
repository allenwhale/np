#include "msgq.h"
#include <string.h>
#include <bits/stdc++.h>
#include "function.h"
#include "sem.h"
#include <unistd.h>

Message::Message(){
    Init();
}

void Message::Init(){
    memset(msg, 0, sizeof(msg));
    rear = 0;
}

void Message::Append(const char *new_msg){
    shm_sem_wait(id);
    memset(msg[rear], 0, sizeof(msg[rear]));
    memcpy(msg[rear], new_msg, strlen(new_msg));
    rear = (rear + 1) % MAX_MESSAGE_SIZE;
    shm_sem_post(id);
}

void Message::Flush(){
    shm_sem_wait(id);
    for(int i=0;i<rear;i++){
        printf("%s", msg[i]);
        FSTDOUT;
    }
    Init();
    shm_sem_post(id);
}

#include "msgq.h"
#include <string.h>
#include <bits/stdc++.h>
#include "function.h"
#include "sem.h"

Message::Message(){
    Init();
}

void Message::Init(){
    memset(msg, 0, sizeof(msg));
    rear = 0;
}

void Message::Append(const char *new_msg){
    int sem = shm_sem_open(key);
    shm_sem_wait(sem);
    memset(msg[rear], 0, sizeof(msg[rear]));
    memcpy(msg[rear], new_msg, strlen(new_msg));
    rear += 1;
    shm_sem_post(sem);
    shm_sem_close(sem);
}

void Message::Flush(){
    int sem = shm_sem_open(key);
    shm_sem_wait(sem);
    for(int i=0;i<rear;i++){
        printf("%s", msg[i]);
        FSTDOUT;
    }
    Init();
    shm_sem_post(sem);
    shm_sem_close(sem);
}

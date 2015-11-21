#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include "sem.h"
static struct sembuf op_lock[2] = {
    {2, 0, 0},
    {2, 1, SEM_UNDO}
};
static struct sembuf op_endcreate[2] = {
    {1, -1, SEM_UNDO},
    {2, -1, SEM_UNDO}
};
static struct sembuf op_open[1] = {
    {1, -1, SEM_UNDO}
};
static struct sembuf op_close[3] = {
    {2, 0, 0},
    {2, 1, SEM_UNDO},
    {1, 1, SEM_UNDO}
};
static struct sembuf op_unlock[1] = {
    {2, -1, SEM_UNDO}
};
static struct sembuf op_op[1] = {
    {0, 99, SEM_UNDO}
};
union semun{
    int             val;
    struct sen_id   *buff;
    ushort          *array;
} _semctl_arg;

int shm_sem_create(key_t key, int initval){
    int id = -1, semval;
    if(key == IPC_PRIVATE) return -1;
    if(key == (key_t)-1) return -1;
again:
    if((id = semget(key, 3, 0666|IPC_CREAT)) < 0){
        perror("semget");
        return -1;
    }
    if(semop(id, &op_lock[0], 2) < 0){
        if(errno == EINVAL) goto again;
        perror("can\'t lock");
        return -1;
    }
    if((semval = semctl(id, 1, GETVAL, 0)) < 0){
        perror("GETVAL");
        return -1;
    }
    if(semval == 0){
        _semctl_arg.val = initval;
        if(semctl(id, 0, SETVAL, _semctl_arg) < 0){
            perror("SETVAL 1");
            return -1;
        }
        _semctl_arg.val = BIGCOUNT;
        if(semctl(id, 1, SETVAL, _semctl_arg) < 0){
            perror("SETVAL 2");
            return -1;
        }
    }
    if(semop(id, &op_endcreate[0], 2) < 0){
        perror("end create error");
        return -1;
    }
    return id;
}

void shm_sem_rm(int id){
    if(semctl(id, 0, IPC_RMID, 0) < 0)
        perror("error IPC_RMID");
}

int shm_sem_open(key_t key){
    int id;
    if(key == IPC_PRIVATE) return -1;
    if(key == (key_t)-1) return -1;
    if((id = semget(key, 3, 0)) < 0)
        return -1;
    if(semop(id, &op_open[0], 1) < 0){
        perror("error open");
        return -1;
    }
    return id;

}

void shm_sem_close(int id){
    int semval;
    if(semop(id, &op_close[0], 3) < 0){
        perror("error semop");
        return ;
    }
    if((semval = semctl(id, 1, GETVAL, 0)) < 0){
       perror("error GETVAL"); 
       return ;
    }
    if(semval > BIGCOUNT){
        fprintf(stderr, "> BIGCOUNT\n");
        return ;
    }else if(semval == BIGCOUNT){
        shm_sem_rm(id);
    }else{
        if(semop(id, &op_unlock[0], 1) < 0){
            perror("error unlock");
        }
    }
}

void sem_op(int id, int value){
    if((op_op[0].sem_op = value) == 0) {
        fprintf(stderr, "value can\'t be 0\n");
        return ;
    }
    if(semop(id, &op_op[0], 1) < 0) {
        perror("erro semop");
        return ;
    }
}

void shm_sem_wait(int id){
    sem_op(id, -1);
}
void shm_sem_post(int id){
    sem_op(id, 1);
}


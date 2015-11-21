#ifndef __SHM__SEM__H__
#define __SHM__SEM__H__
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#define BIGCOUNT 10

int shm_sem_create(key_t, int);

void shm_sem_rm(int);

int shm_sem_open(key_t);

void shm_sem_close(int);

void shm_sem_op(int, int);

void shm_sem_wait(int);

void shm_sem_post(int);
#endif

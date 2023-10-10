/***** MySemaphores.cpp *****/
#include "MySemaphores.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

extern int idSem;

int sem_wait(int num)
{
	struct sembuf action;
	action.sem_num = num;
	action.sem_op = -1;
	action.sem_flg = 0;
	return semop(idSem,&action,1);
}

int sem_signal(int num)
{
	struct sembuf action;
	action.sem_num = num;
	action.sem_op = +1;
	action.sem_flg = 0;
	return semop(idSem,&action,1);
}

int sem_nowait(int num)
{
	struct sembuf action;
	action.sem_num = num;
	action.sem_op = -1;
	action.sem_flg = IPC_NOWAIT;
	return semop(idSem,&action,1);
}
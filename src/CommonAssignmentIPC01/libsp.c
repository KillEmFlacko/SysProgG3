/*
 * Course: System Programming 2020/2021
 *
 * Lecturers:
 * Alessia		Saggese asaggese@unisa.it
 * Francesco	Moscato	fmoscato@unisa.it
 *
 * Group:
 * D'Alessio	Simone
 * Falanga		Armando		0622701140  a.falanga13@studenti.unisa.it
 * Fattore		Alessandro
 *
 * Copyright (C) 2021 - All Rights Reserved
 *
 * This file is part of SysProgG3.
 *
 * SysProgG3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SysProgG3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SysProgG3.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
  @file libsp.c
  */

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "CommonAssignmentIPC01/libsp.h"

#define ERRMSG_MAX_LEN 128

/* Request a shared memory */
int get_shm (key_t *chiave, char **ptr_shared, int dim)
{
	// Check if the segment exists
	if ((*ptr_shared = shmget(*chiave, dim, 0)) == -1)
	{
		// It does not exist... Creation of the shared memory
		if ((*ptr_shared = shmget(*chiave, dim, IPC_CREAT | IPC_EXCL)) != -1)
		{
			return 0;
		}
		// In case of error check the value of errno
		else if (errno == EEXIST)
		{
			if ((*ptr_shared = shmget(*chiave, dim, 0)) == -1)
			{
				perror("shmget error: the key already exists");
				return -1;
			}
		}
		else if (errno == EACCES)
		{
			perror("shmget error: the user does not have permission to access the shared memory segment");
			return -1;
		}
		else if (errno == EINVAL)
		{
			perror("shmget error: problem with the size of the segment");
			return -1;
		}
		else
		{
			perror("shmget error");
			return -1;
		}
	}

	return -1;
}

/** @brief Request a semaphore.
 * If the semaphore already exists for the provided key numsem and
 * initsem are ignored.
 *
 * @param chiave_sem key of the semaphore set
 * @param numsem number of semaphores in the set
 * @param initsem initial value of each new semaphore
 * @return the semaphore ID if OK, -1 on error
 */
int get_sem (key_t *chiave_sem, int numsem, int initsem)
{
	int semid;
	char error_string[ERRMSG_MAX_LEN];

	// Check if the semaphore exists
	if ((semid = semget(*chiave_sem, 0, 0)) == -1)
	{
		if(errno == ENOENT) {
			/*
			 * If the semaphore set does not exist
			 * create a new semaphore set
			 */
			if ((semid = semget(*chiave_sem, numsem, IPC_CREAT | IPC_EXCL | SEMPERM)) == -1){
				snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot lock resources",chiave_sem,numsem,initsem);
				perror(error_string);
				return -1;
			}
		} else {
			/*
			 * Semaphore set exists but something went wrong
			 */
			snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot lock resources",chiave_sem,numsem,initsem);
			perror(error_string);
			return -1;

		}
	}

	/*
	 * Load the array for semaphore set initialization
	 */
	struct sembuf *sbuf = (struct sembuf*)malloc(sizeof(struct sembuf) * numsem);
	for(int i = 0; i < numsem; i++){
		sbuf[i].sem_num = numsem;
		sbuf[i].sem_op = initsem;
		sbuf[i].sem_flg = 0;
	}

	/*
	 * Perform semaphore set initialization
	 */
	int status = 0;
	if ((status = semop(semid, sbuf, numsem)) == -1) {
		snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot lock resources",chiave_sem,numsem,initsem);
		perror(error_string);
		return -1;
	}

	return semid;
}

/** @brief Wait on the semaphore
 *
 * @param id_sem semaphore set ID
 * @param numsem index of the semaphore to call wait on (from 0 to sem_nsems-1)
 * @param flag operation flags (IPC_NOWAIT, SEM_UNDO)
 */
void wait_sem (int id_sem, int numsem, int flag)
{
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];
	struct sembuf sbuf;

	sbuf.sem_num = numsem;
	sbuf.sem_op = -1;
	sbuf.sem_flg = flag;

	while((status = semop(id_sem, &sbuf, 1)) == -1 && errno == EINTR);

	if(status == -1)
	{
		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"wait_sem(id_sem: %d,numsem: %d,flag: %d) - Cannot lock resources",id_sem,numsem,flag);
		perror(error_string);
	}
}

/** @brief Signal on a semaphore
 *
 * @param id_sem semaphore set ID
 * @param numsem index of the semaphore to signal (from 0 to sem_nsems-1)
 * @param flag operation flags (IPC_NOWAIT, SEM_UNDO)
 */
void signal_sem (int id_sem, int numsem, int flag){
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	struct sembuf op;
	op.sem_num = numsem;
	op.sem_op = 1; // Signal operation, releasing resources
	op.sem_flg = flag; // WARNING: conversion from int to short

	if((status = semop(id_sem,&op,1)) == -1){
		/*
		 * EINTR is not managed here because we are releasing
		 * resources so no suspension is needed.
		 * The same discussion holds for EAGAIN.
		 */

		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"signal_sem(id_sem: %d,numsem: %d,flag: %d) - Cannot release resources",id_sem,numsem,flag);
		perror(error_string);
	}
}

/** @brief Remove a shared memory
 *
 * @param id_shared shared memory ID
 */
void remove_shm (int id_shared){
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((status = shmctl(id_shared,IPC_RMID,(struct shmid_ds *)NULL)) == -1){
		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"remove_shm(id_shared: %d) - Cannot remove shared memory",id_shared);
		perror(error_string);
	}
}

/** @brief Remove a semaphore
 *
 * @param id_sem semaphore set ID
 */
void remove_sem (int id_sem) {
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((status = semctl(id_sem,0,IPC_RMID)) == -1){
		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"remove_sem(id_sem: %d) - Cannot remove semaphore",id_sem);
		perror(error_string);
	}
}

/* async send on a message queue*/
void send_asyn(int msg_qid, Message *PTR_mess, int send_flag){
	return;
}

/* async send on a message queue*/
void send_sync(int msg_qid, Message *messaggio, int flag){
	return;
}

/*async receive on a message queue*/
void receive_async (int msg_qid, Message *PTR_mess, int receive_flag){
	return;
}

/*sync send on a message queue*/
void receive_sync(int msg_qid, Message *messaggio, int flag){
	return;
}

/* remove a mailbox */
void remove_mailbox(int msg_qid){
	return;
}

Monitor *init_monitor(int ncond);	/*init  monitor :
inputs : numcond  to init;
outputs: Monitor *: */


/*Routine enter_monitor .
inputs : Monitor * mon ;
waits on mutex*/

void enter_monitor(Monitor *mon);

/*Routine leave_monitor  :
inputs: Monitor *mon ;
mutex.signal()*/

void leave_monitor(Monitor *mon);

/* Routine wait_cond  / signal_cond:.
inputs : Monitor *mon :  ;
cond_num : index of the condition var */

void wait_cond(Monitor *mon,int cond_num);

void signal_cond(Monitor *mon,int cond_num);

/*Routine remove_monitor */
void remove_monitor(Monitor *mon);

/*Routine IS_queue_empty : returns 1 if the condition variable queue is empty, 0 otherwise*/
/*inputs : Monitor *mon :
cond_num : number of condition variable*/
int IS_queue_empty(Monitor *mon,int cond_num);

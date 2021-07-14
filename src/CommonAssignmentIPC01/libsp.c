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
#include <errno.h>
#include <stdio.h>
<<<<<<< HEAD
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
=======
>>>>>>> 4ade1b6fe114ca753dd8860758d003464c7e3623
#include "CommonAssignmentIPC01/libsp.h"

#define ERRMSG_MAX_LEN 128

/* Request a shared memory */
<<<<<<< HEAD
int get_shm (key_t *chiave, char **ptr_shared, int dim)
{
	// Check if the segment exists
	if ((ptr_shared = shmget(chiave, dim, 0)) == -1) 
	{
		// It does not exist... Creation of the shared memory
		if ((ptr_shared = shmget(chiave, dim, IPC_CREAT | IPC_EXCL)) != -1)
		{
			return 0;
		}
		// In case of error check the value of errno
		else if (errno == EEXIST) 
		{
			if ((ptr_shared = shmget(chiave, dim, 0)) == -1)
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

/* Request a semaphore */
int get_sem (key_t *chiave_sem, int numsem, int initsem)
{
	int semid;
	struct sembuf sbuf;

	// Check if the semaphore exists
	if ((semid = semget(chiave_sem, 0, 0)) == -1) 
	{
		// It does not exist... Creation of the semaphore
		if ((semid = semget(chiave_sem, 1, IPC_CREAT | IPC_EXCL | S_IRUSR |
			S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) != -1)
		{
			sbuf.sem_num = numsem;
			sbuf.sem_op = initsem;
			sbuf.sem_flg = 0;

			if (semop(semid, &sbuf, 1) == -1) 
			{
				perror("semop error");
				return -1;
			}
		}
		// In case of error check the value of errno
		else if (errno == EEXIST) 
		{
			if ((semid = semget(chiave_sem, 0, 0)) == -1)
			{
				perror("semget error: the key already exists"); 
				return -1;
			}
		}
		else if (errno == EACCES) 
		{
			perror("semget error: the user does not have permission to access the set"); 
			return -1;
		}
		else if (errno == EINVAL) 
		{
			perror("semget error: problem with the number of semaphores of the set");
			return -1;
		}
		else 
		{
			perror("semget error");
			return -1;
		}
	}

	return semid;
}

/* wait on the semaphore */
void wait_sem (int id_sem, int numsem, int flag)
{
	struct sembuf sbuf;

	sbuf.sem_num = numsem;
	sbuf.sem_op = -1;
	sbuf.sem_flg = flag;

	if (semop(mysem, &sbuf, 1) == -1)
	{
		if (errno == EACCES)
		{
			perror("semop error: the calling process does not have the permissions required
              to perform the specified semaphore operations"); 
			exit(EXIT_FAILURE);
		}
		else if (errno == EINVAL)
		{
			perror("semop error: the semaphore set doesn't exist"); 
			exit(EXIT_FAILURE);
		}
		else if (errno == EAGAIN)
		{
			perror("semop error: the operation could not proceed immediately"); 
			exit(EXIT_FAILURE);
		}
		else
		{
			perror("semop error"); 
			exit(EXIT_FAILURE);
		}
	}
}

/* signal on the semaphore */
void signal_sem (int id_sem, int numsem, int flag);
/* remove a shared memory */
void remove_shm (int id_shared);
/* remove a shared memory */
void remove_sem (int id_sem);
=======
int get_shm (key_t *chiave, char **ptr_shared, int dim){
	return 0;
}

/* Request a semaphore */
int get_sem (key_t *chiave_sem, int numsem, int initsem){
	return 0;
}

/* wait on the semaphore */
void wait_sem (int id_sem, int numsem, int flag){
	return;
}

/** @brief Signal on a semaphore
 *
 * @param id_sem semaphore set ID
 * @param numsem index of the semaphore to signal (from 0 to sem_nsems-1)
 * @param flag operation flags ( IPC_NOWAIT, SEM_UNDO)
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

>>>>>>> 4ade1b6fe114ca753dd8860758d003464c7e3623
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
<<<<<<< HEAD
void remove_mailbox(int msg_qid);

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
=======
void remove_mailbox(int msg_qid){
	return;
}
>>>>>>> 4ade1b6fe114ca753dd8860758d003464c7e3623

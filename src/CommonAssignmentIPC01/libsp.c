/*
 * Course: System Programming 2020/2021
 *
 * Lecturers:
 * Alessia		Saggese asaggese@unisa.it
 * Francesco	Moscato	fmoscato@unisa.it
 *
 * Group:
 * D'Alessio	Simone		0622701120	s.dalessio8@studenti.unisa.it
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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#ifdef DEBUG
#include <unistd.h>
#endif
#include "CommonAssignmentIPC01/libsp.h"

#define ERRMSG_MAX_LEN 128

/**
 * @brief Request a shared memory area and attach to process address space
 *
 * @param chiave key of the shared memory
 * @param ptr_shared if all OK points to the start of shared memory area
 * @param dim dimension in byte of the shared memory area (if created)
 * @retval shared memory ID if OK, -1 on error
 */
int get_shm (key_t *chiave, char **ptr_shared, int dim)
{
	int shmid;
	void *area;
	char error_string[ERRMSG_MAX_LEN];

	// Check if the segment exists
	if ((shmid = shmget(*chiave, dim, SEMPERM)) == -1)
	{
		if(errno == ENOENT)
		{
#ifdef DEBUG
			write(STDERR_FILENO,"Creation of a shared memory segment\n",27);
#endif
			// It does not exist... Creation of the shared memory
			if ((shmid = shmget(*chiave, dim, IPC_CREAT | IPC_EXCL | SEMPERM)) == -1)
			{
				snprintf(error_string,ERRMSG_MAX_LEN,"get_shm(chiave: %p, ptr_shared: %p, dim: %d) - Cannot create shared memory",chiave,ptr_shared,dim);
				perror(error_string);
				return -1;
			}
		}
		else
		{
			/*
			 * Something unexpected happened
			 */
			snprintf(error_string,ERRMSG_MAX_LEN,"get_shm(chiave: %p, ptr_shared: %p, dim: %d) - Cannot create shared memory",chiave,ptr_shared,dim);
			perror(error_string);
			return -1;
		}
	}
#ifdef DEBUG
	write(STDERR_FILENO,"Function body\n",16);
#endif
	/*
	 * Attach shared memory area to process address space
	 */

	if ((area = shmat(shmid, NULL, 0)) == (void *)(-1))
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"get_shm(chiave: %p, ptr_shared: %p, dim: %d) - Cannot create shared memory",chiave,ptr_shared,dim);
		perror(error_string);
		return -1;
	}
	*ptr_shared = (char*)area;

	return shmid;
}

/**
 * @brief Request a semaphore.
 * If the semaphore already exists for the provided key numsem and
 * initsem are ignored.
 *
 * @param chiave_sem key of the semaphore set
 * @param numsem number of semaphores in the set
 * @param initsem initial value of each new semaphore
 * @retval the semaphore ID if OK, -1 on error
 */
int get_sem (key_t *chiave_sem, int numsem, int initsem)
{
	int semid;
	char error_string[ERRMSG_MAX_LEN];

	// Check if the semaphore exists
	if ((semid = semget(*chiave_sem, numsem, SEMPERM)) == -1)
	{
		fprintf(stderr,"OK\n");
		if(errno == ENOENT) {
			/*
			 * If the semaphore set does not exist
			 * create a new semaphore set
			 */
			if ((semid = semget(*chiave_sem, numsem, IPC_CREAT | IPC_EXCL | SEMPERM)) == -1) {
				snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot create semaphore set",chiave_sem,numsem,initsem);
				perror(error_string);
				return -1;
			}
		} else {
			/*
			 * Semaphore set exists but something went wrong
			 */
			snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot retrieve semaphore set",chiave_sem,numsem,initsem);
			perror(error_string);
			return -1;

		}
	}

	/*
	 * Load the array for semaphore set initialization
	 */
	unsigned short *set_array = (unsigned short*)malloc(sizeof(unsigned short) * numsem);
	for(int i = 0; i < numsem; i++) {
		set_array[i] = initsem;
	}

	/*
	 * Perform semaphore set initialization
	 */
	int status = 0;
	if ((status = semctl(semid, 0, SETALL, set_array)) == -1) {
		snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot init semaphore set",chiave_sem,numsem,initsem);
		perror(error_string);
		remove_sem(semid);
		return -1;
	}

	return semid;
}

/**
 * @brief Wait on the semaphore
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

/**
 * @brief Signal on a semaphore
 *
 * @param id_sem semaphore set ID
 * @param numsem index of the semaphore to signal (from 0 to sem_nsems-1)
 * @param flag operation flags (IPC_NOWAIT, SEM_UNDO)
 */
void signal_sem (int id_sem, int numsem, int flag) {
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	struct sembuf op;
	op.sem_num = numsem;
	op.sem_op = 1; // Signal operation, releasing resources
	op.sem_flg = flag; // WARNING: conversion from int to short

	if((status = semop(id_sem,&op,1)) == -1) {
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

/**
 * @brief Remove a shared memory
 *
 * @param id_shared shared memory ID
 */
void remove_shm (int id_shared) {
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((status = shmctl(id_shared,IPC_RMID,(struct shmid_ds *)NULL)) == -1) {
		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"remove_shm(id_shared: %d) - Cannot remove shared memory",id_shared);
		perror(error_string);
	}
}

/**
 * @brief Remove a semaphore
 *
 * @param id_sem semaphore set ID
 */
void remove_sem (int id_sem) {
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((status = semctl(id_sem,0,IPC_RMID)) == -1) {
		/*
		 * TODO: Maybe you should specify the error type cheking errno
		 */
		snprintf(error_string,ERRMSG_MAX_LEN,"remove_sem(id_sem: %d) - Cannot remove semaphore",id_sem);
		perror(error_string);
	}
}

/* async send on a message queue*/
void send_asyn(int msg_qid, Message *PTR_mess, int send_flag) {
	return;
}

/* async send on a message queue*/
void send_sync(int msg_qid, Message *messaggio, int flag) {
	return;
}

/*async receive on a message queue*/
void receive_async (int msg_qid, Message *PTR_mess, int receive_flag) {
	return;
}

/*sync send on a message queue*/
void receive_sync(int msg_qid, Message *messaggio, int flag) {
	return;
}

/* remove a mailbox */
void remove_mailbox(int msg_qid) {
	return;
}

/**
 * @brief Initialize monitor.
 *
 * @param ncond number of conditiions to support and to initialize
 * @retval reference to the monitor, NULL on error
 */
Monitor *init_monitor(int ncond)
{
	char error_string[ERRMSG_MAX_LEN];

	/*
	 * 	Creation of mutex semaphore set:
	 *	- The first semaphore is related to the mutex that garantee the access to the monitor
	 *	- The second semaphore is related to preemption
	 */
	key_t key_mutex = KEY(1);
	if(key_mutex == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot generate mutex key",ncond);
		perror(error_string);
		return NULL;
	}

	int mutex_sem;

#ifdef DEBUG
	fprintf(stderr,"Path to file: %s\n",TMP_FILE);
	fprintf(stderr,"Mutex key: %d\n",key_mutex);
#endif

	if ((mutex_sem = get_sem(&key_mutex, LEN_MUTEX, 0)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot init monitor",ncond);
		perror(error_string);
		return NULL;
	}

	/*
	 * 	The only semaphore initialized to 1 is the mutex one
	 */
	if (semctl(mutex_sem, I_MUTEX, SETVAL, 1) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot init monitor",ncond);
		perror(error_string);
		return NULL;
	}

	/*
	 * 	Creation of a semaphore set with ncond semaphore. All are initialized to 0
	 */
	key_t key_cond = KEY(2);
	if(key_cond == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot generate condition variables key",ncond);
		perror(error_string);
		return NULL;
	}

	int cond_sem;

#ifdef DEBUG
	fprintf(stderr,"Cond key: %d\n",key_cond);
#endif
	if ((cond_sem = get_sem(&key_cond, ncond, 0)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot init monitor",ncond);
		perror(error_string);
		return NULL;
	}

	/*
	 *	Is our responsibility to allocate and deallocate the structure
	 */
	Monitor* mon = (Monitor*)malloc(sizeof(Monitor));

	mon -> id_mutex = mutex_sem;
	mon -> numcond = ncond;
	mon -> id_cond = cond_sem;

	return mon;
}

/**
 * @brief Enter monitor
 * Simply wait on moutex
 *
 * @param mon pointer to the monitor
 */
void enter_monitor(Monitor *mon) {
	wait_sem(mon->id_mutex,I_MUTEX,0);
}

/**
 * @brief Leave monitor
 * Check for preemption and signals on the right semaphore
 *
 * @param mon pointer to the monitor
 */
void leave_monitor(Monitor *mon) {
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((status = semctl(mon->id_mutex,I_PREEMPT,GETNCNT)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"leave_monitor(mon: %p) - Cannot leave monitor",mon);
		perror(error_string);
		return;
	}

	if(status > 0)
	{
		signal_sem(mon->id_mutex,I_PREEMPT,0);
	}
	else
	{
		signal_sem(mon->id_mutex,I_MUTEX,0);
	}
}

/* Routine wait_cond  / signal_cond:.
inputs : Monitor *mon :  ;
cond_num : index of the condition var */

void wait_cond(Monitor *mon,int cond_num)
{
	int preempt_count;
	char error_string[ERRMSG_MAX_LEN];
	
	if ((preempt_count = semctl(mon -> id_mutex, I_PREEMPT, GETNCNT)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"wait_cond(mon: %p, cond_num: %d) - Cannot wait on resource", mon, cond_num);
		perror(error_string);
		return;
	}

	if (preempt_count > 0)
	{
		signal_sem(mon -> id_mutex, I_PREEMPT, 0);
	}
	else
	{
		signal_sem(mon -> id_mutex, I_MUTEX, 0);
	}

	wait_sem(mon -> id_cond, cond_num, 0);

}

void signal_cond(Monitor *mon,int cond_num)
{
	int is_empty = IS_queue_empty(mon, cond_num);
	char error_string[ERRMSG_MAX_LEN];
	if(is_empty == -1)
	{	
		snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
		perror(error_string);
		return;
	}
	if(!is_empty)
	{
		signal_sem(mon->id_cond, cond_num, 0);
		wait_sem(mon->id_mutex, I_PREEMPT, 0);
	}
	
}

/**
 * @brief Remove the monitor
 * 
 * @param mon pointer to the monitor
 */
void remove_monitor(Monitor *mon)
{
	remove_sem(mon->id_cond);
	remove_sem(mon->id_mutex);
	free((void*)mon);
}

/**
 * @brief Check if the condition queue is empty
 *
 * @param mon pointer to the monitor containing the condition variable
 * @param cond_num index of the condition variable
 * @retval 1 if the condition variable queue is empty, 0 otherwise
 */
int IS_queue_empty(Monitor *mon,int cond_num)
{
	int status = 0;
	char error_string[ERRMSG_MAX_LEN];

	if ((status = semctl(mon -> id_cond, cond_num, GETNCNT)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IS_queue_empty(mon: %p, cond_num: %d) - Cannot check if queue is empty", mon, cond_num);
		perror(error_string);
		return -1;
	}

	return (status == 0);
}

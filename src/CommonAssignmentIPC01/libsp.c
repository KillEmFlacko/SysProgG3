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
 * Fattore		Alessandro  0622701115  a.fattore@studenti.unisa.it
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
  @brief Library for CommonAssignmentIPC01
  */

#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "CommonAssignmentIPC01/libsp.h"

#define ERRMSG_MAX_LEN 128
#define MAX_RETRY 5
#define MAX_WAITU 1000

// ---------- SUPERFAST HASH ----------
// http://www.azillionmonkeys.com/qed/hash.html
#include <stdint.h>
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
	|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
		+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const char * data, int len) {
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (; len > 0; len--) {
		hash  += get16bits (data);
		tmp    = (get16bits (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	case 3:
		hash += get16bits (data);
		hash ^= hash << 16;
		hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
		hash += hash >> 11;
		break;
	case 2:
		hash += get16bits (data);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1:
		hash += (signed char)*data;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}
// ------------------------------------

/**
 * @brief Request a shared memory area and attach to process address space
 *
 * @param chiave key of the shared memory
 * @param ptr_shared if all OK points to the start of shared memory area
 * @param dim dimension in byte of the shared memory area (if created)
 * @param created 1 if newly created, 0 if not created
 * @retval shared memory ID if OK, -1 on error
 */
int get_shm (key_t *chiave, char **ptr_shared, int dim, int *created)
{
	int shmid;
	void *area;
	char error_string[ERRMSG_MAX_LEN];
	if(created != NULL) *created = 1;

	// Try to create a shm area
	if ((shmid = shmget(*chiave, dim, IPC_CREAT | IPC_EXCL | SEMPERM)) == -1)
	{
		if(errno == EEXIST)
		{
			if(created != NULL) *created = 0;
			// If exists then retrieve that shm area
			if ((shmid = shmget(*chiave, dim, SEMPERM)) == -1)
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
	if((semid = semget(*chiave_sem,numsem,SEMPERM | IPC_CREAT | IPC_EXCL)) == -1)
	{
		if(errno == EEXIST) {
			/*
			 * If a semaphore set exist
			 * return that set
			 */
			if ((semid = semget(*chiave_sem, numsem, SEMPERM)) == -1) {
				snprintf(error_string,ERRMSG_MAX_LEN,"get_sem(chiave_sem: %p,numsem: %d,initsem: %d) - Cannot create semaphore set",chiave_sem,numsem,initsem);
				perror(error_string);
				return -1;
			}
			return semid;
		} else {
			/*
			 * Cannot retrieve/create semaphore set
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
 * @retval 0 if all OK, -1 on error
 */
int wait_sem (int id_sem, int numsem, int flag)
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
		snprintf(error_string,ERRMSG_MAX_LEN,"wait_sem(id_sem: %d,numsem: %d,flag: %d) - Cannot lock resources",id_sem,numsem,flag);
		perror(error_string);
		return -1;
	}

	return 0;
}

/**
 * @brief Signal on a semaphore
 *
 * @param id_sem semaphore set ID
 * @param numsem index of the semaphore to signal (from 0 to sem_nsems-1)
 * @param flag operation flags (IPC_NOWAIT, SEM_UNDO)
 * @retval 0 if all OK, -1 on error
 */
int signal_sem (int id_sem, int numsem, int flag) {
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

		snprintf(error_string,ERRMSG_MAX_LEN,"signal_sem(id_sem: %d,numsem: %d,flag: %d) - Cannot release resources",id_sem,numsem,flag);
		perror(error_string);
		return -1;
	}

	return 0;
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

/**
 * @brief Send an async message on a queue
 *
 * @param msg_qid message queue ID
 * @param PTR_mess pointer to the structure of the message
 * @param send_flag operation flags (MSG_EXCEPT, MSG_NOERROR)
 *
 */
int send_async(int msg_qid, Message *PTR_mess, int send_flag)
{
	char error_string[ERRMSG_MAX_LEN];

	if (msgsnd(msg_qid, PTR_mess, MAX_MSGQUEUE_LEN, IPC_NOWAIT | send_flag) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"send_async(msg_qid: %d, PTR_mess: %p, send_flag: %d) - Cannot send an asynchronous message", msg_qid, PTR_mess, send_flag);
		perror(error_string);
		return -1;
	}
	return 0;
}

/**
 * @brief Send a sync message on a queue
 *
 * @param msg_qid message queue ID
 * @param messaggio pointer to the structure of the message
 * @param flag operation flags (MSG_EXCEPT, MSG_NOERROR)
 *
 */
int send_sync(int msg_qid, Message *messaggio, int flag) {
	int status;
	char error_string[ERRMSG_MAX_LEN];

	/*
	 * Actual message to send, pid of the sender in the message
	 */
	Message act_msg;
	act_msg.type = messaggio->type;
	snprintf(act_msg.data,MAX_MSGQUEUE_LEN,"%d-%s",getpid(),messaggio->data);

	if((status = msgsnd(msg_qid,&act_msg,MAX_MSGQUEUE_LEN,flag)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"send_sync(msg_qid: %d, messaggio: %p, flag: %d) - Cannot send message",msg_qid,messaggio,flag);
		perror(error_string);
		return -1;
	}

	/*
	 * Computing message hash for ack
	 */
	int ack_value = SuperFastHash(messaggio->data,strlen(messaggio->data));

	/*
	 * waiting for message received
	 */
	kill(getpid(),SIGSTOP);

	/*
	 * Rcv the ack
	 */
	Message ack;
	if((status = msgrcv(msg_qid,&ack,MAX_MSGQUEUE_LEN,messaggio->type,0)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"send_sync(msg_qid: %d, messaggio: %p, flag: %d) - No ACK received",msg_qid,messaggio,flag);
		perror(error_string);
		return -1;
	}

	/*
	 * Check the ack
	 */
	int msg_retry = 0, hash_rcv = 0;
	sscanf(ack.data,"%d-%d",&hash_rcv,&msg_retry);

	int my_retry = 0;
	unsigned int seed = time(NULL);
	while(hash_rcv != ack_value && my_retry < MAX_RETRY)
	{
		/*
		 * Maybe others processes are using my type so I have received one of
		 * their ACK, I will try to receive my ack MAX_RETRY times
		 */

		/*
		 * If the received ACK is not the right one I will reinsert
		 * it into the queue
		 */
		if(msg_retry < MAX_RETRY)
		{
			sprintf(ack.data,"%d-%d",hash_rcv,msg_retry+1);
			if((status = msgsnd(msg_qid,&ack,MAX_MSGQUEUE_LEN,flag)) == -1)
			{
				snprintf(error_string,ERRMSG_MAX_LEN,"send_sync(msg_qid: %d, messaggio: %p, flag: %d) - Cannot retry ack rcv",msg_qid,messaggio,flag);
				perror(error_string);
				return -1;
			}

			/*
			 * Wait for a random time within the msec in order
			 * to not pick always the same ack
			 */
			useconds_t usecs = rand_r(&seed) % MAX_WAITU;
			usleep(usecs);
		}

		/*
		 * Rcv the ack
		 */
		Message ack;
		if((status = msgrcv(msg_qid,&ack,MAX_MSGQUEUE_LEN,messaggio->type,0)) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"send_sync(msg_qid: %d, messaggio: %p, flag: %d) - No ACK received",msg_qid,messaggio,flag);
			perror(error_string);
			return -1;
		}

		sscanf(ack.data,"%d-%d",&hash_rcv,&msg_retry);
		my_retry++;
	}

	if(ack_value != hash_rcv)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"send_sync(msg_qid: %d, messaggio: %p, flag: %d) - No ACK received",msg_qid,messaggio,flag);
		perror(error_string);
		return -1;
	}

	return 0;
}

/**
 * @brief Receive an async message on a queue
 *
 * @param msg_qid message queue ID
 * @param PTR_mess pointer to the structure of the message
 * @param receive_flag operation flags (MSG_EXCEPT, MSG_NOERROR)
 *
 */
int receive_async (int msg_qid, Message *PTR_mess, int receive_flag)
{
	char error_string[ERRMSG_MAX_LEN];

	if (msgrcv(msg_qid, PTR_mess, MAX_MSGQUEUE_LEN, PTR_mess -> type, IPC_NOWAIT | receive_flag) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"receive_async(msg_qid: %d, PTR_mess: %p, receive_flag: %d) - Cannot receive an asynchronous message", msg_qid, PTR_mess, receive_flag);
		perror(error_string);
		return -1;
	}
	return 0;
}

/**
 * @brief Receive a sync message on a queue
 *
 * @param msg_qid message queue ID
 * @param messaggio pointer to the structure of the message
 * @param flag operation flags (MSG_EXCEPT, MSG_NOERROR)
 *
 */
int receive_sync(int msg_qid, Message *messaggio, int flag) {
	int status;
	char error_string[ERRMSG_MAX_LEN];


	/*
	 * Rcv the actual message
	 */
	Message act_msg;
	if((status = msgrcv(msg_qid,&act_msg,MAX_MSGQUEUE_LEN,\
						messaggio->type,flag)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"receive_sync(msg_qid: %d, messaggio: %p, flag: %d) - Cannot receive message",msg_qid,messaggio,flag);
		perror(error_string);
		return -1;
	}

	/*
	 * Separate sender pid from the actual message
	 */
	pid_t to_cont;
	sscanf(act_msg.data,"%d-%s",&to_cont,messaggio->data);

	/*
	 * Compute and send ack
	 */
	Message ack;
	ack.type = messaggio->type;
	int ack_value = SuperFastHash(messaggio->data,strlen(messaggio->data));
	sprintf(ack.data,"%d-%d",ack_value,0);

	if((status = msgsnd(msg_qid,&ack,MAX_MSGQUEUE_LEN,0)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"receive_sync(msg_qid: %d, messaggio: %p, flag: %d) - Cannot send ACK",msg_qid,messaggio,flag);
		perror(error_string);
		return -1;
	}

	/*
	 * Wake up the sender
	 */
	kill(to_cont,SIGCONT);
	return 0;
}

/**
 * @brief Create a new mailbox
 *
 * @param chiave_msg key of the message queue
 * @retval message queue id if all OK, -1 on error
 */
int get_mailbox(key_t *chiave_msg)
{
	int msg_qid;
	char error_string[ERRMSG_MAX_LEN];

	if ((msg_qid = msgget(*chiave_msg, IPC_CREAT | SEMPERM)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"get_mailbox(key_t: %p) - Cannot get a mailbox", chiave_msg);
		perror(error_string);
		return -1;
	}

	return msg_qid;
}

/**
 * @brief Remove the selected mailbox
 *
 * @param msg_qid ID of the queue to remove
 *
 */
void remove_mailbox(int msg_qid)
{
	char error_string[ERRMSG_MAX_LEN];

	if (msgctl(msg_qid, IPC_RMID, NULL) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"remove_mailbox(msg_qid: %d) - Cannot remove the selected mailbox", msg_qid);
		perror(error_string);
		return;
	}

}

/**
 * @brief Initialize monitor.
 *
 * @param ncond number of conditiions to support and to initialize
 * @retval reference to the monitor, NULL on error
 */
Monitor *init_monitor(key_t *key, int ncond)
{
	char error_string[ERRMSG_MAX_LEN];

	int shmid;
	int created;
	Monitor *mon;

	/*
	 * Get shared memory area in order to share the monitor
	 */
	if((shmid = get_shm(key,(char**)&mon,sizeof(Monitor),&created)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot get shared memory area",ncond);
		perror(error_string);
		return NULL;
	}

	/*
	 * If it is not a newly created area is not
	 * my responsability to initialize it
	 */
	if(!created) return mon; 

	/*
	 * Save the newly created shared memory ID in order
	 * to remove it later.
	 */
	mon->id_shm = shmid;

	/*
	 * 	Creation of mutex semaphore set:
	 *	- The first semaphore is related to the mutex that garantee the access to the monitor
	 *	- The second semaphore is related to preemption
	 */
	key_t key_mutex = IPC_PRIVATE;
	if(key_mutex == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"init_monitor(ncond: %d) - Cannot generate mutex key",ncond);
		perror(error_string);
		return NULL;
	}

	int mutex_sem;

#ifdef DEBUG
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
	key_t key_cond = IPC_PRIVATE;
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
	char error_string[ERRMSG_MAX_LEN];

	if(wait_sem(mon->id_mutex,I_MUTEX,0) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"enter_monitor(mon: %p) - Cannot enter monitor",mon);
		perror(error_string);
		return;
	}
}

/**
 * @brief Leave monitor
 * Check for preemption and signals on the right semaphore
 *
 * @param mon pointer to the monitor
 */
void leave_monitor(Monitor *mon) {
	int preempt_count = 0;
	char error_string[ERRMSG_MAX_LEN];

	if((preempt_count = semctl(mon->id_mutex,I_PREEMPT,GETNCNT)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"leave_monitor(mon: %p) - Cannot leave monitor",mon);
		perror(error_string);
		return;
	}

	if (preempt_count > 0)
	{
		if(signal_sem(mon -> id_mutex, I_PREEMPT, 0) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"leave_monitor(mon: %p) - Cannot leave monitor",mon);
			perror(error_string);
			return;
		}
	}
	else
	{
		if(signal_sem(mon -> id_mutex, I_MUTEX, 0) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"leave_monitor(mon: %p) - Cannot leave monitor",mon);
			perror(error_string);
			return;
		}
	}
}

/**
 * @brief Wait on a specified condition of Monitor
 *
 * @param mon pointer to the struct of the Monitor
 * @param cond_num number of semaphore of the set associated to the condition to be considered
 * @retval 0 if all OK, -1 on error
 */
int wait_cond(Monitor *mon,int cond_num)
{
	int preempt_count;
	char error_string[ERRMSG_MAX_LEN];

	if ((preempt_count = semctl(mon -> id_mutex, I_PREEMPT, GETNCNT)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"wait_cond(mon: %p, cond_num: %d) - Cannot wait on resource", mon, cond_num);
		perror(error_string);
		return -1;
	}

	if (preempt_count > 0)
	{
		if(signal_sem(mon -> id_mutex, I_PREEMPT, 0) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"wait_cond(mon: %p, cond_num: %d) - Cannot wait on resource", mon, cond_num);
			perror(error_string);
			return -1;
		}
	}
	else
	{
		if(signal_sem(mon -> id_mutex, I_MUTEX, 0) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"wait_cond(mon: %p, cond_num: %d) - Cannot wait on resource", mon, cond_num);
			perror(error_string);
			return -1;
		}
	}

	if(wait_sem(mon -> id_cond, cond_num, 0) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"wait_cond(mon: %p, cond_num: %d) - Cannot wait on resource", mon, cond_num);
		perror(error_string);
		return -1;
	}

	return 0;
}

/**
 * @brief Signal on a specified condition of Monitor
 *
 * @param mon pointer to the struct of the Monitor
 * @param cond_num number of semaphore of the set associated to the condition to be considered
 * @retval 0 if all OK, -1 on error
 */
int signal_cond(Monitor *mon,int cond_num)
{
	int is_empty = IS_queue_empty(mon, cond_num);
	char error_string[ERRMSG_MAX_LEN];
	if(is_empty == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
		perror(error_string);
		return -1;
	}
	if(!is_empty)
	{
		if(signal_sem(mon->id_cond, cond_num, 0) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
			perror(error_string);
			return -1;
		}
		if(wait_sem(mon->id_mutex, I_PREEMPT, 0) == -1)
		{
			wait_sem(mon->id_cond,cond_num,0); // If signal is ok and we are in an atomic procedure no problem can occur on cond semaphore
			snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
			perror(error_string);
			return -1;
		}
	}

	return 0;
}

/**
 * @brief Signal broadcast on a specified condition of Monitor
 *
 * @param mon pointer to the struct of the Monitor
 * @param cond_num number of semaphore of the set associated to the condition to be considered
 * @retval 0 if all OK, -1 on error
 */
int broadcast_cond(Monitor *mon, int cond_num)
{
	int is_empty = IS_queue_empty(mon, cond_num);
	char error_string[ERRMSG_MAX_LEN];
	if(is_empty == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
		perror(error_string);
		return -1;
	}
	if(!is_empty)
	{
		/*
		 * Get number of processes in queue
		 */
		int n_queue = semctl(mon -> id_cond, cond_num, GETNCNT);
		if(n_queue == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot get num procs in queue", mon, cond_num);
			perror(error_string);
			return -1;
		}

		/*
		 * wake up each queued process
		 */
		for(int i = 0; i < n_queue; i++)
		{

			/*
			 * Release a resource for each process in queue
			 */
			if(signal_sem(mon->id_cond, cond_num, 0) == -1)
			{
				snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal condition", mon, cond_num);
				perror(error_string);
				return -1;
			}

			if(wait_sem(mon->id_mutex, I_PREEMPT, 0) == -1)
			{
				wait_sem(mon->id_cond,cond_num,0); // If signal is ok and we are in an atomic procedure no problem can occur on cond semaphore
				snprintf(error_string,ERRMSG_MAX_LEN,"signal_cond(mon: %p, cond_num: %d) - Cannot signal the selected semaphore", mon, cond_num);
				perror(error_string);
				return -1;
			}
		}
	}

	return 0;
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
	remove_shm(mon->id_shm);
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

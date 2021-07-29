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

/** @file libsp.h */

#ifndef _LIBSP_H
#define _LIBSP_H

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define MAX_MSGQUEUE_LEN 128
#define SEMPERM (S_IRUSR | S_IWUSR)

#define LEN_MUTEX 2 //!< Number of semaphores in mutex set
#define I_MUTEX 0   //!< Index of the mutex semaphore in the set
#define I_PREEMPT 1 //!< Index of the preempt semaphore in the set (also called urgent)

/**
 * @brief Struct of a message in the message queue
 */
typedef struct Message_Struct
{
   long type; /**< Represents the type of the message */
   char data[MAX_MSGQUEUE_LEN]; /**< Contains the text of the message */
} Message;

/* Request a shared memory */
int get_shm(key_t *chiave, char **ptr_shared, int dim, int *created);
/* Request a semaphore */
int get_sem(key_t *chiave_sem, int numsem, int initsem);
/* wait on the semaphore */
int wait_sem(int id_sem, int numsem, int flag);
/* signal on the semaphore */
int signal_sem(int id_sem, int numsem, int flag);
/* remove a shared memory */
void remove_shm(int id_shared);
/* remove a shared memory */
void remove_sem(int id_sem);
/* async send on a message queue*/
int send_async(int msg_qid, Message *PTR_mess, int send_flag);
/* async send on a message queue*/
int send_sync(int msg_qid, Message *messaggio, int flag);
/*async receive on a message queue*/
int receive_async(int msg_qid, Message *PTR_mess, int receive_flag);
/*sync send on a message queue*/
int receive_sync(int msg_qid, Message *messaggio, int flag);
/* create a mailbox */
int get_mailbox(key_t *chiave_msg);
/* remove a mailbox */
void remove_mailbox(int msg_qid);

/* Monitor struct  :
   MUTEX ( sem )
   NUM_VAR_CONDITION (int)
   ID VAR CONDITION Semaphore (sem)
   (in this version all conditions are associated to the same semaphore */

/**
 * @brief Monitor structure
 */
typedef struct Monitor_Struct
{
	int id_shm;
   	int id_mutex; /**< ID of the mutex and preempt semaphore */
   	int numcond; /**< Number of avaliable conditions  */
   	int id_cond; /**< ID of the condition semaphores */
} Monitor;

Monitor *init_monitor(key_t *key,int ncond); /*init  monitor :
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

int wait_cond(Monitor *mon, int cond_num);

int signal_cond(Monitor *mon, int cond_num);

int broadcast_cond(Monitor *mon, int cond_num);

/*Routine remove_monitor */
void remove_monitor(Monitor *mon);

/*Routine IS_queue_empty : returns 1 if the condition variable queue is empty, 0 otherwise*/
/*inputs : Monitor *mon : 
cond_num : number of condition variable*/
int IS_queue_empty(Monitor *mon, int cond_num);

#endif

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
  @file readwrite_file.c
  @brief Performs the reader/writer problem in the case that the shared resource is a variable
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/lib.h"

#define NCOND 3             // Number of semaphores
#define S_WRITE 0           // Semaphore for writing lock
#define S_NUM_READERS 1     // Semaphore on the counter variable
#define S_QUEUE 2           // Semaphore for the process queue

#define BUFFER_SIZE 100

/**
 * @brief Definition of the queue struct composed by a circular array
 * 
 * @param batch array of dimension BUFFER_SIZE that contains element
 * @param front index of the youngest element of the queue
 * @param back index of the oldest element of the queue
 * @param num_elements inumber of elements presents in the queue
 * 
 */
typedef struct {
    int batch[BUFFER_SIZE];
    int front;
    int back;
    int num_elements;
} Queue;

/**
 * @brief Initialization of the queue
 * 
 * @param q pointer to the queue struct
 * 
 */
Queue* initQueue(Queue* q)
{
    q -> front = q -> back = 0;
    q -> num_elements = 0;
    return q;
}

/**
 * @brief Adding an element to the front of the queue
 * 
 * @param q pointer to the queue struct
 * @param val value to add
 * 
 */
int push(Queue* q, int val)
{
    if((++(q -> num_elements)) == BUFFER_SIZE) return -1;
    q -> batch [q -> front] = val;
    q -> front = (q -> front + 1) % BUFFER_SIZE;
    return 0;
}

/**
 * @brief Getting the element to the back of the queue
 * 
 * @param q pointer to the queue struct
 * 
 */
int pop(Queue* q)
{
    if (q -> num_elements == 0) return -1;
    (q -> num_elements)--;
    int returnValue = q -> batch [q -> back];
    q -> back = (q -> back + 1) % BUFFER_SIZE;
    return returnValue;
}

/**
 * @brief Adds an element to the queue
 * 
 * @param sem set of semaphores associated to the problem
 * @param q pointer to the queue struct
 * @param new_val new value to add
 * 
 */
void writer(int sem, Queue *q, int new_val)
{
    printf("\n### STARTING WRITER PROCESS ###\n");

    printf("I queue for writing\n");
    wait_sem(sem, S_QUEUE, 0);

    printf("When it is my turn I ask for the exclusive access to the resource\n");
    wait_sem(sem, S_WRITE, 0);

    printf("Resource get. Exiting from the waiting queue\n");
    signal_sem(sem, S_QUEUE, 0);

    printf("<< CRITICAL SECTION >>\n*\n*\n");
    // ***
    sleep(12);
    // ***
    push(q, new_val);
    printf("\n*\n*\n<< EXIT SECTION >>\n");

    signal_sem(sem, S_WRITE, 0);
    
    printf("\n### END OF WRITER PROCESS ###\n");
}

/**
 * @brief Gets an element from the queue removing it
 * 
 * @param sem set of semaphores associated to the problem
 * @param nr counter of the number of readers
 * @param q pointer to the queue struct
 * 
 */
void reader(int sem, int *nr, Queue *q)
{
    printf("\n### STARTING READER PROCESS ###\n");

    printf("I queue for reading\n");
    wait_sem(sem, S_QUEUE, 0);

    printf("I request the esclusive access to the reader counter (+)\n");
    wait_sem(sem, S_NUM_READERS, 0);
    ++(*nr);
    printf("Number of readers: %d\n", *nr);
    if (*nr == 1)
    {
        printf("I'm the first reader\n");
        wait_sem(sem, S_WRITE, 0);
    }
    printf("Exiting from the waiting queue\n");
    signal_sem(sem, S_QUEUE, 0);
    printf("I release the access to the reader counter\n");
    signal_sem(sem, S_NUM_READERS, 0);

    printf("<< CRITICAL SECTION >>\n");

    printf("VALUE: %d\n", pop(q));

    printf("<< EXIT SECTION >>\n");

    printf("I request the esclusive access to the reader counter (-)\n");
    wait_sem(sem, S_NUM_READERS, 0);
    --(*nr);
    if (*nr == 0)
    {
        printf("I'm the last reader to release the resource\n");
        signal_sem(sem, S_WRITE, 0);
    }   
    printf("I release the access to the reader counter\n");
    signal_sem(sem, S_NUM_READERS, 0);

    printf("### END OF READING PROCESS ###\n\n");
}

int main(int argc, char **argv)
{
	key_t key0, key1, keysem;
    int sem;

    /************************************
    *   SHARED MEMORY AREA
    */
	Queue* q;
    int *n_readers;
    /*
    ************************************
    */

    fprintf(stdout,"Generating keys...\n");

    // Generating key for shared memory
	if ((key0 = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }
    if ((key1 = ftok(".", 101)) == -1) { perror("ftok"); exit(1); }
    if ((keysem = ftok(".", 104)) == -1) { perror("ftok"); exit(1); }

    fprintf(stdout,"Initializing shm...\n");

    // Attach shared memory to data
    get_shm(&key0, (char**)&q, sizeof(Queue), NULL);
    get_shm(&key1, (char**)&n_readers, sizeof(int), NULL);

    // Reset value in case of existing shm
    initQueue(q);
    *n_readers = 0;

    fprintf(stdout,"Initializing set of semaphores...\n");

    // Initializing a set of 3 semaphores with value 1
    sem = get_sem(&keysem, NCOND, 1);
    printf("S_WRITE: %d\n",semctl(sem, S_WRITE, GETVAL));
    printf("S_NUM_READERS: %d\n",semctl(sem, S_NUM_READERS, GETVAL));
    printf("S_QUEUE: %d\n",semctl(sem, S_QUEUE, GETVAL));

    sleep(2);

    int num = 0;

    if (fork() == 0)
    {
        fprintf(stdout,"SONS START\n");

        if (fork() == 0)
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, q);
            }
        }
        else
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, q);
            }
        }
        
    }
    else
    {
        fprintf(stdout,"FATHER STARTS\n");
        while (1) 
        {
            num++;
            sleep(3);
            writer(sem, q, num);
        }
    }
    
    return 0;
}

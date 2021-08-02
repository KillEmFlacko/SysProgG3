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
  @file readwrite_array.c
  @brief Performs the reader/writer problem in the case that the shared resource is an array
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CommonAssignmentIPC01/libsp.h"

#define NCOND 3             // Number of semaphores
#define S_WRITE 0           // Semaphore for writing lock
#define S_NUM_READERS 1     // Semaphore on the counter variable
#define S_QUEUE 2           // Semaphore for the process queue

#define BUFFSIZE 10
#define BATCH 5
#define ERRMSG_MAX_LEN 128

/**
 * @brief Updates the value of a variable
 * 
 * @param sem set of semaphores associated to the problem
 * @param array pointer to the shared memory area
 * @param index index of the element of the array
 * @param new_val new value to assign at the variable
 * 
 */
void writer(int *sem, int *array, int index, int new_val)
{
    char error_string[ERRMSG_MAX_LEN];
    if (index < 0 || index > BUFFSIZE-1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, array: %p, index: %d, new_val: %d) - Index out of bound exception", *sem, array, index, new_val);
		perror(error_string);
        return;
	}

    printf("\n### STARTING WRITER PROCESS ###\n");

    int sem_index = index / BATCH;

    printf("I queue for writing\n");
    wait_sem(sem[sem_index], S_QUEUE, 0);

    printf("When it is my turn I ask for the exclusive access to the resource\n");
    wait_sem(sem[sem_index], S_WRITE, 0);

    printf("Resource get. Exiting from the waiting queue\n");
    signal_sem(sem[sem_index], S_QUEUE, 0);

    printf("<< CRITICAL SECTION >>\n*\n*\n");
    // ***
    sleep(12);
    // ***
    array[index] = new_val;
    printf("ARRAY: %d - [ ", sem_index);
    for (int i=sem_index*BATCH; i<(sem_index+1)*BATCH; i++)
    {
        printf("%d ", array[i]);
    }
    printf("]\n");
    printf("\n*\n*\n<< EXIT SECTION >>\n");

    signal_sem(sem[sem_index], S_WRITE, 0);
    
    printf("\n### END OF WRITER PROCESS ###\n");
}

/**
 * @brief Read the value of the variable 
 * 
 * @param sem set of semaphores associated to the problem
 * @param nr counter of numbers of readers
 * @param array pointer to the shared memory area
 * @param index index of the element of the array
 * 
 */
void reader(int *sem, int *nr, int *array, int index)
{
    char error_string[ERRMSG_MAX_LEN];
    if (index < 0 || index > BUFFSIZE-1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, nr: %d, array: %p, index: %d) - Index out of bound exception", *sem, *nr, array, index);
		perror(error_string);
        return;
	}

    printf("\n### STARTING READER PROCESS ###\n");

    int sem_index = index / BATCH;

    printf("I queue for reading\n");
    wait_sem(sem[sem_index], S_QUEUE, 0);

    printf("I request the esclusive access to the reader counter (+)\n");
    wait_sem(sem[sem_index], S_NUM_READERS, 0);
    ++(nr[sem_index]);
    printf("Number of readers: %d\n", *nr);
    if (nr[sem_index] == 1)
    {
        printf("I'm the first reader\n");
        wait_sem(sem[sem_index], S_WRITE, 0);
    }
    printf("Exiting from the waiting queue\n");
    signal_sem(sem[sem_index], S_QUEUE, 0);
    printf("I release the access to the reader counter\n");
    signal_sem(sem[sem_index], S_NUM_READERS, 0);

    printf("<< CRITICAL SECTION >>\n");

    printf("ARRAY: %d - [ ", sem_index);
    for (int i=sem_index*BATCH; i<(sem_index+1)*BATCH; i++)
    {
        printf("%d ", array[i]);
    }
    printf("]\n");

    printf("<< EXIT SECTION >>\n");

    printf("I request the esclusive access to the reader counter (-)\n");
    wait_sem(sem[sem_index], S_NUM_READERS, 0);
    --(nr[sem_index]);
    if (nr[sem_index] == 0)
    {
        printf("I'm the last reader to release the resource\n");
        signal_sem(sem[sem_index], S_WRITE, 0);
    }   
    printf("I release the access to the reader counter\n");
    signal_sem(sem[sem_index], S_NUM_READERS, 0);

    printf("### END OF READING PROCESS ###\n\n");
}

int main(int argc, char **argv)
{
    unsigned int n_batch = BUFFSIZE/BATCH;
    if (BUFFSIZE%BATCH != 0) { n_batch++; }

	key_t key0, key1, keysem[n_batch];
    int sem[n_batch];

    /************************************
    *   SHARED MEMORY AREA
    */
	int *array;
    int *n_readers;
    /*
    *************************************
    */

    fprintf(stdout,"Generating keys...\n");

    // Generating key for shared memory
	key0 = IPC_PRIVATE;
    key1 = IPC_PRIVATE;
    for (int i=0 ; i<n_batch; i++)
    {
        if ((keysem[i] = ftok(".", 104+i)) == -1) { perror("ftok"); exit(1); }
    }  

    fprintf(stdout,"Initializing shm...\n");

    // Attach shared memory to data
    get_shm(&key0, (char**)&array, sizeof(int)*BUFFSIZE, NULL);
    get_shm(&key1, (char**)&n_readers, sizeof(int)*n_batch, NULL);

    // Reset value in case of existing shm
    for(int i=0; i<n_batch; i++) { n_readers[i] = 0; }
    for(int i=0; i<BUFFSIZE; i++) { array[i] = 0; }

    fprintf(stdout,"Initializing set of semaphores...\n");

    // Initializing n_batch set of 3 semaphores with value 1
    for (int i=0 ; i<n_batch; i++)
    {
        sem[i] = get_sem(&keysem[i], NCOND, 1);
    }
    printf("S_WRITE: %d\n",semctl(sem[0], S_WRITE, GETVAL));
    printf("S_NUM_READERS: %d\n",semctl(sem[0], S_NUM_READERS, GETVAL));
    printf("S_QUEUE: %d\n",semctl(sem[0], S_QUEUE, GETVAL));

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
                reader(sem, n_readers, array, 4);
            }
        }
        else
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, array, 4);
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
            writer(sem, array, 4, num);
        }
    }
    
    return 0;
}

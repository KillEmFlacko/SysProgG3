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
  @brief Performs the reader/writer problem in the case that the shared resource is a file
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/lib.h"

#define NCOND 3             // Number of semaphores
#define S_WRITE 0           // Semaphore for writing lock
#define S_NUM_READERS 1     // Semaphore on the counter variable
#define S_QUEUE 2           // Semaphore for the process queue

#define ERRMSG_MAX_LEN 128
#define BUFFSIZE 128

/**
 * @brief Append a string in the file indicated
 * 
 * @param sem set of semaphores associated to the problem
 * @param str string to append at the end of the file
 * @param path path of the file
 * 
 */
void writer(int sem, char* str, char path)
{
    char error_string[ERRMSG_MAX_LEN];

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
    int fd = open(path, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);
    if (fd == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, str: %s) - Cannot write on file", sem, str);
		perror(error_string);
        return;
    }
    if(write(fd, str, strlen(str)) == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, str: %s) - Cannot write on file", sem, str);
		perror(error_string);
        return;
    }
    close(fd);

    printf("\n*\n*\n<< EXIT SECTION >>\n");

    signal_sem(sem, S_WRITE, 0);
    
    printf("### ENDING WRITER PROCESS ###\n\n");
}

/**
 * @brief Read the entire file 
 * 
 * @param sem set of semaphores associated to the problem
 * @param nr counter of numbers of readers
 * @param path path of the file
 * 
 */
void reader(int sem, int *nr, char path)
{
    char error_string[ERRMSG_MAX_LEN];

    printf("\n### STARTING READING PROCESS ###\n");

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

    int fd = open(path, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);
    if (fd == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, nr: %d) - Cannot read on file", sem, *nr);
		perror(error_string);
        return;
    }
    int status;
    char buffer[BUFFSIZE];
    while((status = (read(fd, buffer, sizeof(buffer)))>0))
    {
        printf("%s\n", buffer);
        if(status < 0)
        {
            snprintf(error_string,ERRMSG_MAX_LEN,"writer(sem: %d, nr: %d) - Cannot read on file", sem, *nr);
		    perror(error_string);
            return;
        }   
    }
    
    close(fd);

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
	int *value = 0;
    int *n_readers = 0;
    /*
    *************************************
    */

    fprintf(stdout,"Generating keys...\n");

    // Generating key for shared memory
	if ((key0 = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }
    if ((key1 = ftok(".", 101)) == -1) { perror("ftok"); exit(1); }
    if ((keysem = ftok(".", 104)) == -1) { perror("ftok"); exit(1); }

    fprintf(stdout,"Initializing shm...\n");

    // Attach shared memory to data
    get_shm(&key0, (char**)&value, sizeof(int), NULL);
    get_shm(&key1, (char**)&n_readers, sizeof(int), NULL);

    // Reset value in case of existing shm
    *value = 0;
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
                reader(sem, n_readers, "pippo.txt");
            }
        }
        else
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, "pippo.txt");
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
            writer(sem, "CIAO", "pippo.txt");
        }
    }
    
    return 0;
}

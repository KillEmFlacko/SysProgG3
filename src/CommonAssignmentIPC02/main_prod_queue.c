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
  @file main_prod_queue.c
  @brief Performs the producer in the producer/consumer problem synchronized with monitor in the case that the shared resource is a queue in a shared memory
  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/queue.h"

#define NCOND 2
#define NOTFULL 0
#define NOTEMPTY 1

int id_shared = -1;
Monitor *monitor = NULL;

void exit_procedure(void)
{
	if(id_shared != -1) remove_shm(id_shared);
	if(monitor != NULL) remove_monitor(monitor);
}

int main(int argc, char **argv)
{
	atexit(exit_procedure);

	key_t key_mon = ftok(KEY_FILE,1);

	if((monitor = init_monitor(&key_mon,NCOND)) == NULL)
	{
		fprintf(stderr,"Cannot get monitor\n");
		exit(EXIT_FAILURE);
	}

	key_t key_shm = ftok(KEY_FILE,2);

	/*
	 * Create and attach shared memory area
	 */
	int created;
	Queue_TypeDef* shm_addr;
	if((id_shared = get_shm(&key_shm,(char**)&shm_addr,sizeof(Queue_TypeDef), &created)) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Init queue if this process has created the area
	 */
	if(created == 1)
	{
		enter_monitor(monitor);
		Queue_init(shm_addr);
		leave_monitor(monitor);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	int read_val;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		enter_monitor(monitor);

		/*
		 * Try to add an element in the queue
		 */
		if(Queue_enqueue(shm_addr,rand()) == -1)
		{
			if(wait_cond(monitor,NOTFULL) == -1)
			{
				fprintf(stderr,"Cannot wait for free space in queue\n");
				exit(EXIT_FAILURE);
			}
		}

		/*
		 * Print the produced element
		 */
		if(Queue_back(shm_addr,&read_val) == -1)
		{
			fprintf(stderr,"Cannot read from queue\n");
			exit(EXIT_FAILURE);
		}

		printf("Produced value: %d\n",read_val);

		/*
		 * Signal to consumer that there is something in the buffer
		 */
		if(signal_cond(monitor,NOTEMPTY) == -1)
		{
			fprintf(stderr,"Error while signaling new element in buffer\n");
			exit(EXIT_FAILURE);
		}

		leave_monitor(monitor);

		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

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
  @file main_prod.c
  @brief Performs the producer in the producer/consumer problem synchronized with only semaphores in the case that the shared resource is an integer variable in a shared memory
  */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"

int id_sem_notfull = -1, id_sem_notempty = -1, id_shared = -1;

void exit_procedure(void)
{
	if(id_shared != -1) remove_shm(id_shared);
	if(id_sem_notfull != -1) remove_sem(id_sem_notfull);
	if(id_sem_notempty != -1) remove_sem(id_sem_notempty);
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	atexit(exit_procedure);

	key_t key_shm = ftok(KEY_FILE,1);
	key_t key_notfull = ftok(KEY_FILE,2);
	key_t key_notempty = ftok(KEY_FILE,3);

	/*
	 * Create semaphore set for NOTFULL
	 */
	if((id_sem_notfull = get_sem(&key_notfull,1,1)) == -1)
	{
		fprintf(stderr,"Cannot get notfull semaphore\n");
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Create semaphore set for NOTEMPTY
	 */
	if((id_sem_notempty = get_sem(&key_notempty,1,0)) == -1)
	{
		fprintf(stderr,"Cannot get notempty semaphore\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Create and attach shared memory area
	 */
	int* shm_addr;
	if((id_shared = get_shm(&key_shm,(char**)&shm_addr,sizeof(int),NULL)) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		/*
		 * Wait for content to be consumed and buffer to be empty
		 */
		if(wait_sem(id_sem_notfull,0,0) == -1)
		{
			fprintf(stderr,"Error while waiting empty buffer\n");
			exit(EXIT_FAILURE);
		}

		*shm_addr = rand();
		printf("Produced value: %d\n",*shm_addr);

		/*
		 * Signal to consumer that there is something in the buffer
		 */
		if(signal_sem(id_sem_notempty,0,0) == -1)
		{
			fprintf(stderr,"Error while signaling full buffer\n");
			exit(EXIT_FAILURE);
		}
		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

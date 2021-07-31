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
  @file dining_philosophers.c
  @brief Program simulating the dining philosophers problem required in GroupAssignmentGIPC01
  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include "libsp.h"

#define NUM_PHILOSOPHERS 10
#define LEN 30

int phil_id[NUM_PHILOSOPHERS];	// Numerical id to differentiate philosophers
char text[NUM_PHILOSOPHERS][LEN];	// Resource

void* philosopher(int, void*);
void eat(int);
void init_sem(key_t *, int);
void start_eating(int);
void filltext();

/**
 * @brief Main method
 * Each philosopher to perform his action need to obtain two adiacent rows of the
 * resource that in the general problem represents the sticks.
 * Processes related to philosphers are started and after their termination is
 * expected.
 * 
 */
int main()
{
	key_t keysem;
	int sem;

	if ((keysem = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }

	filltext();
	
	sem = get_sem(&keysem, NUM_PHILOSOPHERS + 1, 1);

	for (int i=1; i < NUM_PHILOSOPHERS; i++) { signal_sem(sem, NUM_PHILOSOPHERS, 0); }

	printf("Number of chairs: %d\n",semctl(sem, NUM_PHILOSOPHERS, GETVAL));

	start_eating(sem);
	
	for(int i=0; i<NUM_PHILOSOPHERS; i++) { wait(NULL); }

	remove_sem(sem);

	return 0;
}

/**
 * @brief Creates all the processes associated to the task of each philosopher
 * 
 */
void start_eating(int sem)
{
	for (int i=0; i<NUM_PHILOSOPHERS; i++)
	{
		phil_id[i]=i;
		if (fork() == 0)
		{
			philosopher(sem, (void *)&phil_id[i]);
		}
	}
}

/**
 * @brief Definition of the operation that each philosopher should do
 *
 * @param num identificative number of the philosopher
 * 
 */
void* philosopher(int sem, void *num)
{
	int phil= *(int *)num;

	// The last semaphore is associated with the table
	wait_sem(sem, NUM_PHILOSOPHERS, 0);
	printf("\nPhilosopher %d has joined table", phil);

	// Dijkstra solution
	if (phil % 2) 
	{	
		// Take first the left stick and then the right
		wait_sem(sem, phil, 0);
		printf("\nPhilosopher %d take %s", phil, text[phil]);
		wait_sem(sem, (phil+1)%NUM_PHILOSOPHERS, 0);
		printf("\nPhilosopher %d take %s", phil, text[(phil+1)%NUM_PHILOSOPHERS]);
	}
	else
	{
		// Take first the right stick and then the left
		wait_sem(sem, (phil+1)%NUM_PHILOSOPHERS, 0);
		printf("\nPhilosopher %d take %s", phil, text[(phil+1)%NUM_PHILOSOPHERS]);
		wait_sem(sem, phil, 0);
		printf("\nPhilosopher %d take %s", phil, text[phil]);
	}

	eat(phil);
	sleep(3);
	printf("\nPhilosopher %d has finished eating", phil);

	signal_sem(sem, phil, 0);
	signal_sem(sem, (phil+1)%NUM_PHILOSOPHERS, 0);
	signal_sem(sem, NUM_PHILOSOPHERS, 0);

	exit(0);
}

/**
 * @brief Action performed by the philosopher
 *
 * @param phil identificative number of the philosopher
 * 
 */
void eat(int phil)
{
	printf("\nPhilosopher %d is eating", phil);
}

/**
 * @brief Fill the resource shared by philosophers
 *
 */
void filltext()
{
	for (int i=0; i < NUM_PHILOSOPHERS; i++)
	{
		char str1[LEN] = "stick number ";
		char str2[10];
		sprintf(str2, "%d", i);
		strcat(str1, str2);
		strcpy(text[i], str1);
	}
}
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
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define NUM_PHILOSOPHERS 10
#define LEN 30

sem_t table;	// Semaphore of the table
sem_t stick[NUM_PHILOSOPHERS];	// Set of semaphores. One for every row
int phil_id[NUM_PHILOSOPHERS];	// Numerical id to differentiate philosophers

char text[NUM_PHILOSOPHERS][LEN];	// Resource

void* philosopher(void*);
void eat(int);
void init_sem(sem_t *, sem_t *, int);
void start_eating(pthread_t *);
void filltext();

/**
 * @brief Main method
 * Each philosopher to perform his action need to obtain two adiacent rows of the
 * resource that in the general problem represents the sticks.
 * Thread related to philosphers are started and after their termination is
 * expected.
 * 
 */
int main()
{
	pthread_t tid[NUM_PHILOSOPHERS];

	filltext();
	
	init_sem(&table, stick, NUM_PHILOSOPHERS);
		
	start_eating(tid);
	
	for(int i=0; i<NUM_PHILOSOPHERS; i++)
		pthread_join(tid[i],NULL);

	printf("\nAll philosophers have eaten");

	return 0;
}

/**
 * @brief Initialize the semaphores
 *
 * @param table semaphore associated with the table
 * @param stick set of semaphores associated with the resources
 * @param num_sticks number of resources at disposal
 * 
 */
void init_sem(sem_t *table, sem_t *stick, int num_sticks)
{
  for (int i = 0; i < num_sticks; i++) 
  {
    	sem_init(&stick[i], 0, 1);
  }

  sem_init(table, 0, num_sticks - 1);
}

/**
 * @brief Creates the tread associated to the task of each philosopher
 *
 * @param tid pointer to the threads structure
 * 
 */
void start_eating(pthread_t *tid)
{
	for (int i=0; i<NUM_PHILOSOPHERS; i++)
	{
		phil_id[i]=i;
		pthread_create(&tid[i], NULL, philosopher, (void *)&phil_id[i]);
	}
}

/**
 * @brief Definition of the operation that each philosopher sholud do
 *
 * @param num identificative number of the philosopher
 * 
 */
void* philosopher(void *num)
{
	int phil= *(int *)num;

	sem_wait(&table);
	printf("\nPhilosopher %d has joined table", phil);

	// Dijkstra solution
	if (phil % 2) 
	{	
		// Take first the left stick and then the right
		sem_wait(&stick[phil]);
		printf("\nPhilosopher %d take %s", phil, text[phil]);
		sem_wait(&stick[(phil+1)%NUM_PHILOSOPHERS]);
		printf("\nPhilosopher %d take %s", phil, text[(phil+1)%NUM_PHILOSOPHERS]);
	}
	else
	{
		// Take first the right stick and then the left
		sem_wait(&stick[(phil+1)%NUM_PHILOSOPHERS]);
		printf("\nPhilosopher %d take %s", phil, text[(phil+1)%NUM_PHILOSOPHERS]);
		sem_wait(&stick[phil]);
		printf("\nPhilosopher %d take %s", phil, text[phil]);
	}

	eat(phil);
	sleep(3);
	printf("\nPhilosopher %d has finished eating", phil);

	sem_post(&stick[phil]);
	sem_post(&stick[(phil+1)%NUM_PHILOSOPHERS]);
	sem_post(&table);

	pthread_exit(0);
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
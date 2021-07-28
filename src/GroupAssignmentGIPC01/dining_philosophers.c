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

sem_t table;
sem_t stick[NUM_PHILOSOPHERS];
int phil_id[NUM_PHILOSOPHERS];

char text[NUM_PHILOSOPHERS][LEN];

void* philosopher(void*);
void eat(int);
void init_sem(sem_t *, sem_t *, int);
void start_eating(pthread_t *);
void filltext();

int main()
{
	pthread_t tid[NUM_PHILOSOPHERS];

	filltext();
	
	init_sem(&table, stick, NUM_PHILOSOPHERS);
		
	start_eating(tid);
	
	for(int i=0; i<NUM_PHILOSOPHERS; i++)
		pthread_join(tid[i],NULL);
}

void init_sem(sem_t *table, sem_t *stick, int num_forks)
{
  for (int i = 0; i < num_forks; i++) 
  {
    	sem_init(&stick[i], 0, 1);
  }

  sem_init(table, 0, num_forks - 1);
}

void start_eating(pthread_t *tid)
{
	for (int i=0; i<NUM_PHILOSOPHERS; i++)
	{
		phil_id[i]=i;
		pthread_create(&tid[i], NULL, philosopher, (void *)&phil_id[i]);
	}
}

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
		sem_wait(&stick[(phil+1)%5]);
		printf("\nPhilosopher %d take %s", phil, text[(phil+1)%5]);
	}
	else
	{
		// Take first the right stick and then the left
		sem_wait(&stick[(phil+1)%5]);
		sem_wait(&stick[phil]);
	}

	sem_wait(&stick[phil]);
	sem_wait(&stick[(phil+1)%5]);

	eat(phil);
	sleep(5);
	printf("\nPhilosopher %d has finished eating", phil);

	sem_post(&stick[(phil+1)%5]);
	sem_post(&stick[phil]);
	sem_post(&table);

	pthread_exit(0);
}

void eat(int phil)
{
	printf("\nPhilosopher %d is eating", phil);
}

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
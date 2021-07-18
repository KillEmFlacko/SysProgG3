/*
 * Course: System Programming 2020/2021
 *
 * Lecturers:
 * Alessia		Saggese asaggese@unisa.it
 * Francesco	Moscato	fmoscato@unisa.it
 *
 * Group:
 * D'Alessio	Simone
 * Falanga		Armando		0622701140  a.falanga13@studenti.unisa.it
 * Fattore		Alessandro
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "CommonAssignmentIPC01/libsp.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FAILURE_MESSAGE ANSI_COLOR_RED"Test FAILED"ANSI_COLOR_RESET

#define NCOND 3

int main(int argc, char **argv)
{
	/*
	 * Test monitor creation
	 */
	Monitor *mon = init_monitor(NCOND);
	assert(mon != NULL);

	/*
	 * Check if condition variables were initialized correctly
	 */
	unsigned short array[NCOND];
	if(semctl(mon->id_cond,2,GETALL,array) == -1)
	{
		perror("Cannot retrieve condition variables' values");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < NCOND; i++)
	{
		assert(array[i] == 0);
	}

	/*
	 * Check if mutex and preempt were initialized correctly
	 */
	if(semctl(mon->id_mutex,0,GETALL,array) == -1)
	{
		perror("Cannot retrieve mutex values");
		exit(EXIT_FAILURE);
	}

	assert(array[I_PREEMPT] == 0);
	assert(array[I_MUTEX] == 1);

#ifdef DEBUG
	fprintf(stderr,"Monitor initialized correctly\n");
#endif

	pid_t child;
	if((child = fork()) != 0)
	{
		/*
		 * PARENT
		 */
		sleep(1);

		enter_monitor(mon);
		assert(semctl(mon->id_mutex,I_MUTEX,GETVAL) == 0);

#ifdef DEBUG
		fprintf(stderr,"Parent acquired monitor, sending SIGCONT to child\n");
#endif

		kill(child,SIGCONT);
		wait_cond(mon,0);
		assert(semctl(mon->id_mutex,I_MUTEX,GETVAL) == 0);
		assert(semctl(mon->id_mutex,I_PREEMPT,GETVAL) == 0);
		assert(semctl(mon->id_mutex,I_PREEMPT,GETNCNT) == 1); // Child has been preempted

		/*
		 * Condition is verified, parent can do its things
		 * and then quit from the monitor.
		 */

		leave_monitor(mon);

		int retval;
		wait(&retval);
		
		/*
		 * Test if monitor is removed correctly
		 */
		int id_cond = mon->id_cond;
		int id_mutex = mon->id_mutex;
		remove_monitor(mon);
		assert(semctl(id_cond,0,GETVAL) == -1);
		assert(semctl(id_mutex,0,GETVAL) == -1);

		if(retval == EXIT_SUCCESS)
		{
			printf("["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "ANSI_COLOR_GREEN"Test OK\n"ANSI_COLOR_RESET,basename(argv[0]));
			exit(EXIT_SUCCESS);
		}
		else
		{
			printf("["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "ANSI_COLOR_RED"Test FAILED\n"ANSI_COLOR_RESET,basename(argv[0]));
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		/*
		 * CHILD
		 */

		/*
		 * Wait for OK from parent
		 */
#ifdef DEBUG
		fprintf(stderr,"Proc %d: Child paused\n",getpid());
#endif
		kill(getpid(), SIGSTOP);

		enter_monitor(mon);
		assert(semctl(mon->id_mutex,I_MUTEX,GETVAL) == 0);

		signal_cond(mon,0);
		assert(semctl(mon->id_mutex,I_MUTEX,GETVAL) == 0);
		assert(semctl(mon->id_mutex,I_PREEMPT,GETVAL) == 0);
		assert(semctl(mon->id_mutex,I_PREEMPT,GETNCNT) == 0); // No process is preempted so child has the mutex

		leave_monitor(mon);

		exit(EXIT_SUCCESS);
	}

}


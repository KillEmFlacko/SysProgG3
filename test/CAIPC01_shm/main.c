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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include "lib/lib.h"
#include "CommonAssignmentIPC01/libsp.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FAILURE_MESSAGE ANSI_COLOR_RED"Test FAILED"ANSI_COLOR_RESET

int main(int argc, char **argv){

	key_t key;
	int shmid;
	int pid;

	if ((key = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }

	if ((pid = fork()) == 0) 
	{

		char* shm1;
		
		assert((shmid = get_shm(&key, &shm1, 128)) != -1);
#ifdef DEBUG
		fprintf(stderr, "shmid: %d\n", shmid);
#endif

		return 0;

	}
	else 
	{
		wait(NULL);

		char* shm2;
		
		assert((shmid = get_shm(&key, &shm2, 128)) != -1);
#ifdef DEBUG
		fprintf(stderr, "shmid: %d\n", shmid);
#endif
		remove_shm(shmid);

	}

	printf("[%s] "ANSI_COLOR_GREEN"Test OK\n"ANSI_COLOR_RESET,basename(argv[0]));
	exit(EXIT_SUCCESS);
}


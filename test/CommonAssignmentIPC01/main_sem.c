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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include "CommonAssignmentIPC01/libsp.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FAILURE_MESSAGE ANSI_COLOR_RED"Test FAILED"ANSI_COLOR_RESET

#define LEN 8

int main(int argc, char **argv){
	key_t key = IPC_PRIVATE;

	/*
	 * Test semaphore set creation
	 */
	int id_sem = get_sem(&key,LEN,1);
	assert(id_sem != -1);

	unsigned short array[LEN];
	int status = semctl(id_sem,0,GETALL,array);
	assert(status != -1);
	for(int i = 0; i < LEN; i++)
	{
		assert(array[i] == 1);
	}

	/*
	 * Test wait
	 *
	 * IPC_NOWAIT should not cause an error, that is because
	 * no other process has this semaphore set.
	 */
	wait_sem(id_sem,0,IPC_NOWAIT);
	assert(semctl(id_sem,0,GETVAL) == 0);

	/*
	 * Test signal
	 *
	 * IPC_NOWAIT should not cause an error, that is because
	 * no other process has this semaphore set.
	 */
	signal_sem(id_sem,0,IPC_NOWAIT);
	assert(semctl(id_sem,0,GETVAL) == 1);

	/*
	 * Test remove semaphore
	 */
	remove_sem(id_sem);
	assert(semctl(id_sem,0,GETVAL) == -1);

	printf("["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "ANSI_COLOR_GREEN"Test OK\n"ANSI_COLOR_RESET,basename(argv[0]));
	exit(EXIT_SUCCESS);
}


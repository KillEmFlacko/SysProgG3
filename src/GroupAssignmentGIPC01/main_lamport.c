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

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "GroupAssignmentGIPC01/lamport.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"Usage:\n\t%s [number_of_procs]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int nprocs = atoi(argv[1]);

	int act_id;
	pid_t child_pid;
	for(act_id = 0; (child_pid = fork()) != 0 && act_id < nprocs; act_id++);

	if(child_pid == 0)
	{
		char string_id[128];
		snprintf(string_id,128,"%d",act_id);
		// TODO: path to executable to be fixed or direclty write the child task...
		execl("path/to/exec","lamport",string_id,(char*)NULL);
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i<nprocs ;i++)
	{
		int ret;
		wait(&ret);
		if(ret != EXIT_SUCCESS)
		{
			fprintf(stderr,"Something went wrong...\n");
			exit(ret);
		}
	}

	exit(EXIT_SUCCESS);
}

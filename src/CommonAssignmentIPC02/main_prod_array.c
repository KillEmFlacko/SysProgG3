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
#include <time.h>
#include "CommonAssignmentIPC02/array.h"
#include "CommonAssignmentIPC01/libsp.h"

#define NCOND 2
#define COND_NEWRES 0
#define COND_HASCONSUMED 1

Monitor *monitor = NULL;
Array_TypeDef *array = NULL;

void exit_procedure(void)
{
	putchar('\n');
	if(array != NULL)
	{
		if(Array_remove(array))
		{
			remove_monitor(monitor);
		}
	}
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	atexit(exit_procedure);

	if(argc < 3)
	{
		fprintf(stderr,"Usage:\t%s num_res num_cons\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int nres = atoi(argv[1]);
	int ncons = atoi(argv[2]);

	key_t key_monitor = ftok(KEY_FILE,1);
	key_t key_array = ftok(KEY_FILE,2);

	/*
	 * Create monitor
	 */
	if((monitor = init_monitor(&key_monitor,NCOND)) == NULL)
	{
		fprintf(stderr,"Cannot get monitor\n");
		exit(EXIT_FAILURE);
	}

	enter_monitor(monitor);

	/*
	 * Create array
	 */
	if((array = Array_init(&key_array,nres,ncons,NULL)) == NULL)
	{
		fprintf(stderr,"Cannot get array\n");
		exit(EXIT_FAILURE);
	}

	leave_monitor(monitor);

	/*
	 * Wait for user input to produce
	 */
	char c;
	int res;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		do
		{
			printf("Enter resource to use:");
			scanf("%d",&res);
			getchar();
		}
		while(!(res >= 0 && res < array->array_len));

		enter_monitor(monitor);

		/*
		 * wait until all the consumers have consumed
		 */
		while(Array_isDirty(array,res))
		{
			if(wait_cond(monitor,COND_HASCONSUMED) == -1)
			{
				fprintf(stderr, "Cannot wait on condition\n");
				exit(EXIT_FAILURE);
			}
		}

		/*
		 * Produce the resource
		 */
		array->array[res] = rand();
		printf("Produced value: %d\n",array->array[res]);

		/*
		 * Set all the dirty bits of the produced value
		 */
		for(int i = 0; i < array->n_consumers; i++)
		{
			Array_setDirty(array,res,i);
		}

		/*
		 * Signal to all the waiting processes that new values are
		 * avaliable
		 */
		if(broadcast_cond(monitor,COND_NEWRES) == -1)
		{
			fprintf(stderr, "Cannot signal on condition\n");
			exit(EXIT_FAILURE);
		}

		leave_monitor(monitor);

		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

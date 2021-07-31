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
  @file main_prod_array.c
  @brief Performs the producer in the producer/consumer problem synchronized with only semaphores in the case that the shared resource is a vector of integer variables in a shared memory
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
	int my_array_id;
	if((array = Array_init(&key_array,nres,ncons,&my_array_id)) == NULL)
	{
		fprintf(stderr,"Cannot get array\n");
		exit(EXIT_FAILURE);
	}

	leave_monitor(monitor);

	/*
	 * Wait for user input to consume
	 */
	char c;
	int res;

	printf("Press a key to consume (Ctrl-D to exit)");
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
		 * white while value is not dirty
		 */
		while(array->bitmap[my_array_id][res] == 0)
		{
			if(wait_cond(monitor,COND_NEWRES) == -1)
			{
				fprintf(stderr, "Cannot wait on condition\n");
				exit(EXIT_FAILURE);
			}
		}

		printf("Consumed value: %d\n",array->array[res]);

		/*
		 * Unset your dirty bit in order to allow new value from
		 * the producers
		 */
		Array_unsetDirty(array,res,my_array_id);

		if(broadcast_cond(monitor,COND_HASCONSUMED) == -1)
		{
			fprintf(stderr, "Cannot signal on condition\n");
			exit(EXIT_FAILURE);
		}

		leave_monitor(monitor);

		printf("Press a key to consume (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

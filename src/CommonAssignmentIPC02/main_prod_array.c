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

Array_TypeDef *array = NULL;

void exit_procedure(void)
{
	putchar('\n');
	if(array != NULL) Array_remove(array);
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	atexit(exit_procedure);

	if(argc < 2)
	{
		fprintf(stderr,"Usage:\t%s num_res\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int nres = atoi(argv[1]);

	key_t key_monitor = ftok(KEY_FILE,1);
	key_t key_array = ftok(KEY_FILE,2);

	/*
	 * Create array
	 */
	if((array = Array_init(&key_monitor,&key_array,nres,0)) == NULL)
	{
		fprintf(stderr,"Cannot get array\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	int res;
	int value;
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
		while(!(res >= 0 && res < Array_getLen(array)));

		value = rand();
		if(Array_produce(array,res,value) == -1)
		{
			fprintf(stderr, "Cannot produce value\n");
			exit(EXIT_FAILURE);
		}
		printf("Produced value: %d\n",value);

		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

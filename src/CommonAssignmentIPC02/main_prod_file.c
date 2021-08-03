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
#include <string.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "CommonAssignmentIPC02/file.h"

#define MAX_STR_LEN 256

IPC02_File_TypeDef *file = NULL;

void exit_procedure(void)
{
	putchar('\n');
	if(file != NULL) IPC02_File_remove(file);
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	atexit(exit_procedure);

	/*
	 * Get file path
	 */
	if(argc < 2)
	{
		fprintf(stderr,"Usage:\n\t%s file_path\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	char *filepath = argv[1];

	key_t key_shm = ftok(KEY_FILE,1);
	key_t key_monitor = ftok(KEY_FILE,2);

	/*
	 * Open file
	 */

	if((file = IPC02_File_init(&key_monitor, &key_shm, filepath)) == NULL)
	{
		fprintf(stderr,"Error while opening file\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	char string_produced[MAX_STR_LEN];
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		snprintf(string_produced,MAX_STR_LEN,"Stringa prodotta - %03d\n",rand()%100);
		printf("Produced string: %s\n",string_produced);

		IPC02_File_produce(file,string_produced,strlen(string_produced));

		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

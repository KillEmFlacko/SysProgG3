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
  @file random_chars.c
  @brief a C program that writes 1MByte of random chars in a binary files 
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>

#define ERRMSG_MAX_LEN 128
#define MEGABYTE 1000000
#define FILENAME "random_chars_file.bin"

int main(int argc, char* argv[]){
    int fd;
	char* file_name = FILENAME;
	char error_string[ERRMSG_MAX_LEN];

	// Open the file
    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 
    	S_IRUSR | S_IWUSR | S_IXUSR |
		S_IRGRP | S_IWGRP | S_IXGRP |
		S_IROTH | S_IWOTH | S_IXOTH);

	// Checking the opening of the file
	if(fd == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"open(file_name: %s) - Cannot open file",file_name);
		perror(error_string);
		exit(EXIT_FAILURE);
	}

	struct stat buf;
	// Checking the recovery of the file informations
	if(fstat(fd, &buf) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"fstat(file_name: %s, file_descriptor: %d) - Cannot retrieve informations from the file",file_name, fd);
		perror(error_string);
		exit(EXIT_FAILURE);
	}

	// Seed to change the execution each time
	srand(time(NULL));

	// Inserting 1MegaByte of random characters into the file
	while(buf.st_size < MEGABYTE)
	{	
		char random_char = rand() % (CHAR_MAX+1);
		if(write(fd, (void*) &random_char, sizeof(char)) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"write(file_name: %s, file_descriptor: %d) - Cannot write the character in the file",file_name, fd);
			perror(error_string);
			exit(EXIT_FAILURE);
		}
		// update the stat structure
		if(fstat(fd, &buf) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"fstat(file_name: %s, file_descriptor: %d) - Cannot retrieve informations from the file",file_name, fd);
			perror(error_string);
			exit(EXIT_FAILURE);
		}
	}

    // Closing file
    if(close(fd) == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"close(file_name: %s, file_descriptor: %d) - Cannot close the file",file_name, fd);
        perror(error_string);
        exit(EXIT_FAILURE);
    }	
    exit(EXIT_SUCCESS);
	 
}

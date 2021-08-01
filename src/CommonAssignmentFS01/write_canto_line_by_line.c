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
  @brief a C program that writes the whole first "Canto" of the "Divina Commedia", line by line, in a text files
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

#define ERRMSG_MAX_LEN 2048
#define MAX_LINE_LEN 128
#define PATH_MAX_LEN 1024
#define FILENAME_READ "data/primo_canto_div_comm.txt"
#define FILENAME_WRITE "/line_by_line.txt" // will be generated near the executable

int main(int argc, char* argv[]){
    int fd1, fd2;
    char error_string[ERRMSG_MAX_LEN];
	char* file_name1 = FILENAME_READ;
    char file_name2[PATH_MAX_LEN];
    // getting the path of the executable where to create the writing file
    ssize_t size = readlink("/proc/self/exe", file_name2, sizeof(file_name2)-1);
    if(size == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"readlink(file_name: %s) - Cannot open reading file","/proc/self/exe");
		perror(error_string);
        exit(EXIT_FAILURE);
    }
    file_name2[size] = '\0';
    int i = strlen(file_name2);
    // truncate at first occurence of /
    while(i >= 0)
    {
        if(file_name2[i] == '/')
        {
            file_name2[i] = '\0';
            break;
        }
        i--;
    }
    strcat(file_name2, FILENAME_WRITE);
    
	// Open the reading file
    fd1 = open(file_name1, O_RDONLY);
    // Checking the opening of the reading file
	if(fd1 == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"open(file_name: %s) - Cannot open reading file",file_name1);
		perror(error_string);
		exit(EXIT_FAILURE);
	}

    // Open the writing file
    fd2 = open(file_name2, O_WRONLY | O_CREAT | O_TRUNC, 
    	S_IRUSR | S_IWUSR | S_IXUSR |
		S_IRGRP | S_IWGRP | S_IXGRP |
		S_IROTH | S_IWOTH | S_IXOTH);

    // Checking the opening of the writing file
    if(fd2 == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"open(file_name: %s) - Cannot open writing file",file_name2);
		perror(error_string);
		exit(EXIT_FAILURE);
	}

    char letter;
    size_t bytes_read;
    i = 0;
    char buf[MAX_LINE_LEN];

    while(bytes_read = read(fd1, (void*) &letter, 1)) // when 0, EOF is encountered
    {
        // Checking errors
        if(bytes_read == -1)
        {
            snprintf(error_string,ERRMSG_MAX_LEN,"read(file_name: %s) - Cannot read a character from the file",file_name1);
            perror(error_string);
            exit(EXIT_FAILURE);
        }
        
        buf[i] = letter;
        // Writing
        if(letter == '\n')
        {
            i++;
            buf[i] = '\0';
            if(write(fd2, (void*)buf, strlen(buf)) == -1)
            {
                snprintf(error_string,ERRMSG_MAX_LEN,"write(file_name: %s) - Cannot write the character in the file",file_name2);
                perror(error_string);
                exit(EXIT_FAILURE);
            }

            // Resetting buffer
            for(int i=0; strlen(buf); i++)
                buf[i] = '\0';
            i = 0;
        }
        else
            i++;

        
    }

    // Closing files
    if(close(fd1) == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"close(file_name: %s, file_descriptor: %d) - Cannot close the file",file_name1, fd1);
        perror(error_string);
        exit(EXIT_FAILURE);
    }
    if(close(fd2) == -1)
    {
        snprintf(error_string,ERRMSG_MAX_LEN,"close(file_name: %s, file_descriptor: %d) - Cannot close the file",file_name2, fd2);
        perror(error_string);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}

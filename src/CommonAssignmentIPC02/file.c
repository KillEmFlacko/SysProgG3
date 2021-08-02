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

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "CommonAssignmentIPC02/file.h"
#include "CommonAssignmentIPC01/libsp.h"

#define ERRMSG_MAX_LEN 128

#define NCOND 1
#define COND_NEWDATA 0

IPC02_File_TypeDef* IPC02_File_init(key_t *key_mon, key_t *key_shm, const char* file_path)
{
	char error_string[ERRMSG_MAX_LEN];

	/*
	 * Create monitor
	 */
	Monitor *mon;
	if((mon = init_monitor(key_mon,NCOND)) == NULL)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_init(key_mon: %p, key_shm: %p, file_path: %p) - Cannot get monitor",key_mon,key_shm,file_path);
		perror(error_string);
		return NULL;
	}

	/*
	 * Enter monitor before doing anything on the file or other data
	 */
	enter_monitor(mon);

	/*
	 * Open the file
	 */
	int fd;
	if((fd = open(file_path, O_CREAT | O_RDWR, 0666)) == -1)
	{
		remove_monitor(mon);
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_init(key_mon: %p, key_shm: %p, file_path: %p) - Cannot open file",key_mon,key_shm,file_path);
		perror(error_string);
		return NULL;
	}

	/*
	 * Get shared memory area for the offset and counter
	 */
	int shm_id;
	int created = 0;
	off_t *offset;
	unsigned int *counter;
	if((shm_id = get_shm(key_shm,(char**)&offset,sizeof(off_t) + sizeof(unsigned int),&created)) == -1)
	{
		remove_monitor(mon);
		close(fd);
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_init(key_mon: %p, key_shm: %p, file_path: %p) - Cannot get shm",key_mon,key_shm,file_path);
		perror(error_string);
		return NULL;
	}
	counter = (unsigned int *)(offset + 1);

	/*
	 * If i have created the shared memory area for the
	 * offset then I have to initialize it
	 */
	if(created != 0)
	{
		*counter = 0;
		if((*offset = lseek(fd,0,SEEK_END)) == -1)
		{
			remove_monitor(mon);
			close(fd);
			remove_shm(shm_id);
			snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_init(key_mon: %p, key_shm: %p, file_path: %p) - Cannot get current file offset",key_mon,key_shm,file_path);
			perror(error_string);
			return NULL;
		}
	}

	/*
	 * Create the structure for file handling
	 */
	IPC02_File_TypeDef *file = (IPC02_File_TypeDef*)malloc(sizeof(IPC02_File_TypeDef));

	if(lseek(fd,0,SEEK_SET) == -1)
	{
		remove_monitor(mon);
		close(fd);
		remove_shm(shm_id);
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_init(key_mon: %p, key_shm: %p, file_path: %p) - Cannot get 0 offset",key_mon,key_shm,file_path);
		perror(error_string);
		return NULL;
	}
	file->prod_offset = offset;
	file->fd = fd;
	file->mon = mon;
	file->shm_id = shm_id;
	file->counter = counter;

	(*counter)++;

	leave_monitor(mon);

	return file;
}

int IPC02_File_consume(IPC02_File_TypeDef *file, void* buf, size_t nbyte)
{
	char error_string[ERRMSG_MAX_LEN];

	enter_monitor(file->mon);

	/*
	 * Get current offset
	 */
	off_t curr_offset = lseek(file->fd,0,SEEK_CUR);
	if(curr_offset == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_consume(file: %p, buf: %p, nbyte: %zu) - Cannot get current offset",file,buf,nbyte);
		perror(error_string);
		return -1;
	}

	/*
	 * Wait until you can consume produced data
	 */
	while(curr_offset + nbyte > (*file->prod_offset))
	{
		if(wait_cond(file->mon,COND_NEWDATA) == -1)
		{
			snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_consume(file: %p, buf: %p, nbyte: %zu) - Cannot wait for new data",file,buf,nbyte);
			perror(error_string);
			return -1;
		}
	}

	/*
	 * Consume produced data
	 */
	if(read(file->fd,buf,nbyte) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_consume(file: %p, buf: %p, nbyte: %zu) - Error while reading file",file,buf,nbyte);
		perror(error_string);
		return -1;
	}

	leave_monitor(file->mon);
	return 0;
}

int IPC02_File_produce(IPC02_File_TypeDef *file, void* buf, size_t nbyte)
{
	char error_string[ERRMSG_MAX_LEN];

	enter_monitor(file->mon);

	/*
	 * Produce new data
	 */
	ssize_t wrote_data = 0;
	if((wrote_data = pwrite(file->fd,buf,nbyte,*file->prod_offset)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_produce(file: %p, buf: %p, nbyte: %zu) - Error while writing file",file,buf,nbyte);
		perror(error_string);
		return -1;
	}

	*file->prod_offset += wrote_data;

	/*
	 * Signal that new data is avaliable
	 */
	if(broadcast_cond(file->mon,COND_NEWDATA) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"IPC02_File_produce(file: %p, buf: %p, nbyte: %zu) - Error while signaling new data",file,buf,nbyte);
		perror(error_string);
		return -1;
	}

	leave_monitor(file->mon);
	return 0;
}

int IPC02_File_remove(IPC02_File_TypeDef* file)
{
	(*(file->counter))--;
	if(*file->counter == 0) remove_monitor(file->mon);
	remove_shm(file->shm_id);
	close(file->fd);
	free(file);
	return 0;
}

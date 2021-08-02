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

#ifndef _IPC02_FILE_H
#define _IPC02_FILE_H

#include "CommonAssignmentIPC01/libsp.h"
#include <sys/types.h>

struct IPC02_File_Struct
{
	Monitor* mon;
	int fd;
	int shm_id;
	unsigned int *counter;
	off_t *prod_offset;
};

typedef struct IPC02_File_Struct IPC02_File_TypeDef;

IPC02_File_TypeDef* IPC02_File_init(key_t *key_mon, key_t *key_shm, const char* file_path);

int IPC02_File_consume(IPC02_File_TypeDef *file, void* buf, size_t nbyte);

int IPC02_File_produce(IPC02_File_TypeDef *file, void* buf, size_t nbyte);

int IPC02_File_remove(IPC02_File_TypeDef* file);

#endif /* ifndef _IPC02_FILE_H */

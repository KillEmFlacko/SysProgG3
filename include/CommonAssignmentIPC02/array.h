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
 * @file array.h
 * @brief Library header for array handling in producer-consumer problem
 */

#ifndef _H_IPC02ARRAY
#define _H_IPC02ARRAY

#include <sys/types.h>

#define MAX_ARRAY_LEN 256
#define MAX_CONSUMERS 256

/**
 * @brief Array data structure
 */
struct Array_Struct
{
	int shm_id; /**< shared memory ID where this data structure is located */
	int cons_id; /**< next consumer ID */
	int array[MAX_ARRAY_LEN]; /**< actual array */
	int array_len; /**< number of elements used in the array */
	int bitmap[MAX_CONSUMERS][MAX_ARRAY_LEN]; /**< status bitmap of the array */
	int n_consumers; /**< number of consumers using the array */
	int counter; /**< number of processes using the data structure */
};

typedef struct Array_Struct Array_TypeDef;

Array_TypeDef* Array_init(key_t *key,int len, int n_consumers, int *id);

void Array_setDirty(Array_TypeDef* array, int index, int id);

void Array_unsetDirty(Array_TypeDef* array, int index, int id);

int Array_isDirty(Array_TypeDef *array, int index);

int Array_remove(Array_TypeDef* array);

#endif /* ifndef _H_IPC02ARRAY */

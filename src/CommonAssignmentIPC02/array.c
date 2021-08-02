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
 * @file array.c
 * @brief Array library for Producer and Consumer problem.
 * Use with monitor!
 */

#include <stdio.h>
#include <stdlib.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "CommonAssignmentIPC02/array.h"

#define ERRMSG_MAX_LEN 128

/**
 * @brief Create and initialize the array structure
 *
 * @param key pointer to the key used for creating the shared memory area
 * @param len length of the array
 * @param n_consumers number of consumers that will use the array
 * @param id pointer to the memory area where to store consumer id (set to NULL for producer)
 * @retval pointer to the shared Array structure
 */
Array_TypeDef* Array_init(key_t *key,int len, int n_consumers, int *id)
{
	char error_string[ERRMSG_MAX_LEN];

	if(len > MAX_ARRAY_LEN)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key: %p, len: %d, n_consumers: %d) - Array too long",key,len,n_consumers);
		perror(error_string);
		return NULL;
	}

	if(n_consumers > MAX_CONSUMERS)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key: %p, len: %d, n_consumers: %d) - Too much consumers",key,len,n_consumers);
		perror(error_string);
		return NULL;
	}

	int shm_id;
	int created = 0;
	Array_TypeDef *array;

	if((shm_id = get_shm(key,(char**)&array,sizeof(Array_TypeDef),&created)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key: %p, len: %d, n_consumers: %d) - Cannot create shared memory",key,len,n_consumers);
		perror(error_string);
		return NULL;
	}

	/*
	 * If I have created the shared memory I have to initialize
	 * that area
	 */
	if(created)
	{
		array->cons_id = 0;
		array->counter = 0;
		for(int j = 0; j < n_consumers; j++)
			for(int i = 0; i < len; i++)
				array->bitmap[j][i] = 0;
	}

	/*
	 * If i'm a producer i don't need an ID
	 */
	if(id != NULL)
	{
		if(array->cons_id == array->n_consumers)
		{
			remove_shm(shm_id);
			fprintf(stderr,"Array_init(key: %p, len: %d, n_consumers: %d) - Too much consumers\n",key,len,n_consumers);
			return NULL;
		}
		*id = array->cons_id++;
	}

	array->array_len = len;
	array->n_consumers = n_consumers;
	array->shm_id = shm_id;
	array->counter++;

	return array;
}

/**
 * @brief Set an array location as dirty (ready to be consumed) for a consumer
 *
 * @param array array to set the dirty bit to
 * @param index index of the newly produced element
 * @param id consumer id to which the value is new
 */
void Array_setDirty(Array_TypeDef* array, int index, int id)
{
	array->bitmap[id][index] = 1;
}

/**
 * @brief Set an array location as not dirty (already consumed) for a consumer
 *
 * @param array array to unset the dirty bit to
 * @param index index of the consumed element
 * @param id consumer id
 */
void Array_unsetDirty(Array_TypeDef* array, int index, int id)
{
	array->bitmap[id][index] = 0;
}

/**
 * @brief Check if a location is dirty for at least one consumer
 *
 * @param array array to check the bit
 * @param index index of the element
 * @retval 1 if dirty, 0 otherwise
 */
int Array_isDirty(Array_TypeDef *array, int index)
{
	for(int i = 0; i < array->n_consumers; i++)
	{
		if(array->bitmap[i][index] == 1) return 1;
	}
	return 0;
}

/**
 * @brief Remove the array and all the structures attached to it
 *
 * @param array the array to remove
 * @retval 0 if no other process is using the array 
 */
int Array_remove(Array_TypeDef* array)
{
	array->counter--;
	int cnt = array->counter;
	remove_shm(array->shm_id);
	return cnt == 0;
}

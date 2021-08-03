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
#include <sys/types.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "CommonAssignmentIPC02/array.h"

#define NCOND 2
#define COND_NEWRES 0
#define COND_HASCONSUMED 1
#define ERRMSG_MAX_LEN 128

/**
 * @brief Create and initialize the array structure
 *
 * @param key pointer to the key used for creating the shared memory area
 * @param len length of the array
 * @param is_consumer signals if the process behave as a consumer or producer
 * @retval pointer to the shared Array structure
 */
Array_TypeDef* Array_init(key_t *key_mon, key_t *key_shm,int len, int is_consumer)
{
	char error_string[ERRMSG_MAX_LEN];

	/*
	 * Check if array length is valid
	 */
	if(len > MAX_ARRAY_LEN)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key_mon: %p, key_shm: %p, len: %d, is_consumer: %d) - Array too long",key_mon,key_shm,len,is_consumer);
		perror(error_string);
		return NULL;
	}

	/*
	 * get the monitor data structure before working on the array
	 */
	Monitor *mon;
	if((mon = init_monitor(key_mon,NCOND)) == NULL)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key_mon: %p, key_shm: %p, len: %d, is_consumer: %d) - Cannot get monitor",key_mon,key_shm,len,is_consumer);
		perror(error_string);
		return NULL;
	}

	enter_monitor(mon);

	int shm_id;
	int created = 0;
	struct _Array_Bitmap *array;
	if((shm_id = get_shm(key_shm,(char**)&array,sizeof(struct _Array_Bitmap),&created)) == -1)
	{
		snprintf(error_string,ERRMSG_MAX_LEN,"Array_init(key_mon: %p, key_shm: %p, len: %d, is_consumer: %d) - Cannot get shared memory area",key_mon,key_shm,len,is_consumer);
		perror(error_string);
		return NULL;
	}

	/*
	 * If I have created the shared memory I have to initialize
	 * that area
	 */
	if(created)
	{
		array->counter = 0;

		for( int i = 0 ; i < MAX_CONSUMERS; i++)
			array->used_ids[i] = 0;

		array->n_consumers = 0;

		array->array_len = len;
	}

	/*
	 * If I'm a producer I don't need an ID
	 */
	int consumer_id = -1;
	if(is_consumer)
	{
		if(array->n_consumers >= MAX_CONSUMERS)
		{
			remove_shm(shm_id);
			fprintf(stderr,"Array_init(key_mon: %p, key_shm: %p, len: %d, is_consumer: %d) - Too much consumers\n",key_mon,key_shm,len,is_consumer);
			return NULL;
		}

		array->n_consumers++;

		/*
		 * Get the first avaliable consumer ID
		 */
		for(consumer_id = 0; consumer_id < array->n_consumers  && array->used_ids[consumer_id] != 0 ; consumer_id++);

		array->used_ids[consumer_id] = 1;

		/*
		 * Initialize your bitmap
		 */
		for(int i = 0; i < array->array_len; i++)
		{
			array->bitmap[consumer_id][i] = 0;
		}
	}

	/*
	 * Counter of the processes using the array
	 */
	array->counter++;

	Array_TypeDef *data = (Array_TypeDef*)malloc(sizeof(Array_TypeDef));
	data->mon = mon;
	data->array = array;
	data->shm_id = shm_id;
	data->consumer_id = consumer_id;

	leave_monitor(mon);

	return data;
}

/**
 * @brief Check if a location is dirty for at least one consumer
 *
 * @param array array to check the bit
 * @param index index of the element
 * @retval 1 if dirty, 0 otherwise
 */
int _Array_isDirty(Array_TypeDef *array, int index)
{
	for(int i = 0; i < MAX_CONSUMERS; i++)
	{
		/*
		 * If the ID is used and the dirty bit is on then return 1
		 */
		if(array->array->used_ids[i] != 0 &&  array->array->bitmap[i][index] == 1) return 1;
	}
	return 0;
}

/**
 * @brief Consume a resource from the array
 *
 * @param array Array handle data structure
 * @param index index of the resource to be consumed
 * @param value pointer to the consumed value area
 * @retval 0 if all Ok, -1 otherwise
 */
int Array_consume(Array_TypeDef *array, int index, int *value)
{
	enter_monitor(array->mon);

	/*
	 * Check if the calling process is a consumer or a producer
	 */
	if(array->consumer_id == -1)
	{
		fprintf(stderr,"Array_consume(array:%p, index: %d, value: %p) - Cannot consume, producer process\n", array, index, value);
		leave_monitor(array->mon);
		return -1;
	}

	/*
	 * Check id index is valid
	 */
	if(index >= array->array->array_len || index < 0)
	{
		fprintf(stderr,"Array_consume(array:%p, index: %d, value: %p) - Index out of bound\n", array, index, value);
		return -1;
	}

	/*
	 * Wait until value at index is dirty
	 */
	while(array->array->bitmap[array->consumer_id][index] == 0)
	{
		if(wait_cond(array->mon,COND_NEWRES) == -1)
		{
			fprintf(stderr,"Array_consume(array:%p, index: %d, value: %p) - Cannot wait on new data condition\n", array, index, value);
			leave_monitor(array->mon);
			return -1;
		}
	}

	/*
	 * Consume the value
	 */
	*value = array->array->array[index];

	/*
	 * Clear the dirty bit
	 */
	array->array->bitmap[array->consumer_id][index] = 0;


	/*
	 * Signal that a value has been consumed
	 */
	if(broadcast_cond(array->mon,COND_HASCONSUMED) == -1)
	{
		fprintf(stderr,"Array_consume(array:%p, index: %d, value: %p) - Cannot signal on consumed data condition\n", array, index, value);
		leave_monitor(array->mon);
		return -1;
	}

	leave_monitor(array->mon);
	return 0;
}

/**
 * @brief Produce a resource in the array
 *
 * @param array Array handle data structure
 * @param index index of the resource to be produces
 * @param value value to produce
 * @retval 0 if all Ok, -1 otherwise
 */
int Array_produce(Array_TypeDef *array, int index, int value)
{
	enter_monitor(array->mon);

	/*
	 * Check if the calling process is a consumer or a producer
	 */
	if(array->consumer_id != -1)
	{
		fprintf(stderr,"Array_produce(array:%p, index: %d, value: %d) - Cannot produce, consumer process\n", array, index, value);
		leave_monitor(array->mon);
		return -1;
	}

	/*
	 * Check id index is valid
	 */
	if(index >= array->array->array_len || index < 0)
	{
		fprintf(stderr,"Array_produce(array:%p, index: %d, value: %d) - Index out of bound\n", array, index, value);
		leave_monitor(array->mon);
		return -1;
	}

	/*
	 * Wait until value at index is clear
	 */
	while(_Array_isDirty(array,index))
	{
		if(wait_cond(array->mon,COND_HASCONSUMED) == -1)
		{
			fprintf(stderr,"Array_produce(array:%p, index: %d, value: %d) - Cannot wait on consumed data condition\n", array, index, value);
			leave_monitor(array->mon);
			return -1;
		}
	}

	/*
	 * Produce the value
	 */
	array->array->array[index] = value;

	/*
	 * Set all dirty bits
	 */
	for(int i = 0; i < MAX_CONSUMERS; i++)
	{
		if(array->array->used_ids[i] != 0)
			array->array->bitmap[i][index] = 1;
	}


	/*
	 * Signal that a value has been produced
	 */
	if(broadcast_cond(array->mon,COND_NEWRES) == -1)
	{
		fprintf(stderr,"Array_consume(array:%p, index: %d, value: %d) - Cannot signal on new data condition\n", array, index, value);
		leave_monitor(array->mon);
		return -1;
	}

	leave_monitor(array->mon);
	return 0;
}

/**
 * @brief Remove the array and all the structures attached to it
 *
 * @param array the array to remove
 */
void Array_remove(Array_TypeDef* array)
{
	enter_monitor(array->mon);

	array->array->counter--;
	int cnt = array->array->counter;
	if(array->consumer_id >= 0) {
		array->array->used_ids[array->consumer_id] = 0;
		array->array->n_consumers--;
	}

	/*
	 * Signal that a value has been consumed in order
	 * to allow producers to read bitmap
	 */
	if(broadcast_cond(array->mon,COND_HASCONSUMED) == -1)
	{
		fprintf(stderr,"Array_remove(array:%p) - Cannot signal on consumed data condition\n", array);
	}
	leave_monitor(array->mon);

	remove_shm(array->shm_id);
	if(cnt == 0)
		remove_monitor(array->mon);

	free(array);
}

/**
 * @brief Get the array length
 *
 * @param array pointer to Array handle data structure
 * @retval array length
 */
int Array_getLen(Array_TypeDef* array)
{
	enter_monitor(array->mon);
	int len = array->array->array_len;
	leave_monitor(array->mon);
	return len;
}

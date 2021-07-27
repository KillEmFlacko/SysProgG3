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
  @file lamport.c
  @brief Library for Lamport's bakery algorithm required in GroupAssignmentGIPC01
  */

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "GroupAssignmentGIPC01/lamport.h"
#include "lib/lib.h"
#include "lib/queue.h"

/**
 * @brief Initialize the Lamport_TypeDef structure
 *
 * @param key pointer to the key used for creating the undelying IPC structures
 * @param nprocs number of processes involved
 * @param id identifier of the calling process
 * @param lamport pointer to the allocated Lamport_TypeDef structure
 * @retval 0 if all OK, -1 on error
 */
int Lamport_init(key_t *key, int nprocs, int id, Lamport_TypeDef *lamport)
{
	int shm_id;
	int created = 0;
	int *shm_addr;

	if((shm_id = get_shm(key,(char**)&shm_addr,sizeof(int)*nprocs*2,&created)) == -1)
	{
		fprintf(stderr, "Lamport_init(key: %p, nprocs: %d, lamport: %p) - Cannot get shared memory area",key,nprocs,lamport);
		return -1;
	}

	/*
	 * I'm the process that has created the shared memory
	 * so is my responsability to initialize it
	 */
	if(created)
	{
		for(int i = 0; i < nprocs*2 ; i++ )
		{
			shm_addr[i] = 0;
		}
	}

	lamport->id = id;
	lamport->shm_id = shm_id;
	lamport->num_procs = nprocs;
	lamport->entering = shm_addr;
	lamport->number = shm_addr + nprocs;

	return 0;
}

/**
 * @brief Request to enter in the critical section
 *
 * @param lamport Lamport_TypeDef structure used for lock request
 * @retval 0 if all OK, -1 on error
 */
int Lamport_lock(Lamport_TypeDef *lamport)
{
	/*
	 * Signal that you are picking a number
	 */
	lamport->entering[lamport->id] = 1;

	/*
	 * Pick the next number
	 */
	lamport->number[lamport->id] = 1 + array_max(lamport->number,lamport->num_procs,NULL);

	/*
	 * Stop signaling thet you are picking a number
	 */
	lamport->entering[lamport->id] = 0;

	/*
	 * Now we have to find out if we are the next customer
	 * to be served in the queue
	 */
	for (int j = 0; j <= lamport->num_procs ; j++)
	{
		/*
		 * No need to manage sched_yield errors,
		 * that is because if it fails the process
		 * will simply keep executing our while loop
		 */

		/*
		 * First wait while customer j picks its number
		 * (if he's picking one)
		 */
		while (lamport->entering[j]) sched_yield(); // Schedule another thread

		/*
		 * Now go forward if your number is less than his number or your id is
		 * less than his id
		 *
		 * If none of these conditions are true then wait until customer j has
		 * finished his service
		 */
		while(lamport->number[j] != 0 &&   // End of service for customer j
				(lamport->number[j] < lamport->number[lamport->id] || 						// Higher
				 (lamport->number[j] == lamport->number[lamport->id] && j < lamport->id)))	// priority
			sched_yield(); // Schedule another thread
	}

	return 0;
}

/**
 * @brief Exit form critical section
 *
 * @param lamport Lamport_TypeDef structure used for unlocking
 * @retval 0 if all OK, -1 on error
 */
int Lamport_unlock(Lamport_TypeDef *lamport)
{
	/*
	 * Signal end of your service
	 */
	lamport->number[lamport->id] = 0;
	return 0;
}

/**
 * @brief Remove Lamport_TypeDef structure destroying underlying IPC structures
 *
 * @param lamport Lamport_TypeDef structure to be removed
 * @retval 0 if all OK, -1 on error
 */
int Lamport_remove(Lamport_TypeDef *lamport)
{
	remove_shm(lamport->shm_id);
	return 0;
}

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
  @file lamport.h
  @brief Library header for GroupAssignmentGIPC01
  */

#ifndef LAMPORT_H
#define LAMPORT_H

#include <sys/types.h>
#include "lib/queue.h"

struct Lamport_Struct
{
	int msg_qid;
	Queue_TypeDef* proc_queue;
};

typedef struct Lamport_Struct Lamport_TypeDef;

Lamport_TypeDef* Lamport_init(key_t *key);

int Lamport_lock(pid_t pid);

int Lamport_unlock(pid_t pid);

int Lamport_remove(Lamport_TypeDef *lamport);

#endif


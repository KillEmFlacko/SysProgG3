/*
 * Course: System Programming 2020/2021
 *
 * Lecturers:
 * Alessia		Saggese asaggese@unisa.it
 * Francesco	Moscato	fmoscato@unisa.it
 *
 * Group:
 * D'Alessio	Simone
 * Falanga		Armando		0622701140  a.falanga13@studenti.unisa.it
 * Fattore		Alessandro
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libgen.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "CommonAssignmentIPC01/libsp.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define FAILURE_MESSAGE ANSI_COLOR_RED"Test FAILED"ANSI_COLOR_RESET"\n"
#define SUCCESS_MESSAGE ANSI_COLOR_GREEN"Test OK"ANSI_COLOR_RESET"\n"
#define TEST_STR "Ciccio"

int main(int argc, char **argv){
	key_t key = IPC_PRIVATE;

	/*
	 * Test mailbox creation
	 */
	int msg_qid = get_mailbox(&key);
	assert(msg_qid != -1);

#ifdef DEBUG
	fprintf(stderr,"Message queue created\n");
#endif

	/*
	 * Check msgq access
	 */
	int status = 0;
	struct msqid_ds buffer;
	assert((status = msgctl(msg_qid,IPC_STAT,&buffer)) != -1); 
	assert(buffer.msg_qnum == 0);
#ifdef DEBUG
	fprintf(stderr,"Message queue access verified\n");
#endif

	pid_t child;
	Message msg;
	if((child = fork()) != 0)
	{
		/*
		 * PARENT
		 */

		/*
		 * !!!!!!!!! WITHOUT THIS SLEEP SIGCONT COULD BE DELIVERED TOO EARLY !!!!!!!!
		 */
		sleep(1);

		/*
		 * Parent sends a message of type 3
		 */
		msg.type = 3;
		strcpy(msg.data,TEST_STR);
		send_sync(msg_qid,&msg,0);

#ifdef DEBUG
	fprintf(stderr,"PARENT: sent message on queue\n");
#endif

		/*
		 * Check if message have been added in the queue
		 */
		assert((status = msgctl(msg_qid,IPC_STAT,&buffer)) != -1); 
		assert(buffer.msg_qnum == 1);

		/*
		 * Resume child execution
		 */
		kill(child,SIGCONT);

		/*
		 * Wait for a message
		 */
		msg.type = 2;
		strcpy(msg.data,"---");
		receive_sync(msg_qid,&msg,0);

		/*
		 * Last message on queue, check for queue status
		 * and message correctness.
		 */
		assert((status = msgctl(msg_qid,IPC_STAT,&buffer)) != -1); 
		assert(buffer.msg_qnum == 0);
		assert(strcmp(msg.data,TEST_STR) == 0);

		remove_mailbox(msg_qid);

		/*
		 * Check for child return value.
		 */
		int retval;
		wait(&retval);
		if(retval == EXIT_SUCCESS)
		{
			printf(ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "SUCCESS_MESSAGE,basename(argv[0]));
			exit(EXIT_SUCCESS);
		}
		else
		{
			printf(ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "FAILURE_MESSAGE,basename(argv[0]));
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		/*
		 * CHILD
		 */

		/*
		 * Wait for OK from the parent
		 */
		kill(getpid(),SIGSTOP);
		
		/*
		 * Receive message from the parent
		 */
#ifdef DEBUG
	fprintf(stderr,"CHILD: reading from msgqueue\n");
#endif
		msg.type = 3;
		receive_async(msg_qid,&msg,0); // No need to wait

		assert((status = msgctl(msg_qid,IPC_STAT,&buffer)) != -1); 
		assert(buffer.msg_qnum == 0);
		assert(strcmp(msg.data,TEST_STR) == 0);

		msg.type = 2;
		send_sync(msg_qid,&msg,0);

		exit(EXIT_SUCCESS);
	}

	printf(ANSI_COLOR_RESET"["ANSI_COLOR_YELLOW"%s"ANSI_COLOR_RESET"] "SUCCESS_MESSAGE,basename(argv[0]));
	exit(EXIT_SUCCESS);
}


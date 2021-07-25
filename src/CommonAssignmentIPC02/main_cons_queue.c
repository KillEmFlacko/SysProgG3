#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/queue.h"

#define NCOND 2
#define NOTFULL 0
#define NOTEMPTY 1

int id_shared = -1;
Monitor *monitor = NULL;

void exit_procedure(void)
{
	if(id_shared != -1) remove_shm(id_shared);
	if(monitor != NULL) remove_monitor(monitor);
}

int main(int argc, char **argv)
{
	atexit(exit_procedure);

	key_t key_mon = ftok(KEY_FILE,1);

	if((monitor = init_monitor(&key_mon,NCOND)) == NULL)
	{
		fprintf(stderr,"Cannot get monitor\n");
		exit(EXIT_FAILURE);
	}

	key_t key_shm = ftok(KEY_FILE,2);

	/*
	 * Create and attach shared memory area
	 */
	int created;
	Queue_TypeDef* shm_addr;
	if((id_shared = get_shm(&key_shm,(char**)&shm_addr,sizeof(Queue_TypeDef), &created)) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Init queue if this process has created the area
	 */
	if(created == 1)
	{
		enter_monitor(monitor);
		Queue_init(shm_addr);
		leave_monitor(monitor);
	}

	/*
	 * Wait for user input to consume
	 */
	char c;
	int read_val;

	printf("Press a key to consume (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		enter_monitor(monitor);

		/*
		 * Try to remove an element from the queue
		 */
		if(Queue_dequeue(shm_addr,&read_val) == -1)
		{
			if(wait_cond(monitor,NOTEMPTY) == -1)
			{
				fprintf(stderr,"Cannot wait for elements in the queue\n");
				exit(EXIT_FAILURE);
			}
		}

		/*
		 * Print the consumed element
		 */
		printf("Consumed value: %d\n",read_val);

		/*
		 * Signal to producer that queue is not full
		 */
		if(signal_cond(monitor,NOTFULL) == -1)
		{
			fprintf(stderr,"Error while signaling free space into queue\n");
			exit(EXIT_FAILURE);
		}

		leave_monitor(monitor);

		printf("Press a key to consume (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

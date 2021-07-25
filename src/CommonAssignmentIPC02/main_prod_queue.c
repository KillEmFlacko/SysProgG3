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
	 * TODO: Come controlli che non venga inizializzato anche da
	 * altri processi?????????
	 */
	/*
	 * Create and attach shared memory area
	 */
	Queue_TypeDef* shm_addr;
	if((id_shared = get_shm(&key_shm,(char**)&shm_addr,sizeof(Queue_TypeDef))) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Init queue
	 */
	Queue_init(shm_addr);

	/*
	 * Wait for user input to produce
	 */
	char c;
	int read_val;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		enter_monitor(monitor);

		/*
		 * Try to add an element in the queue
		 */
		if(Queue_enqueue(shm_addr,rand()) == -1)
		{
			if(wait_cond(monitor,NOTFULL) == -1)
			{
				fprintf(stderr,"Cannot wait for free space in queue\n");
				exit(EXIT_FAILURE);
			}
		}

		/*
		 * Print the produced element
		 */
		if(Queue_back(shm_addr,&read_val) == -1)
		{
			fprintf(stderr,"Cannot read from queue\n");
			exit(EXIT_FAILURE);
		}

		printf("Produced value: %d\n",read_val);

		/*
		 * Signal to consumer that there is something in the buffer
		 */
		if(signal_cond(monitor,NOTEMPTY) == -1)
		{
			fprintf(stderr,"Error while signaling new element in buffer\n");
			exit(EXIT_FAILURE);
		}

		leave_monitor(monitor);

		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}
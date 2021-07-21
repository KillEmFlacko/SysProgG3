#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"

#define NOTFULL_SEM 1
#define NOTEMPTY_SEM 0

int id_sem = -1, id_shared = -1;

void exit_procedure(void)
{
	remove_shm(id_shared);
	remove_sem(id_sem);
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	key_t key = ftok(KEY_FILE,1);
	int* shm_addr;

	atexit(exit_procedure);

	/*
	 * Create semaphore set with NOTFULL and NOTEMPTY
	 */
	if((id_sem = get_sem(&key,2,0)) == -1)
	{
		fprintf(stderr,"Cannot get semaphore\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Create and attach shared memory area
	 */
	if((id_shared = get_shm(&key,(char**)&shm_addr,sizeof(int))) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	if((c = getchar()) != '\n') putchar('\n');

	*shm_addr = rand();
	printf("Produced value: %d\n",*shm_addr);

	/*
	 * Signal to producer that there is something in the buffer
	 */
	if(signal_sem(id_sem,NOTEMPTY_SEM,0) == -1)
	{
		fprintf(stderr,"Error while signaling full buffer\n");
		exit(EXIT_FAILURE);
	}

	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		/*
		 * Wait for content to be consumed and buffer to be empty
		 */
		if(wait_sem(id_sem,NOTFULL_SEM,0) == -1)
		{
			fprintf(stderr,"Error while waiting empty buffer\n");
			exit(EXIT_FAILURE);
		}

		*shm_addr = rand();
		printf("Produced value: %d\n",*shm_addr);

		/*
		 * Signal to consumer that there is something in the buffer
		 */
		if(signal_sem(id_sem,NOTEMPTY_SEM,0) == -1)
		{
			fprintf(stderr,"Error while signaling full buffer\n");
			exit(EXIT_FAILURE);
		}
		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

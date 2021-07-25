#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CommonAssignmentIPC01/libsp.h"

int id_sem_notfull = -1, id_sem_notempty = -1, id_shared = -1;

void exit_procedure(void)
{
	if(id_shared != -1) remove_shm(id_shared);
	if(id_sem_notfull != -1) remove_sem(id_sem_notfull);
	if(id_sem_notempty != -1) remove_sem(id_sem_notempty);
}

/*
 * One producer and one consumer, one variable in shared memory
 */
int main(int argc, char **argv)
{
	atexit(exit_procedure);

	if(argc < 2)
	{
		fprintf(stderr,"Usage:\t%s num_res\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int nres = atoi(argv[1]);
	
	key_t key_shm = ftok(KEY_FILE,1);
	key_t key_notfull = ftok(KEY_FILE,2);
	key_t key_notempty = ftok(KEY_FILE,3);

	/*
	 * Create semaphore set for NOTFULL
	 */
	if((id_sem_notfull = get_sem(&key_notfull,nres,1)) == -1)
	{
		fprintf(stderr,"Cannot get notfull semaphore\n");
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Create semaphore set for NOTEMPTY
	 */
	if((id_sem_notempty = get_sem(&key_notempty,nres,0)) == -1)
	{
		fprintf(stderr,"Cannot get notempty semaphore\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Create and attach shared memory area
	 */
	int* shm_addr;
	if((id_shared = get_shm(&key_shm,(char**)&shm_addr,sizeof(int)*nres,NULL)) == -1)
	{
		fprintf(stderr,"Cannot get shared memory area\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * Wait for user input to produce
	 */
	char c;
	int res;
	srand(time(NULL));
	printf("Press a key to produce (Ctrl-D to exit)");
	while((c = getchar()) != EOF)
	{
		if(c != '\n') putchar('\n');

		printf("Enter resource to use:");
		scanf("%d",&res);
		
		/*
		 * Wait for content to be consumed and buffer to be empty
		 */
		if(wait_sem(id_sem_notfull,res,0) == -1)
		{
			fprintf(stderr,"Error while waiting empty buffer\n");
			exit(EXIT_FAILURE);
		}

		shm_addr[res] = rand();
		printf("Produced value: %d\n",shm_addr[res]);

		/*
		 * Signal to consumer that there is something in the buffer
		 */
		if(signal_sem(id_sem_notempty,res,0) == -1)
		{
			fprintf(stderr,"Error while signaling full buffer\n");
			exit(EXIT_FAILURE);
		}
		printf("Press a key to produce (Ctrl-D to exit)");
	}

	exit(EXIT_SUCCESS);
}

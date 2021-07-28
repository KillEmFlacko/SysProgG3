#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/lib.h"

#define NCOND 3
#define S_WRITE 0
#define S_NUM_READERS 1
#define S_QUEUE 2

void writer(int sem, int *val, int new_val)
{
    printf("\n### INIZIO PROCESSO SCRITTURA ###\n");

    printf("Mi metto in coda per la scrittura\n");
    wait_sem(sem, S_QUEUE, 0);

    printf("Giunto il mio turno richiedo accesso esclusivo alla risorsa\n");
    wait_sem(sem, S_WRITE, 0);

    printf("Esco dalla coda di attesa\n");
    signal_sem(sem, S_QUEUE, 0);

    printf("<< CRITICAL SECTION >>\n*\n*\n");
    // ***
    sleep(12);
    // ***
    *val = new_val;
    printf("\n*\n*\n<< EXIT SECTION >>\n");

    signal_sem(sem, S_WRITE, 0);
    
    printf("### FINE PROCESSO SCRITTURA ###\n\n");
}

void reader(int sem, int *nr, int *val)
{
    printf("\n### INIZIO PROCESSO LETTURA ###\n");

    printf("Mi metto in coda per la lettura\n");
    wait_sem(sem, S_QUEUE, 0);

    printf("Chiedo l'accesso esclusivo al contatore dei lettori (+)\n");
    wait_sem(sem, S_NUM_READERS, 0);
    ++(*nr);
    printf("Numero lettori: %d\n", *nr);
    if (*nr == 1)
    {
        printf("Sono il primo lettore\n");
        wait_sem(sem, S_WRITE, 0);
    }
    printf("Esco dalla coda dei processi in attesa\n");
    signal_sem(sem, S_QUEUE, 0);
    printf("Rilascio l'accesso al contatore dei lettori\n");
    signal_sem(sem, S_NUM_READERS, 0);

    printf("<< CRITICAL SECTION >>\n");

    printf("VALUE: %d\n", *val);

    printf("<< EXIT SECTION >>\n");

    printf("Chiedo l'accesso esclusivo al contatore dei lettori (-)\n");
    wait_sem(sem, S_NUM_READERS, 0);
    --(*nr);
    if (*nr == 0)
    {
        printf("Sono l'ultimo lettore che rilascia la risorsa\n");
        signal_sem(sem, S_WRITE, 0);
    }   
    printf("Rilascio l'accesso al contatore dei lettori\n");
    signal_sem(sem, S_NUM_READERS, 0);

    printf("### FINE PROCESSO LETTURA ###\n\n");
}

int main(int argc, char **argv)
{
	key_t key0, key1, keysem;
    int sem;

    /************************************
    *   SHARED MEMORY AREA
    */
	int *value = 0;
    int *n_readers = 0;
    /*
    ************************************
    */

    fprintf(stdout,"Generating keys...\n");

    // Generating key for shared memory
	if ((key0 = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }
    if ((key1 = ftok(".", 101)) == -1) { perror("ftok"); exit(1); }
    if ((keysem = ftok(".", 104)) == -1) { perror("ftok"); exit(1); }

    fprintf(stdout,"Initializing shm...\n");

    // Attach shared memory to data
    get_shm(&key0, (char**)&value, sizeof(int), NULL);
    get_shm(&key1, (char**)&n_readers, sizeof(int), NULL);

    // Reset value in case of existing shm
    *value = 0;
    *n_readers = 0;

    fprintf(stdout,"Initializing set of semaphores...\n");

    // Initializing a set of 3 semaphores with value 1
    sem = get_sem(&keysem, NCOND, 1);
    printf("S_WRITE: %d\n",semctl(sem, S_WRITE, GETVAL));
    printf("S_NUM_READERS: %d\n",semctl(sem, S_NUM_READERS, GETVAL));
    printf("S_QUEUE: %d\n",semctl(sem, S_QUEUE, GETVAL));

    sleep(2);

    int num = 0;

    if (fork() == 0)
    {
        fprintf(stdout,"SONS START\n");

        if (fork() == 0)
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, value);
            }
        }
        else
        {
            while (1) 
            {
                sleep(1);
                reader(sem, n_readers, value);
            }
        }
        
    }
    else
    {
        fprintf(stdout,"FATHER STARTS\n");
        while (1) 
        {
            num++;
            sleep(3);
            writer(sem, value, num);
        }
    }
    
    return 0;
}

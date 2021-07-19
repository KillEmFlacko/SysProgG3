#include <stdio.h>
#include <stdlib.h>
#include "lib/lib.h"
#include "CommonAssignmentIPC01/libsp.h"

#define NCOND 3
#define S_WRITE 0
#define S_NUM_WRITERS 1
#define S_NUM_READERS 2

void write(Monitor *mon, int *val, int *nr, int *nw, int new_val)
{
    enter_monitor(mon);

    // Mi metto in coda per la scrittura
    wait_cond(mon, S_NUM_WRITERS);
    *nw++;
    // Verifico quanti processi lettori ci sono
    wait_cond(mon, S_NUM_READERS);
    // Se ci sono aspetto in coda
    if (*nr > 0)
    {
        wait_cond(mon, S_WRITE);
    }
    signal_cond(mon, S_NUM_READERS);
    // Se non ci sono scrivo
    wait_cond(mon, S_WRITE);
    // Affinchè scriva non devono esserci neanche altri scrittori
    *val = new_val;
    *nw--;
    signal_cond(mon, S_WRITE);
    
    leave_monitor(mon);
}

void read(Monitor *mon, int *val, int *nr, int *nw)
{
    enter_monitor(mon);

    // Mi metto in coda per la lettura
    wait_cond(mon, S_NUM_READERS);
    *nr++;
    // Verifico quanti processi scrittori ci sono
    wait_cond(mon, S_NUM_WRITERS);
    // Se ci sono aspetto in coda
    if (*nw > 0)
    {
        wait_cond(mon, S_WRITE);
    }
    signal_cond(mon, S_NUM_WRITERS);
    // Se non ci sono stampo
    printf("VALUE: %d", *val);
    *nr--;
    // L'ultimo lettore consente allo scrittore di procedere
    if (*nr == 0)
    {
        signal_cond(mon, S_WRITE);
    }
    signal_cond(mon, S_NUM_READERS);

    leave_monitor(mon);
}

int main(int argc, char **argv)
{
	key_t key;
    Monitor *mon;
	int shmid;
	int value = 0;
    int n_writers = 0;
    int n_readers = 0;

    // Generating key for shared memory
	if ((key = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }

    // Obtaining shared memory ID
    shmid = get_shm(&key, &value, sizeof(int));

    // Initializing a monitor with 4 condition variable
    /*
    *   
    */
    mon = init_monitor(NCOND);
    
}
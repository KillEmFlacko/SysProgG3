#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libsp.h"

#define NCOND 4
#define S_WRITE 0
#define S_NUM_WRITERS 1
#define S_NUM_READERS 2
#define S_BATCH_READERS 3

#define BUFFER_SIZE 100

typedef struct {
    int batch[BUFFER_SIZE];
    int front;
    int back;
    int num_elements;
} BatchReaders;

BatchReaders* initBatchReaders()
{
    BatchReaders* b = (BatchReaders*)malloc(sizeof(BatchReaders));
    b -> front = b -> back = 0;
    b -> num_elements = 0;
    return b;
}

int push(BatchReaders* b, int val)
{
    if((++(b -> num_elements)) == BUFFER_SIZE) return -1;
    b -> batch [b -> front] = val;
    b -> front = (b -> front + 1) % BUFFER_SIZE;
    return 0;
}

int pop(BatchReaders* b)
{
    if (b -> num_elements == 0) return 0;
    int returnValue = b -> batch [b -> back];
    b -> back = (b -> back + 1) % BUFFER_SIZE;
    return returnValue;
}

void writer(Monitor *mon, BatchReaders *b, int *val, int *nr, int *nw, int new_val)
{

    //enter_monitor(mon);
    printf("### INIZIO PROCESSO SCRITTURA ###\n");

    printf("Mi metto in coda per la scrittura\n");
    wait_sem(mon -> id_cond, S_NUM_WRITERS, 0);
    (*nw)++;
    printf("n_writers: %d\n", *nw);
    signal_sem(mon -> id_cond, S_NUM_WRITERS, 0);
    
    wait_sem(mon -> id_cond, S_NUM_READERS, 0);
    printf("Salvo il blocco di lettori precedenti in attesa che è %d\n",*nr);
    push(b, *nr);
    printf("Azzero la coda di lettori in attesa\n");
    *nr = 0;
    signal_sem(mon -> id_cond, S_NUM_READERS, 0);
    printf("Il prossimo lettore ad entrare si vedrà come primo\n");

    // Tento la scrittura
    wait_sem(mon -> id_cond, S_WRITE, 0);
    // Affinchè scriva non devono esserci neanche altri scrittori
    printf("Effettuo la scrittura\n");
    // ***
    sleep(12);
    // ***
    *val = new_val;
    signal_sem(mon -> id_cond, S_WRITE, 0);
    

    printf("Decremento il numero di scrittori\n");
    wait_sem(mon -> id_cond, S_NUM_WRITERS, 0);
    (*nw)--;
    signal_sem(mon -> id_cond, S_NUM_WRITERS, 0);
    
    //leave_monitor(mon);
    printf("### FINE PROCESSO SCRITTURA ###\n\n");
}

void reader(Monitor *mon, BatchReaders *b, int *val, int *nr, int *nw)
{
    int batch, n_readers, n_writers;

    //enter_monitor(mon);
    printf("### INIZIO PROCESSO LETTURA ###\n");

    printf("Mi metto in coda per la lettura\n");
    wait_sem(mon -> id_cond, S_NUM_READERS, 0);
    n_readers = ++(*nr);
    printf("n_readers: %d\n", n_readers);
    signal_sem(mon -> id_cond, S_NUM_READERS, 0);


    printf("Se ci sono scrittori aspetto in coda...\n");
    // Il primo lettore prende il lock della scrittura
    if (n_readers == 1)
    {
        printf("Sono il primo lettore\n");
        wait_sem(mon -> id_cond, S_WRITE, 0);

        // Appena prendo il lock di scrittura (quindi blocco altri scrittori)
        // vedo quanti lettori ho accodati da sbloccare

        printf("Verifico quanti processi scrittori ci sono\n");
        wait_sem(mon -> id_cond, S_NUM_WRITERS, 0);
        n_writers = *nw;
        printf("n_writers: %d\n", n_writers);
        signal_sem(mon -> id_cond, S_NUM_WRITERS, 0);

        if (n_writers > 0)
        {
            // Se ci sono processi scrittori il primo ha definito un batch
            batch = pop(b);
        }
        else
        {
            // Non ci sono processi scrittori quindi sblocco tutti i lettori escluso me
            wait_sem(mon -> id_cond, S_NUM_READERS, 0);
            batch = *nr - 1;
            printf("n_readers: %d\n", n_readers);
            signal_sem(mon -> id_cond, S_NUM_READERS, 0);
        }
        
        printf("Numero processi accodati: %d\n",batch);

        // Quando il primo lettore legge deve sbloccare tutti gli altri lettori
        // accodati dietro di lui
        for (int i=0; i<batch; i++)
        {
            signal_sem(mon -> id_cond, S_BATCH_READERS, 0);
        }
        
    }
    else
    {
        printf("Sono un lettore che si accoda\n");
        wait_sem(mon -> id_cond, S_BATCH_READERS, 0);
        printf("Sono stato sbloccato da un lettore\n");
    }
    
    // Una volta sbloccato il primo lettore anche quelli dietro di esso devono procedere
    
    // Se non ci sono stampo
    printf("VALUE: %d\n", *val);

    wait_sem(mon -> id_cond, S_NUM_READERS, 0);
    // L'ultimo lettore consente allo scrittore di procedere
    if (--(*nr) == 0)
    {
        signal_sem(mon -> id_cond, S_WRITE, 0);
        printf("Sono l'ultimo lettore del gruppo che rilascia il semaforo per la scrittura\n");
    }
    signal_sem(mon -> id_cond, S_NUM_READERS, 0);

    //leave_monitor(mon);
    printf("### FINE PROCESSO LETTURA ###\n\n");
}

int main(int argc, char **argv)
{
	key_t key0, key1, key2, key3;
    Monitor *mon;

    /************************************
    *   DA CONDIVIDERE CON SHM
    */
	int *value = 0;
    int *n_writers = 0;
    int *n_readers = 0;
    BatchReaders* b = initBatchReaders();
    /*
    ************************************
    */

    fprintf(stdout,"Generating keys...\n");

    // Generating key for shared memory
	if ((key0 = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }
    if ((key1 = ftok(".", 101)) == -1) { perror("ftok"); exit(1); }
    if ((key2 = ftok(".", 102)) == -1) { perror("ftok"); exit(1); }
    if ((key3 = ftok(".", 103)) == -1) { perror("ftok"); exit(1); }

    fprintf(stdout,"Initializing shm...\n");

    // Attach shared memory to data
    get_shm(&key0, &value, sizeof(int));
    get_shm(&key1, &n_writers, sizeof(int));
    get_shm(&key2, &n_readers, sizeof(int));
    printf("Adesso provo a creare la coda in shm...\n");
    get_shm(&key3, b, sizeof(BatchReaders));

    // Reset value in case of existing shm
    *value = 0;
    *n_writers = 0;
    *n_readers = 0;

    fprintf(stdout,"Initializing monitor...\n");

    // Initializing a monitor with 4 condition variable
    mon = init_monitor(NCOND);
    signal_sem(mon -> id_cond, S_WRITE, 0);
    signal_sem(mon -> id_cond, S_NUM_READERS, 0);
    signal_sem(mon -> id_cond, S_NUM_WRITERS, 0);
    printf("S_WRITE: %d\n",semctl(mon -> id_cond, S_WRITE, GETVAL));
    printf("S_NUM_WRITERS: %d\n",semctl(mon -> id_cond, S_NUM_WRITERS, GETVAL));
    printf("S_NUM_READERS: %d\n",semctl(mon -> id_cond, S_NUM_READERS, GETVAL));

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
                reader(mon, b, value, n_readers, n_writers);
            }
        }
        else
        {
            while (1) 
            {
                sleep(1);
                reader(mon, b, value, n_readers, n_writers);
            }
        }
        
    }
    else
    {
        fprintf(stdout,"FATHER START\n");
        while (1) 
        {
            num++;
            sleep(3);
            writer(mon, b, value, n_readers, n_writers, num);
        }
    }
    
    return 0;
}

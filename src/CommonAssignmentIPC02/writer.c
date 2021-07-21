#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/lib.h"
#include "CommonAssignmentIPC01/libsp.h"

#define NCOND 4
#define S_WRITE 0
#define S_NUM_WRITERS 1
#define S_NUM_READERS 2
#define S_BATCH_READERS 3

// A linked list (LL) node to store a queue entry
struct QNode {
    int key;
    struct QNode* next;
};
  
// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
    struct QNode *front, *rear;
};
  
// A utility function to create a new linked list node.
struct QNode* newNode(int k)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}
  
// A utility function to create an empty queue
struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
  
// The function to add a key k to q
void enQueue(struct Queue* q, int k)
{
    // Create a new LL node
    struct QNode* temp = newNode(k);
  
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
  
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}
  
// Function to remove a key from given queue q
int deQueue(struct Queue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;
  
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;

    int key = temp->key;
  
    q->front = q->front->next;
  
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
  
    free(temp);

    return key;
}

void write(Monitor *mon, struct Queue *q, int *val, int *nr, int *nw, int new_val)
{
    int n_readers, n_writers;

    enter_monitor(mon);

    // Mi metto in coda per la scrittura
    wait_cond(mon, S_NUM_WRITERS);
    *nw++;
    signal_cond(mon, S_NUM_WRITERS);
    
    // Azzero la coda di lettori in attesa
    wait_cond(mon, S_NUM_READERS);
    // Salvo il blocco di lettori precedenti in attesa
    enQueue(q, *nr);
    *nr = 0;
    signal_cond(mon, S_NUM_READERS);
    // Il prossimo lettore ad entrare si vedrà come primo

    // Tento la scrittura
    wait_cond(mon, S_WRITE);
    // Affinchè scriva non devono esserci neanche altri scrittori
    *val = new_val;
    signal_cond(mon, S_WRITE);
    

    // Decremento il numero di scrittori
    wait_cond(mon, S_NUM_WRITERS);
    *nw--;
    signal_cond(mon, S_NUM_WRITERS);
    
    leave_monitor(mon);
}

void read(Monitor *mon, struct Queue *q, int *val, int *nr, int *nw)
{
    int n_readers, n_writers;

    enter_monitor(mon);

    // Mi metto in coda per la lettura
    wait_cond(mon, S_NUM_READERS);
    n_readers = *nr++;
    signal_cond(mon, S_NUM_READERS);

    // Verifico quanti processi scrittori ci sono
    wait_cond(mon, S_NUM_WRITERS);
    n_writers = *nw;
    signal_cond(mon, S_NUM_WRITERS);

    // Se ci sono aspetto in coda
    if (n_writers > 0)
    {
        // Il primo lettore prende il lock della scrittura e crea un semaforo a cui
        // si accodano gli altri lettori che vengono dopo di lui
        if (n_readers == 1)
        {
            
            wait_cond(mon, S_WRITE);

            // Appena prendo il lock di scrittura (quindi blocco altri scrittori)
            // vedo quanti lettori ho accodati da sbloccare
            int batch = deQueue(q);

            // Quando il primo lettore legge deve sbloccare tutti gli altri lettori
            // accodati dietro di lui
            for (int i=0; i<batch; i++)
            {
                signal_cond(mon, S_BATCH_READERS);
            }
            
        }
        else
        {
            wait_cond(mon, S_BATCH_READERS);
        }
        
        // Una volta sbloccato il primo lettore anche quelli dietro di esso devono procedere
    }
    
    // Se non ci sono stampo
    printf("VALUE: %d", *val);

    wait_cond(mon, S_NUM_READERS);
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
	key_t key0, key1, key2, key3;
    Monitor *mon;

    /************************************
    *   DA CONDIVIDERE CON SHM
    */
	int value = 0;
    int n_writers = 0;
    int n_readers = 0;
    struct Queue* q = createQueue();
    /*
    ************************************
    */

#ifdef DEBUG
    fpritnf(stderr,"Generating keys...\n");
#endif

    // Generating key for shared memory
	if ((key0 = ftok(".", 100)) == -1) { perror("ftok"); exit(1); }
    if ((key1 = ftok(".", 101)) == -1) { perror("ftok"); exit(1); }
    if ((key2 = ftok(".", 102)) == -1) { perror("ftok"); exit(1); }
    if ((key3 = ftok(".", 103)) == -1) { perror("ftok"); exit(1); }

#ifdef DEBUG
    fpritnf(stderr,"Initializing shm...\n");
#endif

    // Attach shared memory to data
    get_shm(&key0, &value, sizeof(int));
    get_shm(&key1, &n_writers, sizeof(int));
    get_shm(&key2, &n_readers, sizeof(int));
    get_shm(&key3, q, sizeof(struct Queue));

#ifdef DEBUG
    fpritnf(stderr,"Initializing monitor...\n");
#endif

    // Initializing a monitor with 4 condition variable
    mon = init_monitor(NCOND);

    int num = 0;

    if (fork() == 0)
    {
#ifdef DEBUG
        fprintf(stderr,"SONS START\n");
#endif
        read(mon, q, &value, &n_readers, &n_writers);
        sleep(4);
        read(mon, q, &value, &n_readers, &n_writers);
    }
    else
    {
#ifdef DEBUG
        fprintf(stderr,"FATHER START\n");
#endif
        num++;
        sleep(1);
        write(mon, q, &value, &n_readers, &n_writers, num);
    }
    
}
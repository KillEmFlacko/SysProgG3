#include <stdio.h>
#include <stdlib.h>
#include "lib/lib.h"
#include "CommonAssignmentIPC01/libsp.h"

#define NCOND 3
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
        return;
  
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

    signal_wait(mon, S_NUM_READERS);
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

    struct Queue* q = createQueue();

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
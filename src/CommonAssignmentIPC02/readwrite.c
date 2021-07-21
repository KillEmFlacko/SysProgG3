#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CommonAssignmentIPC01/libsp.h"
#include "lib/lib.h"

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

void writer(Monitor *mon, struct Queue *q, int *val, int *nr, int *nw, int new_val)
{
    int n_readers, n_writers;

    enter_monitor(mon);
    printf("### INIZIO PROCESSO SCRITTURA ###\n");

    printf("Mi metto in coda per la scrittura\n");
    wait_cond(mon, S_NUM_WRITERS);
    (*nw)++;
    printf("n_writers: %d\n", *nw);
    signal_cond(mon, S_NUM_WRITERS);
    
    printf("Azzero la coda di lettori in attesa\n");
    wait_cond(mon, S_NUM_READERS);
    printf("Salvo il blocco di lettori precedenti in attesa\n");
    enQueue(q, *nr);
    *nr = 0;
    signal_cond(mon, S_NUM_READERS);
    printf("Il prossimo lettore ad entrare si vedrà come primo\n");

    // Tento la scrittura
    wait_cond(mon, S_WRITE);
    // Affinchè scriva non devono esserci neanche altri scrittori
    printf("Effettuo la scrittura\n");
    *val = new_val;
    signal_cond(mon, S_WRITE);
    

    printf("Decremento il numero di scrittori\n");
    wait_cond(mon, S_NUM_WRITERS);
    (*nw)--;
    signal_cond(mon, S_NUM_WRITERS);
    
    leave_monitor(mon);
    printf("### FINE PROCESSO SCRITTURA ###\n\n");
}

void reader(Monitor *mon, struct Queue *q, int *val, int *nr, int *nw)
{
    int n_readers, n_writers;

    enter_monitor(mon);
    printf("### INIZIO PROCESSO LETTURA ###\n");

    printf("Mi metto in coda per la lettura\n");
    wait_cond(mon, S_NUM_READERS);
    n_readers = *nr++;
    printf("n_readers: %d\n", n_readers);
    signal_cond(mon, S_NUM_READERS);
 
    printf("Verifico quanti processi scrittori ci sono\n");
    wait_cond(mon, S_NUM_WRITERS);
    n_writers = *nw;
    printf("n_writers: %d\n", n_writers);
    signal_cond(mon, S_NUM_WRITERS);
    
    if (n_writers > 0)
    {
        printf("Se ci sono aspetto in coda\n");
        // Il primo lettore prende il lock della scrittura
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
    printf("VALUE: %d\n", *val);

    wait_cond(mon, S_NUM_READERS);
    *nr--;
    // L'ultimo lettore consente allo scrittore di procedere
    if (*nr == 0)
    {
        signal_cond(mon, S_WRITE);
    }
    signal_cond(mon, S_NUM_READERS);

    leave_monitor(mon);
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
    struct Queue* q = createQueue();
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
    get_shm(&key3, q, sizeof(struct Queue));

    // Reset value in case of existing shm
    *value = 0;
    *n_writers = 0;
    *n_readers = 0;

    fprintf(stdout,"Initializing monitor...\n");

    // Initializing a monitor with 4 condition variable
    mon = init_monitor(NCOND);
    signal_cond(mon, S_WRITE);
    signal_cond(mon, S_NUM_WRITERS);
    signal_cond(mon, S_NUM_READERS);
    printf("S_WRITE: %d\n",semctl(mon -> id_cond, S_WRITE, GETVAL));
    printf("S_NUM_WRITERS: %d\n",semctl(mon -> id_cond, S_NUM_WRITERS, GETVAL));
    printf("S_NUM_READERS: %d\n",semctl(mon -> id_cond, S_NUM_READERS, GETVAL));

    sleep(2);

    int num = 0;

    if (fork() == 0)
    {
        sleep(20);
        fprintf(stdout,"SONS START\n");
        sleep(1);
        reader(mon, q, value, n_readers, n_writers);
        sleep(10);
        reader(mon, q, value, n_readers, n_writers);
        return 0;
    }
    else
    {
        num++;
        sleep(5);
        fprintf(stdout,"FATHER START\n");
        writer(mon, q, value, n_readers, n_writers, num);
    }
    
    return 0;
}

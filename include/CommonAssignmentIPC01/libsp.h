#define MAX_MSGQUEUE_LEN 128

/* Message struct  :
	- type: Represents the type of the message
	- data: Contains the text of the message
*/

typedef struct {
	long type;
	char data[MAX_MSGQUEUE_LEN];	
} Message;

/* Request a shared memory */
int get_shm (key_t *chiave, char **ptr_shared, int dim);
/* Request a semaphore */
int get_sem (key_t *chiave_sem, int numsem, int initsem);
/* wait on the semaphore */
void wait_sem (int id_sem, int numsem, int flag);
/* signal on the semaphore */
void signal_sem (int id_sem, int numsem, int flag);
/* remove a shared memory */
void remove_shm (int id_shared);
/* remove a shared memory */
void remove_sem (int id_sem);
/* async send on a message queue*/
void send_asyn(int msg_qid, Message *PTR_mess, int send_flag);
/* async send on a message queue*/
void send_sync(int msg_qid, Message *messaggio, int flag) ;
/*async receive on a message queue*/
void receive_async (int msg_qid, Message *PTR_mess, int receive_flag);
/*sync send on a message queue*/
void receive_sync(int msg_qid, Message *messaggio, int flag) ;
/* remove a mailbox */
void remove_mailbox(int msg_qid);




 /* Monitor struct  :
	MUTEX ( sem )
	NUM_VAR_CONDITION (int)
	ID VAR CONDITION Semaphore (sem)
(in this version all conditions are associated to the same semaphore */


typedef struct {
	int id_mutex;	
	int numcond ;	
	int id_cond ;	
	} Monitor;


Monitor *init_monitor(int ncond);	/*init  monitor :
					inputs : numcond  to init;
					outputs: Monitor *: */


/*Routine enter_monitor .
	inputs : Monitor * mon ;
	waits on mutex*/

void enter_monitor(Monitor *mon);

/*Routine leave_monitor  :
	inputs: Monitor *mon ;
	mutex.signal()*/

void leave_monitor(Monitor *mon);

/* Routine wait_cond  / signal_cond:.
	inputs : Monitor *mon :  ;
                 cond_num : index of the condition var */

void wait_cond(Monitor *mon,int cond_num);

void signal_cond(Monitor *mon,int cond_num);

/*Routine remove_monitor */
void remove_monitor(Monitor *mon);

/*Routine IS_queue_empty : returns 1 if the condition variable queue is empty, 0 otherwise*/
/*inputs : Monitor *mon : 
	   cond_num : number of condition variable*/
int IS_queue_empty(Monitor *mon,int cond_num);

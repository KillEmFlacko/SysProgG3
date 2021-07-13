#include <stdio.h>
#include <stdlib.h>
#include "CommonAssignmentIPC01/libsp.h"

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

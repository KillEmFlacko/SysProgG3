/*! \file lib.c
 * \brief Template library file.
 *	
 *	This is a template code file for a static library.
 */

#include "lib/queue.h"

void Queue_init(Queue_TypeDef *queue)
{
	queue->front = 0;
	queue->back = 0;
	queue->len = 0;
}

int Queue_enqueue(Queue_TypeDef *queue, int value)
{
	if(Queue_isFull(queue)) return -1;
	queue->array[queue->back] = value;
	queue->back = (queue->back + 1) % MAXLEN_QUEUE;
	queue->len += 1;
	return 0;
}

int Queue_dequeue(Queue_TypeDef *queue, int *value)
{
	if(Queue_isEmpty(queue)) return -1;
	*value = queue->array[queue->front];
	queue->front = (queue->front + 1) % MAXLEN_QUEUE;
	queue->len -= 1;
	return 0;
}

void Queue_remove(Queue_TypeDef *queue)
{
	return;
}

int Queue_isEmpty(Queue_TypeDef *queue)
{
	return queue->len == 0;
}

int Queue_isFull(Queue_TypeDef *queue)
{
	return queue->len == MAXLEN_QUEUE;
}

int Queue_front(Queue_TypeDef *queue, int *value)
{
	if(Queue_isEmpty(queue)) return -1;
	*value = queue->array[queue->front];
	return 0;
}

int Queue_back(Queue_TypeDef *queue, int *value)
{
	if(Queue_isEmpty(queue)) return -1;
	*value = queue->array[queue->back];
	return 0;
}

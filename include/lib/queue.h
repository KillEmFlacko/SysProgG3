#ifndef QUEUE_H
#define QUEUE_H

#define MAXLEN_QUEUE 256

struct Queue_Struct {
	int len;
	int front;
	int back;
	int array[MAXLEN_QUEUE];
};

typedef struct Queue_Struct Queue_TypeDef;

void Queue_init(Queue_TypeDef *queue);

int Queue_enqueue(Queue_TypeDef *queue, int value);

int Queue_dequeue(Queue_TypeDef *queue, int *value);

void Queue_remove(Queue_TypeDef *queue);

int Queue_isEmpty(Queue_TypeDef *queue);

int Queue_isFull(Queue_TypeDef *queue);

int Queue_front(Queue_TypeDef *queue, int *value);

int Queue_back(Queue_TypeDef *queue, int *value);

#endif


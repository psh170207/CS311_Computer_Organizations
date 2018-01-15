#ifndef _LINKED_QUEUE_H_
#define _LINKED_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct QueueNode{
	char valid;
	char dirty;
	char way;
    
	int tag;
	int address;
	
	struct QueueNode *pre;
	struct QueueNode *post;
	
	unsigned int block_data[2];
}Node_q;

typedef struct Queue{
	int count;
	Node_q* first;
	Node_q* last;
}Queue;

Queue *create_queue();
Node_q *dequeue(Queue *queue);
void enqueue(Queue *queue, Node_q* new_node);
Node_q *find_node(Queue* queue, int tag);
void reconstruct_queue(Queue *queue, int tag);
Node_q *find_way(Queue* queue, int way);
#endif

#include <stdio.h>
#include <stdlib.h>
#include "linked_queue.h"

Queue *create_queue(){

	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->first = NULL;
	queue->last = NULL;
	queue->count = 0;
	return queue;
}

Node_q *dequeue(Queue *queue){
	if(queue->count == 0){
		return NULL;
	}
	else{
		Node_q* evict = queue->last;
		(evict->pre)->post = NULL;
		queue->last = evict->pre;
		(queue->count)--;
		return evict;
	}
}

void enqueue(Queue *queue, Node_q* new_node){
	if(queue->count == 0){
		queue->first = new_node;
		queue->last = new_node;

		new_node->pre = NULL;//

		new_node->post = NULL;
		(queue->count)++;
	}	
	else{
		Node_q* prefirst = queue->first;
		new_node->post = prefirst;
		new_node->pre = prefirst->pre;
		prefirst->pre = new_node;
		queue->first = new_node;
		(queue->count)++;
	}
}


Node_q *find_node(Queue* queue, int tag){
	if((queue->count) == 0) return NULL;
	else{
		Node_q *temp = queue->first;
        while(temp != NULL){
			if(temp->valid && (temp->tag) == tag)	return	temp;
			else{
                temp = temp->post;
            }
		}		
	}
    return NULL;
}

Node_q *find_way(Queue* queue, int way){
	if((queue->count) == 0) return NULL;
	else{
		Node_q *temp = queue->first;
        while(temp != NULL){
			if((temp->way) == way)	return	temp;
			else{
                temp = temp->post;
            }
		}		
	}
    return NULL;
}

void reconstruct_queue(Queue *queue, int tag){

	if((queue->count) == 0 || (queue->count) == 1)	return;
	
    else{
		Node_q* temp = find_node(queue, tag);
		
		if(temp == queue->first) return; // 

		(temp->pre)->post = temp->post; //segmentation fault
        if(temp == queue->last){
            dequeue(queue);
            enqueue(queue, temp);
        }
        else{
            (temp->post)->pre = temp->pre;
		    (queue->count)--;
		    enqueue(queue, temp);
        }
		return;
	}
}


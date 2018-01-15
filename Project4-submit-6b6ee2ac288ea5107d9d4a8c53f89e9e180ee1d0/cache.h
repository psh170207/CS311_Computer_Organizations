#ifndef _CACHE_H_
#define _CACHE_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "linked_queue.h"

/* cache.h : Declare functions and data necessary for your project*/

int miss_penalty; // number of cycles to stall when a cache miss occurs
Queue **Cache; // data cache storing data [set][way][byte]
#define  BYTES_PER_WORD 4

void setupCache(int, int, int);
void setCacheMissPenalty(int);
uint32_t cache_read(uint32_t address);
void cache_write(uint32_t address, uint32_t value);
Node_q *from_mem(uint32_t address, int Tag, int Set_index, int Block_offset);
Node_q *eviction(Queue *qptr);

#endif

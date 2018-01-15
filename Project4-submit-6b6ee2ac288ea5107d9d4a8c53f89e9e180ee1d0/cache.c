#include <stdio.h>

#include "cache.h"
#include "util.h"

/* cache.c : Implement your functions declared in cache.h */


/***************************************************************/
/*                                                             */
/* Procedure: setupCache                  		       */
/*                                                             */
/* Purpose: Allocates memory for your cache                    */
/*                                                             */
/***************************************************************/

extern int STALL;
extern int cache_miss;

void setupCache(int capacity, int num_way, int block_size)
{
/*	code for initializing and setting up your cache	*/
/*	You may add additional code if you need to	*/
	
	int i,j; //counter
	int nset=0; // number of sets
	int _wpb=0; //words per block   
	nset=capacity/(block_size*num_way);
	_wpb = block_size/BYTES_PER_WORD;

	Cache = (Queue **)malloc(nset*sizeof(Queue *));
	
	for (i=0;i<nset;i++) {
		//Cache[i] = (uint32_t ** )malloc(num_way*sizeof(uint32_t*));
		Cache[i] = create_queue();
	}
	/*
	for (i=0; i<nset; i++){	
		for (j=0; j<num_way; j++){
			Cache[i][j]=(uint32_t*)malloc(sizeof(uint32_t)*(_wpb));
		}
	}
	*/

}


/***************************************************************/
/*                                                             */
/* Procedure: setCacheMissPenalty                  	       */
/*                                                             */
/* Purpose: Sets how many cycles your pipline will stall       */
/*                                                             */
/***************************************************************/

void setCacheMissPenalty(int penalty_cycles)
{
/*	code for setting up miss penaly			*/
/*	You may add additional code if you need to	*/	
	miss_penalty = penalty_cycles;

}

/* Please declare and implement additional functions for your cache */

uint32_t cache_read(uint32_t address){
	/*
	Associativity : 4-way, Capacity : 64Bytes, Block_size : 8Bytes 
	addr = Tag(28bits) | Set_index(1bit) | Block_offset(1bit) | Byte_offset(2bits)
	 */
	//int Byte_offset = address & 0x3; // Last 2 bits
	int Block_offset = (address>>2) & 0x1; // Next 1 bit
	int Set_index = (address>>3) & 0x1; // Next 1 bit
	int Tag = (address>>4) & 0xfffffff; // Remainders
	Node_q *Target = NULL;

	if((Target = find_node(Cache[Set_index], Tag)) != NULL){
		/* Read Hit */
		reconstruct_queue(Cache[Set_index],Tag); // For LRU
	}
	else{
		/* Read Miss */
		cache_miss = 1;
		STALL = 29;
	
		Target = from_mem(address,Tag,Set_index,Block_offset);
	}

	return (Target->block_data)[Block_offset];
}

void cache_write(uint32_t address, uint32_t value){
	/* Write-Allocate */
	
	int Byte_offset = address & 0x3; // Last 2 bits
	int Block_offset = (address>>2) & 0x1; // Next 1 bit
	int Set_index = (address>>3) & 0x1; // Next 1 bit
	int Tag = (address>>4) & 0xfffffff; // Remainders
	Node_q *Target = NULL;

	if( (Target = find_node(Cache[Set_index], Tag)) != NULL){
		/* Write Hit ; just write */
		Target->block_data[Block_offset] = value;
		Target->dirty = 1;
	}
	else{
		/* Write Miss */
		cache_miss = 1;
		STALL = 29;
	
		Node_q *temp = from_mem(address,Tag,Set_index,Block_offset);		
		temp->block_data[Block_offset] = value;
		temp->dirty = 1;
	}
}

Node_q *from_mem(uint32_t address, int Tag, int Set_index, int Block_offset){
	/* When Cache miss is occurs, Bring the data from memory to cache */
	
	uint32_t prev_word = 0;
	if(address>=MEM_DATA_START+4) prev_word = mem_read_32(address-4);

	uint32_t word = mem_read_32(address);
	uint32_t next_word = mem_read_32(address+4);
	int node_num = 0;
	Node_q *new_block = (Node_q *)malloc(sizeof(Node_q));
		
	new_block->tag = Tag;
	new_block->valid = 1;
	new_block->dirty = 0;
	new_block->address = address;
	
	if(Block_offset == 0){
		new_block->block_data[0] = word;
		new_block->block_data[1] = next_word;
	}
	else{
		new_block->block_data[0] = prev_word;
		new_block->block_data[1] = word;
	}
	if( (node_num = Cache[Set_index]->count) < 4){
		/* The set is not full ; just enqueue */
		switch(node_num){
			case 0:
				new_block->way = 0;
				break;
			case 1:
				new_block->way = 1;
				break;
			case 2:
				new_block->way = 2;
				break;
			case 3:
				new_block->way = 3;
				break;
		}
	}
	else{
		/* The set is full ; evict Least Recently Used Block and add new cache block */
		Node_q *evicted = eviction(Cache[Set_index]);
		
		new_block->way = evicted->way;
	}

	enqueue(Cache[Set_index],new_block);

	return new_block;
}

Node_q *eviction(Queue *qptr){
	/* Write-Back */
	Node_q *evicted = dequeue(qptr);

	if(evicted->dirty){
		uint32_t addr = evicted->address;
		int blocks[2] = {evicted->block_data[0],evicted->block_data[1]};
		//mem_write_32(addr, evicted->block_data[0]);
		//mem_write_32(addr+4, evicted->block_data[1]);
		mem_write_block(addr,blocks);
	}

	return evicted;
}

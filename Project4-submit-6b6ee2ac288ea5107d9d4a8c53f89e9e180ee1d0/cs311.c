/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   cs311.c                                                   */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*    DO NOT MODIFY THIS FILE!                                  */
/*    You should only modify the run.c, run.h and util.h file!  */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "parse.h"
#include "run.h"

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE *prog;
    int ii, word;
    char buffer[33];
    //to notifying data & text segment size
    int flag = 0;
    int text_index = 0;
    int data_index = 0;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    ii = 0;

    //read 32bits + '\0' = 33
    while (fgets(buffer,33,prog) != NULL)
    {
	if(flag == 0)
	{
	    //check text segment size
	    text_size = fromBinary(buffer);
	    NUM_INST = text_size/4;
	    //initial memory allocation of text segment
	    INST_INFO = malloc(sizeof(instruction)*NUM_INST);
	    init_inst_info(NUM_INST);
	}

	else if(flag == 1)
	{
	    //check data segment size
	    data_size = fromBinary(buffer);
	}

	else
	{
	    if(ii < text_size){
		INST_INFO[text_index++] = parsing_instr(buffer, ii);
	    }
	    else if(ii < text_size + data_size){
		parsing_data(buffer, ii-text_size);
	    }
	    else
	    {
		//Do not enter this case
		//assert(0);
		//However, there is a newline in the input file
	    }
	    ii += 4;
	}
	flag++;
    }
    CURRENT_STATE.PC = MEM_TEXT_START;
    //printf("Read %d words from program into memory.\n\n", ii/4);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename) { 
    int i;

    init_memory();
	
	setupCache(64,4,8);
	setCacheMissPenalty(30);
    
	load_program(program_filename);

    INSTRUCTION_COUNT = 0;
    CYCLE_COUNT = 0;
    BR_BIT = TRUE;
    FORWARDING_BIT = TRUE;

    for (i = 0; i < PIPE_STAGE; i++){
	CURRENT_STATE.PIPE[i] = 0;
	CURRENT_STATE.PIPE_STALL[i] = FALSE;
    }

    CURRENT_STATE.IF_ID_INST = 0;
    CURRENT_STATE.IF_ID_NPC = 0;
    CURRENT_STATE.ID_EX_NPC = 0;
    CURRENT_STATE.ID_EX_REG1 = 0;
    CURRENT_STATE.ID_EX_REG2 = 0;
    CURRENT_STATE.ID_EX_IMM = 0;
    CURRENT_STATE.EX_MEM_ALU_OUT = 0;
    CURRENT_STATE.EX_MEM_BR_TARGET = 0;
    CURRENT_STATE.MEM_WB_ALU_OUT = 0;
	
	/* initialization for additional Variables */
	CURRENT_STATE.IF_ID_REG1 = 0;
	CURRENT_STATE.IF_ID_REG2 = 0;
	CURRENT_STATE.ID_EX_REG1_NUM = 0;
	CURRENT_STATE.ID_EX_REG2_NUM = 0;
	CURRENT_STATE.IS_JMP = 0;
	CURRENT_STATE.IS_BRANCH = 0;
	CURRENT_STATE.JUMP_PC = 0;
	CURRENT_STATE.BRANCH_PC = 0;
	CURRENT_STATE.ID_EX_SHAMT = 0;
	CURRENT_STATE.ID_EX_FUNCT = 0;

	CURRENT_STATE.ID_EX_DEST = 0;

	CURRENT_STATE.EX_MEM_BR_TAKE = 0;
	CURRENT_STATE.EX_MEM_W_VALUE = 0;
	CURRENT_STATE.EX_MEM_DEST = 0;
	CURRENT_STATE.EX_MEM_FORWARD_REG = 0;
	CURRENT_STATE.EX_MEM_FORWARD_VALUE = 0;
	CURRENT_STATE.MEM_WB_DEST = 0;
	CURRENT_STATE.MEM_WB_MEM_OUT = 0;
	CURRENT_STATE.MEM_WB_FORWARD_REG = 0;
	CURRENT_STATE.MEM_WB_FORWARD_VALUE = 0;

    RUN_BIT = TRUE;
    FETCH_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    char** tokens;
    int count = 1;
    int addr1 = 0;
    int addr2 = 0;
    int num_inst = 0;
    int i = 100;		//for loop
    int cacheMissPenalty = 30;

    int mem_dump_set = 0;
    int debug_set = 0;
    int num_inst_set = 0;
    int pipe_dump_set = 0;
    int xdump_per_cycle=0;
    int xdump_set = 0;
    int cdump_set = 0;

    //cache configuration
    int cap=64;
    int assoc=4;
    int bsize=8;
	

    /* Error Checking */
    if (argc < 2)
    {
	printf("Usage: %s [-nobp] [-nof] [-m addr1:addr2] [-d] [-p] [-x] [-X] [-n num_instr] [-mc cycle_miss] [-c] inputBinary\n", argv[0]);
	exit(1);
    }

    initialize(argv[argc-1]);

    //for checking parse result

    while(count != argc-1){
	if(strcmp(argv[count], "-m") == 0){
	    tokens = str_split(argv[++count],':');

	    addr1 = (int)strtol(*(tokens), NULL, 16);
	    addr2 = (int)strtol(*(tokens+1), NULL, 16);
	    mem_dump_set = 1;
	}
	else if(strcmp(argv[count], "-d") == 0)
	    debug_set = 1;
	else if(strcmp(argv[count], "-n") == 0){
	    num_inst = (int)strtol(argv[++count], NULL, 10);
	    num_inst_set = 1;
	}
	else if(strcmp(argv[count], "-p") == 0)
	    pipe_dump_set = 1;
	else if(strcmp(argv[count], "-nobp") == 0)
	    BR_BIT = FALSE;
	else if(strcmp(argv[count], "-nof") == 0)
	    FORWARDING_BIT = FALSE;
        else if(strcmp(argv[count], "-mc") == 0)
            cacheMissPenalty = atoi(argv[++count]);
	else if(strcmp(argv[count], "-c")==0){
		cdump_set = 1;
	}
	else if(strcmp(argv[count], "-x")==0)
		xdump_set = 1;
	else if(strcmp(argv[count], "-X")==0)
		xdump_per_cycle = 1;
	else {
	   printf("Usage: %s [-nobp] [-nof] [-m addr1:addr2] [-d] [-p] [-x] [-X] [-n num_instr] [-mc cycle_miss] [-c] inputBinary\n", argv[0]);

	   exit(1);

	}
	count++;
    }
    

    if(num_inst_set)	MAX_INSTRUCTION_NUM = num_inst;
    else		MAX_INSTRUCTION_NUM = i;
    
    if (cacheMissPenalty <= 0) {
        printf("Error: cache miss penalty is zero or a negative number: %d\n", cacheMissPenalty);
        return -1;
    } else if (cacheMissPenalty > 500) {
        printf("Warning Cache miss penalty (%d) is pretty large, are you sure?\n", cacheMissPenalty);
    }
    setCacheMissPenalty(cacheMissPenalty);

    if(num_inst_set && num_inst <= 0){
	printf("Error: The number of instructions should be positive integer\n");
	return -1;
    }
    setupCache(cap,assoc,bsize);
    if(cdump_set) cdump(cap,assoc,bsize);
    if(debug_set || pipe_dump_set || xdump_per_cycle){
	printf("Simulating for %lu instructions...\n\n", MAX_INSTRUCTION_NUM);
	
	while(RUN_BIT){
	    cycle();
	    if(pipe_dump_set)	pdump();
	    if(debug_set)	rdump();	
            if(xdump_per_cycle)	xdump(cap/(assoc*bsize),assoc,bsize,Cache);
	    	
	}
	if(!debug_set)		rdump();
	if(mem_dump_set)	mdump(addr1, addr2);
	printf("Simulator halted after %lu cycles\n\n", CYCLE_COUNT);
    }
    else{
	run();

	rdump();
	if(mem_dump_set) mdump(addr1, addr2);
    }
    if (xdump_set)	xdump(cap/(assoc*bsize),assoc,bsize,Cache);

    return 0;
}

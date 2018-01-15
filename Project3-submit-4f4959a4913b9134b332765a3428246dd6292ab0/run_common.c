/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

CPU_State PREV_STATE;

uint32_t get_reg_val(unsigned char reg_num);
void set_reg_val(unsigned char target_reg. int val);
void IF_STAGE();
void ID_STAGE();
void EX_STAGE();
void MEM_STAGE();
void WB_STAGE();

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	/** Your implementation here */
	
	PREV_STATE = CURRENT_STATE;

	IF_STAGE();
	ID_STAGE();
	EX_STAGE();
	MEM_STAGE();
	WB_STAGE();
}

uint32_t get_reg_val(unsigned char reg_num){
	return CURRENT_STATE.REGS[reg_num];
}

void set_reg_val(unsigned char target_reg, int val){
	CURRENT_STATE.REGS[target_reg] =  val;
}

void IF_STAGE(){
	instruction *inst = get_inst_info(PREV_STATE.IF_ID_NPC);\
	CURRENT_STATE.IF_ID_NPC = PREV_STATE.IF_ID_NPC + 4;
	CURRENT_STATE.INST = inst;
}

void ID_STAGE(){
	instruction *inst = PREV_STATE.IF_ID_INST;
	CURRENT_STATE.ID_EX_NPC = PREV_STATE.IF_ID_NPC;
	CURRENT_STATE.ID_EX_REG1 = get_reg_val(RS(inst));
	CURRENT_STATE.ID_EX_REG2 = get_reg_val(RT(inst));
	CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
	CURRENT_STATE.ID_EX_SHAMT = SHAMT(inst);
	CURRENT_STATE.ID_EX_FUNCT = FUNC(inst);

	if(OPCODE(inst) == 0x0)	CURRENT_STATE.ID_EX_DEST = RD(inst);
	else CURRENT_STATE.ID_EX_DEST = RT(inst);

	CURRENT_STATE.ID_EX_OPCODE = OPCODE(inst);
}

void EX_STAGE(){
	uint32_t rs = PREV_STATE.ID_EX_REG1;
	uint32_t rt = PREV_STATE.ID_EX_REG2;
	int imm = PREV_STATE.ID_EX_IMM;
	int shamt = PREV_STATE.ID_EX_SHAMT;
	
	CURRENT_STATE.EX_MEM_OPCODE = PREV_STATE.ID_EX_OPCODE;
	CURRENT_STATE.EX_MEM_DEST = PREV_STATE.ID_EX_DEST;

	switch(PREV_STATE.ID_EX_OPCODE){
		case 0x9:
			//ADDIU
			CURRENT_STATE.EX_MEM_ALU_OUT = rs+imm;
		case 0xc:
			//ANDI
			CURRENT_STATE.EX_MEM_ALU_OUT = rs&(imm&0xffff);
		case 0xf:
			//LUI
			CURRENT_STATE.EX_MEM_ALU_OUT = imm<<16;
		case 0xd:
			//ORI
			CURRENT_STATE.EX_MEM_ALU_OUT = rs|(imm&0xffff);
		case 0xb:
			//SLTIU
			if(rs < imm) CURRENT_STATE.EX_MEM_ALU_OUT = 1;
			else CURRENT_STATE.EX_MEM_ALU_OUT = 0;
		case 0x23:
			//LW
		case 0x2b:
			//SW
			CURRENT_STATE.EX_MEM_W_VALUE = rt;
		case 0x4:
			//BEQ
			if(rs==rt){
				CURRENT_STATE.EX_MEM_BR_TARGET = PREV_STATE.ID_EX_NPC + imm*4;
				CURRENT_STATE.EX_MEM_BR_TAKE = 1;
			}
			else CURRENT_STATE.EX_MEM_BR_TAKE = 0;

		case 0x5:
			//BNE
			if(rs!=rt){
				CURRENT_STATE.EX_MEM_BR_TARGET = PREV_STATE.ID_EX_NPC + imm*4;
				CURRENT_STATE.EX_MEM_BR_TAKE = 1;
			}
			else CURRENT_STATE.EX_MEM_BR_TAKE = 0;
	
		case 0x0:
			//R-TYPE
			switch(PREV_STATE.ID_EX_FUNCT){
				case 0x21:
					//ADDU
					CURRENT_STATE.EX_MEM_ALU_OUT = rs+rt; /******/
				case 0x24:
					//AND
					CURRENT_STATE.EX_MEM_ALU_OUT = rs&rt;
				case 0x8:
					//JR
				case 0x27:
					//NOR
					CURRENT_STATE.EX_MEM_ALU_OUT = ~(rs|rt);
				case 0x25:
					//OR
					CURRENT_STATE.EX_MEM_ALU_OUT = rs|rt;
				case 0x2b:
					//SLTU
					if(rs<rt) CURRENT_STATE.EX_MEM_ALU_OUT = 1;
					else CURRENT_STATE.EX_MEM_ALU_OUT = 0;
				case 0x0:
					//SLL
					CURRENT_STATE.EX_MEM_ALU_OUT = rt<<shamt;
				case 0x2:
					//SRL
					CURRENT_STATE.EX_MEM_ALU_OUT = rt>>shamt;
				case 0x23:
					//SUBU
					CURRENT_STATE.EX_MEM_ALU_OUT = rs-rt;
			}
		case 0x2:
			//J
		case 0x3:
			//JAL
	}
}

void MEM_STAGE(){
	CURRENT_STATE.MEM_WB_ALU_OUT = PREV_STATE.EX_MEM_ALU_OUT;
	
	uint32_t addr = PREV_STATE.EX_MEM_ALU_OUT;

	if(PREV_STATE.EX_MEM_OPCODE == 0x23){
		//LW
		CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(addr);
	}
	if(PREV_STATE.EX_MEM_OPCODE == 0x2b){
		//SW
		mem_write_32(addr,PREV_STATE.W_VALUE);
	}

	CURRENT_STATE.MEM_WB_DEST = PREV_STATE.EX_MEM_DEST;
}

void WB_STAGE(){
	if(PREV_STATE.MEM_WB_OPCODE == 0x23) set_reg_val(PREV_STATE.MEM_WB_DEST,PREV_STATE.MEM_WB_MEM_OUT);
	else set_reg_val(PREV_STATE.MEM_WB_DEST, PREV_STATE.MEM_WB_ALU_OUT);
}



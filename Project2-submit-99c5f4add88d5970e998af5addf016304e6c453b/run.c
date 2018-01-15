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

uint32_t get_reg_val(unsigned char reg_num);
void set_reg_val(unsigned char target_reg, int val);

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
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
	/** Implement this function */
	if(CURRENT_STATE.PC >= MEM_TEXT_START + (NUM_INST-1)*4) RUN_BIT = FALSE;
	instruction *inst = get_inst_info(CURRENT_STATE.PC);
	unsigned char rs = RS(inst);
	unsigned char rt = RT(inst);
	
	if(OPCODE(inst)==0x0){
		/* Type-R */
		unsigned char rd = RD(inst);
		unsigned char shamt = SHAMT(inst);
		//printf("process_instruction : rd is %d shamt is %d\n",rd,shamt);
		//printf("process_instruction : get_reg_val test rs is %x rt is %x\n",get_reg_val(rs),get_reg_val(rt));
		switch(FUNC(inst)){
			case 0x21:
				// addu
				set_reg_val(rd,get_reg_val(rs)+get_reg_val(rt));
				//printf("addu : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));
				break;
			case 0x24:
				//and
				set_reg_val(rd,get_reg_val(rs)&get_reg_val(rt));
				//printf("and : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));
				break;
			case 0x0:
				// sll
				set_reg_val(rd,get_reg_val(rt)<<shamt);
				//printf("sll : %d is %x, %d is %x, shamt is %d\n",rd,get_reg_val(rd),rt,get_reg_val(rt),shamt);
				break;
			case 0x2:
				// srl
				set_reg_val(rd,get_reg_val(rt)>>shamt);
				//printf("srl : %d is %x, %d is %x, shamt  is %d\n",rd,get_reg_val(rd),rt,get_reg_val(rt),shamt);
				break;
			case 0x23:
				// subu
				set_reg_val(rd,(uint32_t)get_reg_val(rs)-(uint32_t)get_reg_val(rt));
				//printf("subu : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));

				break;
			case 0x2b:
				// sltu
				set_reg_val(rd,(get_reg_val(rs)<get_reg_val(rt))?1:0);
				//printf("sltu : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));

				break;
			case 0x27:
				// nor
				set_reg_val(rd,~(get_reg_val(rs)|get_reg_val(rt)));
				//printf("nor : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));

				break;
			case 0x25:
				// or
				set_reg_val(rd,get_reg_val(rs)|get_reg_val(rt));
				//printf("or : %d is %x, %d is %x, %d is %x\n",rd,get_reg_val(rd),rs,get_reg_val(rs),rt, get_reg_val(rt));

				break;
		}

		if(FUNC(inst)==0x8){
			/* jr Instruction */
			JUMP_INST(get_reg_val(rs));
			//printf("jr : %d is %x, PC is %x",rs,get_reg_val(rs),CURRENT_STATE.PC);
			return;
		}

		CURRENT_STATE.PC += 4;
		return;
	}
	else if(OPCODE(inst) == 0x2 || OPCODE(inst) == 0x3){
		/* J-Type */
		uint32_t target = TARGET(inst);
		if(OPCODE(inst) == 0x2) JUMP_INST(target<<2); // j
		if(OPCODE(inst) == 0x3){ // jal
			CURRENT_STATE.REGS[31] = CURRENT_STATE.PC+8; 
			JUMP_INST(target<<2);
		}
		return;
	}
	else{
		/* I-Type */
		short imm = IMM(inst);
		//printf("I-Type instruction : OPCODE is %x\n",(unsigned short)OPCODE(inst));
		switch((uint32_t)OPCODE(inst)){
			case 0x9:
				// addiu
				set_reg_val(rt, get_reg_val(rs)+SIGN_EX(imm));
				//printf("addiu : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm));

				break;
			case 0xc:
				// andi
				set_reg_val(rt, get_reg_val(rs)&(uint32_t)imm);
				//printf("andi : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),(uint32_t)imm);

				break;
			case 0xf:
				// lui
				set_reg_val(rt, imm<<16);
				//printf("lui : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm));

				break;
			case 0xd:
				// ori
				set_reg_val(rt, get_reg_val(rs)|(uint32_t)imm);
				//printf("ori : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),(uint32_t)imm);

				break;
			case 0xb:
				// sltiu
				set_reg_val(rt, (get_reg_val(rs)<SIGN_EX(imm))?1:0);
				//printf("sltiu : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm));

				break;
			case 0x23:
				// lw
				//printf("lw instruction is executed.\n");
				set_reg_val(rt, mem_read_32(get_reg_val(rs)+SIGN_EX(imm)));
				//printf("lw : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm));
		
				break;
			case 0x2b:
				// sw
				//printf("sw instruction is executed.\n");
				mem_write_32(mem_read_32(get_reg_val(rs)+SIGN_EX(imm)), get_reg_val(rt));
				//printf("sw : %d is %x, %d is %x, imm is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm));

				break;
		}
		
		if(OPCODE(inst) == 0x4){
			// beq
		   	if(get_reg_val(rs) == get_reg_val(rt)){
				CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + SIGN_EX(imm)*4;
				//printf("beq : %d is %x, %d is %x, imm is %x, PC is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm),CURRENT_STATE.PC);
			}
			else CURRENT_STATE.PC += 4;
			return;
		}

		if(OPCODE(inst) == 0x5){
			// bne
			if(get_reg_val(rs) != get_reg_val(rt)){
				CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + SIGN_EX(imm)*4;
				//printf("bne : %d is %x, %d is %x, imm is %x, PC is %x\n",rt,get_reg_val(rt),rs,get_reg_val(rs),SIGN_EX(imm),CURRENT_STATE.PC);
			}
			else CURRENT_STATE.PC += 4;
			return;
		}

		CURRENT_STATE.PC += 4;
		return;
	}
}

uint32_t get_reg_val(unsigned char reg_num){
	//printf("get_reg_val : reg_num is %d\n", reg_num);
	return CURRENT_STATE.REGS[(int)reg_num];
}

void set_reg_val(unsigned char target_reg, int val){
	CURRENT_STATE.REGS[target_reg] = val;
}

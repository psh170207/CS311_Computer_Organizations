/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

/* FINISH */

#include <stdio.h>

#include "util.h"
#include "run.h"

CPU_State PREV_STATE;

uint32_t get_reg_val(unsigned char reg_num);
void set_reg_val(unsigned char target_reg, int val);
void IF_PART();
void ID_PART();
void EX_PART();
void MEM_PART();
void WB_PART();
void flush(int is_jmp);
void stall();
int Is_write(int opcode, int func_code);

int EOI;// END OF INSTRUCTION

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
	EOI= MEM_TEXT_START+(NUM_INST-1)*4;
	
	//if(INSTRUCTION_COUNT == 0) MAX_INSTRUCTION_NUM += 4;

	if(CURRENT_STATE.PIPE[MEM_STAGE] >= EOI) RUN_BIT = FALSE;
		
	PREV_STATE = CURRENT_STATE;
	
	IF_PART();
	WB_PART();	
	ID_PART();
	EX_PART();
	MEM_PART();
//	printf("INSTRUCTION COUNT : %d\n",INSTRUCTION_COUNT);
}

uint32_t get_reg_val(unsigned char reg_num){
	return CURRENT_STATE.REGS[reg_num];
}

void set_reg_val(unsigned char target_reg, int val){
	CURRENT_STATE.REGS[target_reg] =  val;
}

void IF_PART(){
	if(PREV_STATE.PIPE_STALL[IF_STAGE] == 2) CURRENT_STATE.PIPE_STALL[IF_STAGE] = 1;
	else if(PREV_STATE.PIPE_STALL[IF_STAGE]==1){
		CURRENT_STATE.PIPE_STALL[IF_STAGE] = 0;
		return;
	}

	instruction *inst;
	
	if(PREV_STATE.IS_JMP){
		CURRENT_STATE.PC = PREV_STATE.JUMP_PC;
		CURRENT_STATE.IS_JMP = FALSE;
		//printf("jump : %x\n",CURRENT_STATE.PC);
	}
	else if(PREV_STATE.IS_BRANCH){
		CURRENT_STATE.PC = PREV_STATE.BRANCH_PC;
		CURRENT_STATE.IS_BRANCH = FALSE;
		//printf("branch : %x\n",CURRENT_STATE.PC);
	}
	else{
		if(PREV_STATE.IF_ID_NPC>=MEM_TEXT_START) CURRENT_STATE.PC = PREV_STATE.IF_ID_NPC;
		else CURRENT_STATE.PC = MEM_TEXT_START;
		//printf("CURRENT PC : %x\n",CURRENT_STATE.PC);
	}
	
	if(CURRENT_STATE.PC > EOI){
		CURRENT_STATE.PIPE[IF_STAGE] = 0;	
		return;
	}
	
	CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
	
	CURRENT_STATE.IS_BRANCH = FALSE;

	inst = get_inst_info(CURRENT_STATE.PC);
	//INSTRUCTION_COUNT++;
	//printf("IF_PART() : Instruction is 0x%x\n",inst);
	//printf("IF_PART() : Instruction content is 0x%x\n", *inst);
	CURRENT_STATE.PC += 4;
	CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
	CURRENT_STATE.IF_ID_INST = inst;

	//Load-use Detection unit
	CURRENT_STATE.IF_ID_REG1 = RS(inst);
	CURRENT_STATE.IF_ID_REG2 = RT(inst);	
}

void ID_PART(){
	if(PREV_STATE.PIPE_STALL[ID_STAGE] == 2) CURRENT_STATE.PIPE_STALL[ID_STAGE] = 1;
	else if(PREV_STATE.PIPE_STALL[ID_STAGE]==1){
		CURRENT_STATE.PIPE_STALL[ID_STAGE] = 0;
	//	printf("ID_PART() : OPCODE : 0x%x, REG1_NUM : %d, REG2_NUM : %d, REG1 : 0x%x, REG2 : 0x%x, IMM : 0x%x, DEST : %d\n", CURRENT_STATE.ID_EX_OPCODE, CURRENT_STATE.ID_EX_REG1_NUM, CURRENT_STATE.ID_EX_REG2_NUM, CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.ID_EX_REG2, CURRENT_STATE.ID_EX_IMM, CURRENT_STATE.ID_EX_DEST);
		return;
	}

	CURRENT_STATE.PIPE[ID_STAGE] = PREV_STATE.PIPE[IF_STAGE];

	if(CURRENT_STATE.PIPE[ID_STAGE] == 0) return;
	
	instruction *inst = PREV_STATE.IF_ID_INST;
	//printf("ID_PART() : Instruction is 0x%x\n", inst);	
	CURRENT_STATE.ID_EX_NPC = PREV_STATE.IF_ID_NPC;

	CURRENT_STATE.ID_EX_REG1 = get_reg_val(RS(inst));
	CURRENT_STATE.ID_EX_REG2 = get_reg_val(RT(inst));
	
	CURRENT_STATE.ID_EX_REG1_NUM = RS(inst);
	CURRENT_STATE.ID_EX_REG2_NUM = RT(inst);	// put register number
	
	CURRENT_STATE.ID_EX_IMM = SIGN_EX(IMM(inst));
	CURRENT_STATE.ID_EX_SHAMT = SHAMT(inst);
	CURRENT_STATE.ID_EX_FUNCT = FUNC(inst);
	
	if(OPCODE(inst) == 0x0)	CURRENT_STATE.ID_EX_DEST = RD(inst);
	else CURRENT_STATE.ID_EX_DEST = RT(inst);
	
	CURRENT_STATE.ID_EX_OPCODE = OPCODE(inst);
	

	if ((OPCODE(inst) == 0x23) && (CURRENT_STATE.ID_EX_DEST == CURRENT_STATE.IF_ID_REG1 || CURRENT_STATE.ID_EX_DEST == CURRENT_STATE.IF_ID_REG2)){
		CURRENT_STATE.PIPE_STALL[IF_STAGE] = 2;
		CURRENT_STATE.PIPE_STALL[ID_STAGE] = 2;
	}

	//supporting id stage jumps
	if(OPCODE(inst) == 0x2 || OPCODE(inst) == 0x3){
		CURRENT_STATE.IS_JMP = TRUE;
		CURRENT_STATE.JUMP_PC = TARGET(inst)<<2;
		if(OPCODE(inst) == 0x3) set_reg_val(31, CURRENT_STATE.PIPE[ID_STAGE]+8);
	}
	
	if(OPCODE(inst) == 0x0 && FUNC(inst) == 0x8){
	//JR
		CURRENT_STATE.IS_JMP = TRUE;
		CURRENT_STATE.JUMP_PC = CURRENT_STATE.ID_EX_REG1;
	}
//	printf("ID_PART() : OPCODE : 0x%x, REG1_NUM : %d, REG2_NUM : %d, REG1 : 0x%x, REG2 : 0x%x, IMM : 0x%x, DEST : %d\n", CURRENT_STATE.ID_EX_OPCODE, CURRENT_STATE.ID_EX_REG1_NUM, CURRENT_STATE.ID_EX_REG2_NUM, CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.ID_EX_REG2, CURRENT_STATE.ID_EX_IMM, CURRENT_STATE.ID_EX_DEST);
}

void EX_PART(){		
	if(PREV_STATE.PIPE_STALL[IF_STAGE] == 1){
		PREV_STATE.ID_EX_NPC = 0;
		PREV_STATE.ID_EX_REG1 = 0;
		PREV_STATE.ID_EX_REG2 = 0;
		PREV_STATE.ID_EX_REG1_NUM = 0;
		PREV_STATE.ID_EX_REG2_NUM = 0;
		PREV_STATE.ID_EX_IMM = 0;
		PREV_STATE.ID_EX_SHAMT = 0;
		PREV_STATE.ID_EX_FUNCT = 0;
		PREV_STATE.ID_EX_DEST = 0;
		PREV_STATE.ID_EX_OPCODE = 0;
		PREV_STATE.PIPE[ID_STAGE] = 0;
		CURRENT_STATE.PIPE_STALL[IF_STAGE] = 0;
	}
	else if(PREV_STATE.PIPE_STALL[IF_STAGE] == 2) CURRENT_STATE.PIPE_STALL[IF_STAGE] = 1;

//	printf("EX_PART() : PREV_STATE.ID_EX_DEST : %d\n",PREV_STATE.ID_EX_DEST);
	CURRENT_STATE.PIPE[EX_STAGE] = PREV_STATE.PIPE[ID_STAGE];
	
	uint32_t rs = PREV_STATE.ID_EX_REG1;
	uint32_t rt = PREV_STATE.ID_EX_REG2;
	int imm = PREV_STATE.ID_EX_IMM;
	int shamt = PREV_STATE.ID_EX_SHAMT;
	
	CURRENT_STATE.EX_MEM_OPCODE = PREV_STATE.ID_EX_OPCODE;
	CURRENT_STATE.EX_MEM_FUNCT = PREV_STATE.ID_EX_FUNCT;

	CURRENT_STATE.EX_MEM_DEST = PREV_STATE.ID_EX_DEST;
//	printf("EX_PART() : EX_MEM_DEST : %d\n",CURRENT_STATE.EX_MEM_DEST);
	//printf("EX_PART() : OPCODE is 0x%x\n", CURRENT_STATE.EX_MEM_OPCODE);
	
	if(CURRENT_STATE.PIPE[EX_STAGE] == 0) return;

	switch(PREV_STATE.ID_EX_OPCODE){
		case 0x9:
			//ADDIU
			CURRENT_STATE.EX_MEM_ALU_OUT = rs+imm;
			break;
		case 0xc:
			//ANDI
			CURRENT_STATE.EX_MEM_ALU_OUT = rs&(imm&0xffff);
			break;
		case 0xf:
			//LUI
			CURRENT_STATE.EX_MEM_ALU_OUT = imm<<16;
			break;
		case 0xd:
			//ORI
			CURRENT_STATE.EX_MEM_ALU_OUT = rs|(imm&0xffff);
			break;
		case 0xb:
			//SLTIU
			if(rs < imm) CURRENT_STATE.EX_MEM_ALU_OUT = 1;
			else CURRENT_STATE.EX_MEM_ALU_OUT = 0;
			break;
		case 0x23:
			//LW
			CURRENT_STATE.EX_MEM_ALU_OUT = rs+imm;	
			break;
		case 0x2b:
			//SW
			CURRENT_STATE.EX_MEM_ALU_OUT = rs+imm;
			CURRENT_STATE.EX_MEM_W_VALUE = rt;
			break;
		case 0x4:
			//BEQ
			//printf("BEQ : rs is %x rt is %x\n",rs,rt);
			if(rs==rt){
				CURRENT_STATE.EX_MEM_BR_TARGET = PREV_STATE.ID_EX_NPC + imm*4;
				CURRENT_STATE.EX_MEM_BR_TAKE = 1;
			}
			else CURRENT_STATE.EX_MEM_BR_TAKE = 0;
			break;
		case 0x5:
			//BNE
			//printf("BNE : rs is %x rt is %x\n",rs,rt);
			if(rs!=rt){
			//	printf("***SUCCESS!***\n");
				CURRENT_STATE.EX_MEM_BR_TARGET = PREV_STATE.ID_EX_NPC + imm*4;
				CURRENT_STATE.EX_MEM_BR_TAKE = 1;
			//	printf("BNE SUCCESS : BRANCH TARGET is 0x%x\n", CURRENT_STATE.EX_MEM_BR_TARGET);
			}
			else CURRENT_STATE.EX_MEM_BR_TAKE = 0;
			break;
		case 0x0:
			//R-TYPE
			switch(PREV_STATE.ID_EX_FUNCT){
				case 0x21:
					//ADDU
					CURRENT_STATE.EX_MEM_ALU_OUT = rs+rt; /******/
					break;
				case 0x24:
					//AND
					CURRENT_STATE.EX_MEM_ALU_OUT = rs&rt;
					break;
				case 0x27:
					//NOR
					CURRENT_STATE.EX_MEM_ALU_OUT = ~(rs|rt);
					break;
				case 0x25:
					//OR
					CURRENT_STATE.EX_MEM_ALU_OUT = rs|rt;
					break;
				case 0x2b:
					//SLTU
					if(rs<rt) CURRENT_STATE.EX_MEM_ALU_OUT = 1;
					else CURRENT_STATE.EX_MEM_ALU_OUT = 0;
					break;
				case 0x0:
					//SLL
					CURRENT_STATE.EX_MEM_ALU_OUT = rt<<shamt;
					break;
				case 0x2:
					//SRL
					CURRENT_STATE.EX_MEM_ALU_OUT = rt>>shamt;
					break;
				case 0x23:
					//SUBU
					CURRENT_STATE.EX_MEM_ALU_OUT = rs-rt;
					break;
				default:
					break;
			}
		default:
			break;
	}

	if(PREV_STATE.ID_EX_OPCODE == 0x2 || PREV_STATE.ID_EX_OPCODE == 0x3) flush(1);
	else if(PREV_STATE.ID_EX_OPCODE == 0x0 && PREV_STATE.ID_EX_FUNCT == 0x8) flush(1);
	
	CURRENT_STATE.EX_MEM_FORWARD_VALUE = CURRENT_STATE.EX_MEM_ALU_OUT;

	if(Is_write(CURRENT_STATE.EX_MEM_OPCODE, CURRENT_STATE.EX_MEM_FUNCT)
		&& CURRENT_STATE.EX_MEM_DEST != 0
		&& CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_REG1_NUM){
		CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
	}

	if(Is_write(CURRENT_STATE.EX_MEM_OPCODE, CURRENT_STATE.EX_MEM_FUNCT)
		&& CURRENT_STATE.EX_MEM_DEST !=0
		&& CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_REG2_NUM){
		CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
	}	

}

void MEM_PART(){
	CURRENT_STATE.PIPE[MEM_STAGE] = PREV_STATE.PIPE[EX_STAGE];

	if(CURRENT_STATE.PIPE[MEM_STAGE] == 0) return;

	CURRENT_STATE.MEM_WB_ALU_OUT = PREV_STATE.EX_MEM_ALU_OUT;

	CURRENT_STATE.IS_BRANCH = PREV_STATE.EX_MEM_BR_TAKE;
	CURRENT_STATE.BRANCH_PC = PREV_STATE.EX_MEM_BR_TARGET;
	
	CURRENT_STATE.MEM_WB_OPCODE = PREV_STATE.EX_MEM_OPCODE;
	CURRENT_STATE.MEM_WB_FUNCT = PREV_STATE.EX_MEM_FUNCT;

	uint32_t addr = PREV_STATE.EX_MEM_ALU_OUT;

	switch(PREV_STATE.EX_MEM_OPCODE){
		case 0x23:
			//LW
//			printf("MEM_PART() : LW addr is 0x%x\n",addr);
			CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(addr);
//			printf("MEM_PART() : MEM_OUT is 0x%x\n",CURRENT_STATE.MEM_WB_MEM_OUT);
			break;
		case 0x2b:
			//sw
			//printf("MEM_PART() : SW addr is 0x%x\n",addr);
			mem_write_32(addr, PREV_STATE.EX_MEM_W_VALUE);
			//printf("MEM_PART() : WRITED VALUE is 0x%x\n",mem_read_32(addr));
			break;
		case 0x4:
		case 0x5:
			//BRANCH
			if(CURRENT_STATE.IS_BRANCH)	flush(0);
			break;
		default:
			break;
	}

	CURRENT_STATE.MEM_WB_DEST = PREV_STATE.EX_MEM_DEST;
	
	/* Forwarding */
	CURRENT_STATE.MEM_WB_FORWARD_VALUE = (CURRENT_STATE.MEM_WB_OPCODE == 0x23)? CURRENT_STATE.MEM_WB_MEM_OUT:CURRENT_STATE.MEM_WB_ALU_OUT;
	
//	printf("MEM_PART() : Is_write() : %d, EX_MEM_DEST : %d, MEM_WB_OPCODE : 0x%x, MEM_WB_DEST : %d, ID_EX_REG1_NUM : %d, ID_EX_REG2_NUM: %d\n",Is_write(CURRENT_STATE.MEM_WB_OPCODE, CURRENT_STATE.MEM_WB_FUNCT),CURRENT_STATE.EX_MEM_DEST, CURRENT_STATE.MEM_WB_OPCODE, CURRENT_STATE.MEM_WB_DEST, CURRENT_STATE.ID_EX_REG1_NUM, CURRENT_STATE.ID_EX_REG2_NUM);
	if(Is_write(CURRENT_STATE.MEM_WB_OPCODE, CURRENT_STATE.MEM_WB_FUNCT)
		&& CURRENT_STATE.EX_MEM_DEST != CURRENT_STATE.ID_EX_REG1_NUM
		&& CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_REG1_NUM
		&& CURRENT_STATE.MEM_WB_DEST != 0){
		CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
//		printf("%d : 0x%x\n",CURRENT_STATE.ID_EX_REG1_NUM, CURRENT_STATE.ID_EX_REG1);
	}

	if(Is_write(CURRENT_STATE.MEM_WB_OPCODE, CURRENT_STATE.MEM_WB_FUNCT)
		&& CURRENT_STATE.EX_MEM_DEST != CURRENT_STATE.ID_EX_REG2_NUM
		&& CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_REG2_NUM
		&& CURRENT_STATE.MEM_WB_DEST != 0){
		CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
//		printf("%d : 0x%x\n",CURRENT_STATE.ID_EX_REG2_NUM, CURRENT_STATE.ID_EX_REG2);
	}
}
void WB_PART(){
	CURRENT_STATE.PIPE[WB_STAGE] = PREV_STATE.PIPE[MEM_STAGE];
	
	if(CURRENT_STATE.PIPE[WB_STAGE] == 0) return;

	if(PREV_STATE.MEM_WB_OPCODE == 0x23){
		set_reg_val(PREV_STATE.MEM_WB_DEST,PREV_STATE.MEM_WB_MEM_OUT);
		//printf("CHECK %d\n",PREV_STATE.MEM_WB_DEST);
		//printf("UPDATED REGISTER VAL : 0x%x\n", CURRENT_STATE.REGS[PREV_STATE.MEM_WB_DEST]);
	}
	else if(Is_write(PREV_STATE.MEM_WB_OPCODE, PREV_STATE.MEM_WB_FUNCT)) set_reg_val(PREV_STATE.MEM_WB_DEST, PREV_STATE.MEM_WB_ALU_OUT);

	INSTRUCTION_COUNT++;
}

void flush(int is_jmp){
	//printf("FLUSH CALLED arg : %d\n",is_jmp);
	if(is_jmp == 0){
		//Flush IF STAGE
		CURRENT_STATE.IF_ID_NPC = 0;
		CURRENT_STATE.IF_ID_INST = 0;
		CURRENT_STATE.IF_ID_REG1 = 0;
		CURRENT_STATE.IF_ID_REG2 = 0;
		CURRENT_STATE.PIPE[IF_STAGE] = 0;
		CURRENT_STATE.PC = 0;
				
		//Also flush ID,EX STAGES
		CURRENT_STATE.ID_EX_NPC = 0;
		CURRENT_STATE.ID_EX_REG1 = 0;
		CURRENT_STATE.ID_EX_REG2 = 0;
		CURRENT_STATE.ID_EX_REG1_NUM = 0;
		CURRENT_STATE.ID_EX_REG2_NUM = 0;
		CURRENT_STATE.ID_EX_IMM = 0;
		CURRENT_STATE.ID_EX_SHAMT = 0;
		CURRENT_STATE.ID_EX_FUNCT = 0;
		CURRENT_STATE.ID_EX_DEST = 0;
		CURRENT_STATE.ID_EX_OPCODE = 0;
		CURRENT_STATE.PIPE[ID_STAGE] = 0;
		
		CURRENT_STATE.EX_MEM_ALU_OUT = 0;
		CURRENT_STATE.EX_MEM_OPCODE = 0;
		CURRENT_STATE.EX_MEM_DEST = 0;
		CURRENT_STATE.EX_MEM_W_VALUE = 0;
		CURRENT_STATE.EX_MEM_BR_TAKE = 0;
		CURRENT_STATE.EX_MEM_BR_TARGET = 0;
		CURRENT_STATE.EX_MEM_FUNCT = 0;
		CURRENT_STATE.PIPE[EX_STAGE] = 0;
		//INSTRUCTION_COUNT -= 3;
	}
	else if(is_jmp == 1){
		//Flush ID STAGE
		CURRENT_STATE.ID_EX_NPC = 0;
		CURRENT_STATE.ID_EX_REG1 = 0;
		CURRENT_STATE.ID_EX_REG2 = 0;
		CURRENT_STATE.ID_EX_REG1_NUM = 0;
		CURRENT_STATE.ID_EX_REG2_NUM = 0;
		CURRENT_STATE.ID_EX_IMM = 0;
		CURRENT_STATE.ID_EX_SHAMT = 0;
		CURRENT_STATE.ID_EX_FUNCT = 0;
		CURRENT_STATE.ID_EX_DEST = 0;
		CURRENT_STATE.ID_EX_OPCODE = 0;
		CURRENT_STATE.PIPE[ID_STAGE] = 0;

		//INSTRUCTION_COUNT -= 1;
	}
}

void stall(){
	// nop to EX, MEM, WB_PART
}

int Is_write(int opcode, int func_code){
	int check_arr[] = {0x4, 0x5, 0x2, 0x2b, 0x3};
	int i;
	for(i=0; i<sizeof(check_arr)/sizeof(int); i++){
		if(opcode == check_arr[i]) return FALSE;
	}
	if(opcode == 0x0 && func_code == 0x8) return FALSE;
	return TRUE;
}

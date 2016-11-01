
#ifndef _IN_INSTRUCTION_H_
#define _IN_INSTRUCTION_H_

#include "qmachine/qmachine.h"

/*
 * Instructions in Qmachine are 32bits, 
 *
 * 3 parts instructions, such as MOV P1 P2, or ADD P1 P2
 * |   operand2   |   operand1   |    flag   |   opcode  |
 * |<---11bits--->|<---11bits--->|<--5bits-->|<--5bits-->|
 *
 * 2 parts instructions, such as JMP label, CAL label
 * |           operand1          |    flag   |   opcode  |
 * |<-----------22bits---------->|<--5bits-->|<--5bits-->|
 * JMP/CAL range is less than 4MB offset to current instruction
 * Between 0 ~ 0x3fffff 
 *
 * 1 part instructions, such NOP, RET
 * |    Reserved  |    flag   |   opcode  |
 * |<---22bits--->|<--5bits-->|<--5bits-->|
 *
 * flag: MNXYZ, 5bits, 
 *		 In MOV/ADD/SUB instructions,
 *       N=1, means operand1 is immediate operand (0x8)
 *       M=1, means operand2 is immediate operand (0x10)
 *       In JMP instruction,
 *       N=1, means offset is negtive  (0x8)
 *       N=0, means offset is positive (0x00)
 *       In all instructions,
 *       X=0, Y=0, Z=0, (0x0) means operate always
 *       X=0, Y=0, Z=1, (0x1) means operate if sign is less than 0
 *       X=0, Y=1, Z=0, (0x2) means operate if sign is less equal than 0
 *       X=1, Y=0, Z=1, (0x5) means operate if sign is greater than 0
 *       X=1, Y=1, Z=0, (0x6) means operate if sign is greater equal than 0
 *       X=1, Y=1, Z=1, (0x7) means operate if sign is just 0
 *
 * Right now, CMP can set the Sign rigister.
 * CMP A B, A and B can be both register or immediate number.
 * if A -- B set Sign = 000 (0x0) ALWAYS
 * if A <  B set Sign = 001 (0x1) LS
 * if A <= B set Sign = 010 (0x2) LE
 * if A >  B set Sign = 101 (0x5) GT
 * if A >= B set Sign = 110 (0x6) GE
 * if A == B set Sign = 111 (0x7) EQ
 */

#define FLAG_OP1_IMMEDIATE 0x8
#define FLAG_OP2_IMMEDIATE 0x10
#define FLAG_TRANS_FORWARD   0x8
#define FLAG_TRANS_BACKWARD  0x0
#define FLAG_NULL_SIGN 0x0
#define FLAG_LS_SIGN 0x1
#define FLAG_LE_SIGN 0x2
#define FLAG_GT_SIGN 0x5
#define FLAG_GE_SIGN 0x6
#define FLAG_EQ_SIGN 0x7
#define FLAG_ILLEGAL -1

enum Opcode { OP_START = -1, MOV = 0, ADD, SUB, JMP, CAL, RET, CMP, OP_END,};
enum Sign { SIGN_START = -1, SIGN_NULL, SIGN_EQ, SIGN_LE, SIGN_LS,
			 SIGN_GE, SIGN_GT, SIGN_END};

static const char* _instrTable[] = {
	"MOV", "ADD", "SUB", "JMP", "CAL", "RET", "CMP",
};

static const char* _signTable[] = {
	"", "EQ", "LE", "LS", "GE", "GT",
};

#define TransRangeLegal(o) ((((size_t)o) > 0x3fffff) ? 0 : 1)

int _InstrIndex(const char* token);

int _SignIndex(const char* sign);

Instruction _MakeInstr1(int opcode, int flag);

Instruction _MakeInstr2(int opcode, int op1, int flag);

Instruction _MakeInstr3(int opcode, int op1, int op2, int flag);

char* _InstrDecode(Instruction instr);

void _InstrRun(Qmachine qm);

#endif //!_IN_INSTRUCTION_H_

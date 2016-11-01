
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "in_machine.h"
#include "in_instr.h"
#include "in_miscell.h"
#include "in_memory.h"

int _InstrIndex(const char* token)
{
	int i, max;
	max = sizeof(_instrTable)/sizeof(char*);
	for(i = 0; i < max; i++)
		if(_strncmp(token, _instrTable[i], 3) == 0)//instruction symbol are at most 3 chars
			return i;
	return -1;
}

int _SignIndex(const char* sign)
{
	int i, max;
	max = sizeof(_signTable)/sizeof(char*);
	for(i = 0; i < max; i++)
		if(_strcmp(sign, _signTable[i]) == 0)
			return i;
	return -1;
}

#define Instr_Clear(x) ((x) = ((x)&0))
#define OPCODE(x)   ((x)&0x1F)
#define FLAG(x)     (((x)&0x1F)<<5)
#define OPERAND1(x) (((x)&0x3FF)<<10)
#define OPERAND2(x) (((x)&0x3FF)<<21)
#define ONLY_OPERAND1(x) (((x)&0x3FFFFF)<<10)

Instruction _MakeInstr3(int opcode, int op1, int op2, int flag)
{
	Instruction instr;

	Instr_Clear(instr);
	instr = OPERAND2(op2)|OPERAND1(op1)|FLAG(flag)|OPCODE(opcode);
	return instr;
}

Instruction _MakeInstr2(int opcode, int op1, int flag)
{
	Instruction instr;

	Instr_Clear(instr);
	instr = ONLY_OPERAND1(op1)|FLAG(flag)|OPCODE(opcode);
	return instr;
}

Instruction _MakeInstr1(int opcode, int flag)
{
	Instruction instr;

	instr = FLAG(flag)|OPCODE(opcode);
	return instr;
}

#define GET_OPCODE(x) ((x)&0x1F)
#define GET_FLAG(x)   (((x)>>5)&0x1F)
#define GET_OPERAND1(x) (((x)>>10)&0x3ff)
#define GET_OPERAND2(x) (((x)>>21)&0x3ff)
#define GET_ONLY_OPERAND1(x) (((x)>>10)&0x3fffff)

#define CheckFlagOP(x, f)    (((x)&0x18) == f)
#define GetFlagTrans(x)  ((x)&0x18)
#define GetFlagSign(x)   ((x)&0x7)
#define GetSignReg(r)    ((r)&0x7)

static char* _M_A_S_Decode(Instruction instr, enum Opcode OP)
{
	char* str;
	char tmp[100];
	int flag, op1, op2;

	str = (char*)qmalloc(sizeof(char)*100);
	sprintf(str, "Opcode:%s  FLAG:", _instrTable[OP]);
	flag = GET_FLAG(instr);
	strcat(str, "Op1-Reg ");
	if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
		strcat(str, "Op2-%   ");
	else
		strcat(str, "Op2-Reg ");

	switch(GetFlagSign(flag))
	{
	case FLAG_EQ_SIGN:
		strcat(str, "Sign equal ");
		break;
	case FLAG_GE_SIGN:
		strcat(str, "Sign greater equal ");
		break;
	case FLAG_GT_SIGN:
		strcat(str, "Sign greater than");
		break;
	case FLAG_LE_SIGN:
		strcat(str, "Sign less equal");
		break;
	case FLAG_LS_SIGN:
		strcat(str, "Sign less than");
		break;
	case FLAG_NULL_SIGN:
		strcat(str, "");
		break;
	default:
		strcat(str, "Unknown sign");
	}

	op1 = GET_OPERAND1(instr);
	if(op1 <= REG_START || op1 >= REG_END)
		strcat(str, " OP1:Unknow ");
	else
	{
		sprintf(tmp, " Op1:%s ", _regTable[op1]);
		strcat(str, tmp);
	}

	op2 = GET_OPERAND2(instr);
	if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
	{
		sprintf(tmp, "Op2:%%%d", op2);
		strcat(str, tmp);
	}
	else if(op2 <= REG_START || op2 >= REG_END)
		strcat(str, "Op2:Unknow");
	else
	{
		sprintf(tmp, "Op2:%s", _regTable[op2]);
		strcat(str, tmp);
	}

	strcat(str, "\n");
	return str;
}

#define _MOVDecode(i) _M_A_S_Decode((i), MOV)
#define _ADDDecode(i) _M_A_S_Decode((i), ADD)
#define _SUBDecode(i) _M_A_S_Decode((i), SUB)

static char* _J_C_Decode(Instruction instr, enum Opcode OP)
{
	char* str;
	char tmp[100];
	int flag;
	size_t offset;

	str = (char*)qmalloc(sizeof(char)*100);
	sprintf(str, "Opcode:%s  FLAG:", _instrTable[OP]);
	flag = GET_FLAG(instr);
	switch(GetFlagTrans(flag))
	{
	case FLAG_TRANS_FORWARD:
		strcat(str, "Trans forward ");;
		break;
	case FLAG_TRANS_BACKWARD:
		strcat(str, "Trans backward ");
		break;
	default:
		strcat(str, "Unknow sign");
	}

	switch(GetFlagSign(flag))
	{
	case FLAG_EQ_SIGN:
		strcat(str, "Sign equal ");
		break;
	case FLAG_GE_SIGN:
		strcat(str, "Sign greater equal ");
		break;
	case FLAG_GT_SIGN:
		strcat(str, "Sign greater than");
		break;
	case FLAG_LE_SIGN:
		strcat(str, "Sign less equal");
		break;
	case FLAG_LS_SIGN:
		strcat(str, "Sign less than");
		break;
	case FLAG_NULL_SIGN:
		strcat(str, "");
		break;
	default:
		strcat(str, "Unknown sign");
	}

	offset = GET_ONLY_OPERAND1(instr);
	sprintf(tmp, "   Offset:%lu\n", offset);
	strcat(str, tmp);
	return str;
}

#define _JMPDecode(i) _J_C_Decode((i), JMP)
#define _CALDecode(i) _J_C_Decode((i), CAL)

static char* _RETDecode(Instruction instr)
{
	char *str;

	str = (char*)qmalloc(sizeof(char)*100);
	memset(str, 0, sizeof(char)*100);
	sprintf(str, "Opcode:%s  FLAG: null Dest: get from stack runtime\n",\
			_instrTable[RET]);
	return str;
}

static char* _CMPDecode(Instruction instr)
{
	char *str;
	char tmp[100];
	int flag, op1, op2;

	str = (char*)qmalloc(sizeof(char)*100);
	memset(str, 0, sizeof(char)*100);
	memset(tmp, 0, sizeof(tmp));

	sprintf(str, "Opcode:%s  FLAG:", _instrTable[CMP]);
	flag = GET_FLAG(instr);
	if(CheckFlagOP(flag, FLAG_OP1_IMMEDIATE))
		strcat(str, "OP1-% ");
	else
		strcat(str, "OP1-Reg ");
	if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
		strcat(str, "OP2-% ");
	else
		strcat(str, "OP2-Reg ");

	switch(GetFlagSign(flag))
	{
	case FLAG_EQ_SIGN:
		strcat(str, "Sign equal ");
		break;
	case FLAG_GE_SIGN:
		strcat(str, "Sign greater equal ");
		break;
	case FLAG_GT_SIGN:
		strcat(str, "Sign greater than");
		break;
	case FLAG_LE_SIGN:
		strcat(str, "Sign less equal");
		break;
	case FLAG_LS_SIGN:
		strcat(str, "Sign less than");
		break;
	case FLAG_NULL_SIGN:
		strcat(str, "");
		break;
	default:
		strcat(str, "Unknown sign");
	}

	op1 = GET_OPERAND1(instr);
	if(CheckFlagOP(flag, FLAG_OP1_IMMEDIATE))
	{
		sprintf(tmp, "Op1:%%%d", op1);
		strcat(str, tmp);
	}
	else
	{
		sprintf(tmp, " Op1:%s ", _regTable[op1]);
		strcat(str, tmp);
	}

	op2 = GET_OPERAND2(instr);
	if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
	{
		sprintf(tmp, "Op2:%%%d", op2);
		strcat(str, tmp);
	}
	else if(op2 <= REG_START || op2 >= REG_END)
		strcat(str, "Op2:Unknow");
	else
	{
		sprintf(tmp, "Op2:%s", _regTable[op2]);
		strcat(str, tmp);
	}

	strcat(str, "\n");
	return str;
}

char* _InstrDecode(Instruction instr)
{
	char* str;
	int opcode;

	opcode = GET_OPCODE(instr);

	switch(opcode)
	{
	case MOV:
		str = _MOVDecode(instr);
		break;
	case SUB:
		str = _SUBDecode(instr);
		break;
	case ADD:
		str = _ADDDecode(instr);
		break;
	case JMP:
		str = _JMPDecode(instr);
		break;
	case CAL:
		str = _CALDecode(instr);
		break;
	case RET:
		str = _RETDecode(instr);
		break;
	case CMP:
		str = _CMPDecode(instr);
		break;
	default:
		str = (char*)qmalloc(sizeof(char)*100);
		sprintf(str, "Unknow instruction.\n");
	}

	return str;
}

static int Check_Flag_Sign(int flag, int sign)
{
	flag = GetFlagSign(flag);
	sign = GetSignReg(sign);

	if(flag == sign)
		return 1;
	if(flag == FLAG_LE_SIGN && sign == FLAG_EQ_SIGN)
		return 1;
	if(flag == FLAG_LE_SIGN && sign == FLAG_LS_SIGN)
		return 1;
	if(flag == FLAG_GE_SIGN && sign == FLAG_EQ_SIGN)
		return 1;
	if(flag == FLAG_GE_SIGN && sign == FLAG_GT_SIGN)
		return 1;

	return 0;
}
static void _M_A_S_Run(Qmachine qm, Instruction instr, enum Opcode OP, ReState *res)
{
	int op1, op2;
	int flag;

	flag = GET_FLAG(instr);
	if(GetFlagSign(flag) != FLAG_NULL_SIGN
	   && Check_Flag_Sign(flag, qm->sign._reg16.L))
	{
		Set_ReState(res, QM_ERR_DONE);
		return;
	}

	op1 = GET_OPERAND1(instr);
	op2 = GET_OPERAND2(instr);

	switch(OP)
	{
	case MOV:
		if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
			qm->gen[op1]._reg32 = op2;
		else
			qm->gen[op1]._reg32 = qm->gen[op2]._reg32;
		break;
	case SUB:
		if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
			qm->gen[op1]._reg32 -= op2;
		else
			qm->gen[op1]._reg32 -= qm->gen[op2]._reg32;
		break;
	case ADD:
		if(CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
			qm->gen[op1]._reg32 += op2;
		else
			qm->gen[op1]._reg32 += qm->gen[op2]._reg32;
		break;
	default:
		assert(0);//can't be here
	}
	Set_ReState(res, QM_SUCCESS);
}

#define _MOVRun(m, i, r) _M_A_S_Run((m), (i), MOV, (r))
#define _ADDRun(m, i, r) _M_A_S_Run((m), (i), ADD, (r))
#define _SUBRun(m, i, r) _M_A_S_Run((m), (i), SUB, (r))

static void _J_C_Run(Qmachine qm, Instruction instr, enum Opcode OP,
						ReState *res)
{
	size_t op1;
	int flag;

	flag = GET_FLAG(instr);
	if(GetFlagSign(flag) != FLAG_NULL_SIGN
		&& Check_Flag_Sign(flag, qm->sign._reg16.L))
	{
		qm->pc._isptr++;
		Set_ReState(res, QM_ERR_DONE);
		return;
	}

	if(OP == CAL)
	{
		ValueObj obj;
		obj.type = INT_TYPE;
		obj.value._i = (int)(qm->pc._isptr - qm->bin) + 1;//return to next instruction of CAL
		_stackPush(qm, obj);//push return address to the stack
		qm->sign._reg16.H++;//increase return sign in sign register
	}

	op1 = (size_t)GET_ONLY_OPERAND1(instr);
	if(GetFlagTrans(flag) == FLAG_TRANS_BACKWARD)
		qm->pc._isptr += op1;//current position add offset, jmp/call distination
	else
		qm->pc._isptr -= op1;


	Set_ReState(res, QM_SUCCESS);
}

#define _JMPRun(m, i, r) _J_C_Run((m), (i), JMP, (r))
#define _CALRun(m, i, r) _J_C_Run((m), (i), CAL, (r))

void _RETRun(Qmachine qm, Instruction instr, ReState *res)
{

    if(qm->sign._reg16.H <= 0)//no return address
    {
    	Set_ReState(res, QM_ERR_DONE);
		qm->pc._isptr++;
    }
	else//need to return
	{
		ValueObj obj = _stackPop(qm);
		if(obj.type == NULL_TYPE)
		{
			Set_ReState(res, QM_ERR_STACKEMPTY);
			assert(0);
		}
		else
		{
			qm->pc._isptr = (Instruction*)(qm->bin + obj.value._i);
			qm->sign._reg16.H--;
			Set_ReState(res, QM_SUCCESS);
		}
	}
}

void _CMPRun(Qmachine qm, Instruction instr, ReState *res)
{
	int flag;
	int op1, op2;

	flag = GET_FLAG(instr);
	if(GetFlagSign(flag) != FLAG_NULL_SIGN
		&& Check_Flag_Sign(flag, qm->sign._reg16.L))
	{
		Set_ReState(res, QM_ERR_DONE);
		return;
	}

	op1 = GET_OPERAND1(instr);
	op2 = GET_OPERAND2(instr);
	if(!CheckFlagOP(flag, FLAG_OP1_IMMEDIATE))
		op1 = (int)qm->gen[op1]._reg32;
	if(!CheckFlagOP(flag, FLAG_OP2_IMMEDIATE))
		op2 = (int)qm->gen[op2]._reg32;

	if(op1 == op2)
		qm->sign._reg16.L = FLAG_EQ_SIGN;
	else if(op1 < op2)
		qm->sign._reg16.L = FLAG_LS_SIGN;
	else
		qm->sign._reg16.L = FLAG_GT_SIGN;
}

void _InstrRun(Qmachine qm)
{
	Instruction instr;
	ReState res;

	qm->pc._isptr = qm->bin + 1;//bin[0] has been reserved
	printf("qm->bin:%lu\n", qm->bin);
	printf("qm->bin_top:%lu\n", qm->bin_top);
	while(qm->pc._isptr < (qm->bin + qm->bin_top))
	{
		instr = *(qm->pc._isptr);
		res.file     = qm->file;
		res.state    = QM_SUCCESS;
		res.line_num = (size_t)qm->pc._isptr;
		switch(GET_OPCODE(instr))
		{
		case MOV:
			printf("MOV run:%lu\n", (qm->pc._isptr - qm->bin));
			_MOVRun(qm, instr, &res);
			qm->pc._isptr++;
			break;
		case SUB:
			printf("SUB run:%lu\n", (qm->pc._isptr - qm->bin));
			_SUBRun(qm, instr, &res);
			qm->pc._isptr++;
			break;
		case ADD:
			printf("ADD run:%lu\n", (qm->pc._isptr - qm->bin));
			_ADDRun(qm, instr, &res);
			qm->pc._isptr++;
			break;
		case JMP:
			printf("JMP run:%lu\n", (qm->pc._isptr - qm->bin));
			_JMPRun(qm, instr, &res);
			break;
		case CAL:
			printf("CAL run:%lu\n", (qm->pc._isptr - qm->bin));
			_CALRun(qm, instr, &res);
			break;
		case RET:
			printf("RET run:%lu\n", (qm->pc._isptr - qm->bin));
			_RETRun(qm, instr, &res);
			break;
		case CMP:
			printf("CMP run:%lu\n", (qm->pc._isptr - qm->bin));
			_CMPRun(qm, instr, &res);
			qm->pc._isptr++;
			break;
		default:
			printf("XXXXXXXXXXXXXXXXXX\n");
			printf("Address:%x Unkonwn instruction.\n", qm->pc._isptr);
			qm->pc._isptr++;
		}

		if(res.state != QM_SUCCESS && res.state != QM_ERR_DONE)
			printf("Address:%x Error:%s\n", res.line_num, res.msg);
	}
}


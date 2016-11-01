
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "qmachine/qmachine.h"
#include "in_machine.h"
#include "in_hashtable.h"
#include "in_miscell.h"
#include "in_instr.h"
#include "in_memory.h"

#define MAX_LINE_SIZE 1024

//Delimiters are: 'space', '\tab'
static const char* _eatDelimiter(const char* line)
{
	while(*line == ' ' || *line == '\t')
		line++;
	return line;
}

static char* _getToken(const char* line)
{
	char *psz;
	int len = 0;

	//delimiters in 'line' should be removed before with _eatDelimiter(line)
	while(line[len] != ' ' && line[len] != '\t'
		  && line[len] != '\r' && line[len] != '\n' && line[len] != '\0')
		len++;
	psz = (char*)qmalloc(sizeof(char)*(len+1));//one more for '\0'
	strncpy(psz, line, len);
	psz[len] = '\0';
	return psz;
}

static int _SignToFlag(int sign)
{
	switch(sign)
	{
	case SIGN_NULL:
		return FLAG_NULL_SIGN;
	case SIGN_EQ:
		return FLAG_EQ_SIGN;
	case SIGN_LE:
		return FLAG_LE_SIGN;
	case SIGN_LS:
		return FLAG_LS_SIGN;
	case SIGN_GE:
		return FLAG_GE_SIGN;
	case SIGN_GT:
		return FLAG_GT_SIGN;
	default:
		return FLAG_ILLEGAL;
	}
}

static void _endToken(Qmachine qm, ReState *res)
{
	Instruction instr;

	instr = _MakeInstr1(RET, 0);
	res->state = _binAppend(qm, instr);
	res->msg = _errMsg[Get_errMsg_Index(res->state)];
}

static void _labelToken(Qmachine qm, char* token, const char* line,
							ReState *res)
{
	size_t len = 0;

	/*token can't be null, has been checked outside*/
	if(token[0] == '_' || isalpha(token[0]))//Can only start with '_' or letters
	{
		len = 1;
		while(token[len] == '_' || isalnum(token[len]))
			len++;//can be composed by '_' or numbers or letters
		if(token[len] == ':' && token[len+1] == '\0')
		{
			line = _eatDelimiter(line);
			/*Too long lines or null lines has been littered, so fgets()
			make sure there must be a '\r' or '\n' before '\0' in 'line' */
			if(line[0] == '\r' || line[0] == '\n')//no more illegal characters
			{
				token[len] = '\0';//remove the last char ':'
				if(_HashGet(qm->labelTable, token) == NULL)
				{
					/*label points to the first instruction in this block*/
					const char* value = _sizeToString(qm->bin_top);
					_HashInsert(qm->labelTable, token, value);
					Set_ReState(res, QM_SUCCESS);
					return;
				}
				else//multi-define labels
				{
					Set_ReState(res, QM_PARERR_DUP);
					return ;
				}
			}
		}
	}

	if(token[len] == '\0')
		res->state = QM_PARERR_SEMICOLON;
	else if(len == 0 || token[len] != ':' || token[len+1] != '\0')
		res->state = QM_PARERR_LABEL;
	else
		res->state = QM_PARERR_SURPLUS;
	res->msg = _errMsg[Get_errMsg_Index(res->state)];
}

//MOV/ADD/SUB instructions' parse are all the same
static void _M_A_S_Parse(Qmachine qm, const char* line,  enum Opcode op, ReState *res)
{
	char* token;
	int p1 = -1, p2 = -1;
	int sign = 0, flag = 0;

	res->state = QM_SUCCESS;
	token = _getToken(line);//get first token again, to check sign flags
	line += strlen(token);
	token += 3;//MOV/ADD/SUB are 3 chars
	sign = _SignIndex(token);
	qfree(token);
	if(sign <= SIGN_START || sign >= SIGN_END)
	{
		Set_ReState(res, QM_PARERR_SIGN);
		return;
	}
	flag |= _SignToFlag(sign);

	line  = _eatDelimiter(line);
	token = _getToken(line);
	line += strlen(token);
	p1    = _regIndex(token);//first parameter must be a register
	qfree(token);
	if(p1 < 0)
	{
		Set_ReState(res, QM_PARERR_REG);
		return ;
	}

	line  = _eatDelimiter(line);
	if(line[0] == '%')//immediate operand shoube star with '%'
	{
		line++;//skip '%'
		token = _getToken(line);
		line += strlen(token);
		if(strlen(token) == 1 && token[0] == '0')
		{
			p2 = 0;
			flag |= FLAG_OP2_IMMEDIATE;
		}
		else
		{
			p2 = atoi(token);
			if(p2 == 0)
				res->state = QM_PARERR_LEXICAL;
			else
				flag |= FLAG_OP2_IMMEDIATE;
		}
		qfree(token);
	}
	else//operand2 is also a register
	{
		token = _getToken(line);
		line += strlen(token);
		p2    = _regIndex(token);
		qfree(token);
		if(p2 < 0)
			res->state = QM_PARERR_LEXICAL;
		/*MOV R0 R0 is legal*/
	}

	if(res->state == QM_SUCCESS)
	{
		line  = _eatDelimiter(line);
		if(line[0] != '\r' && line[0] != '\n')//p1,p2 is legal, but need to make sure there isn't any more tokens
			res->state = QM_PARERR_SURPLUS;
		else
		{
			Instruction instr = _MakeInstr3(op, p1, p2, flag);
			res->state = _binAppend(qm, instr);
			res->msg = _errMsg[Get_errMsg_Index(res->state)];
		}
	}
	res->msg = _errMsg[Get_errMsg_Index(res->state)];
}

//Macros can be a little faster
#define _MOVParse(m, l, r) _M_A_S_Parse((m), (l), MOV, (r))
#define _ADDParse(m, l, r) _M_A_S_Parse((m), (l), ADD, (r))
#define _SUBParse(m, l, r) _M_A_S_Parse((m), (l), SUB, (r))
/*
static void _MOVParse(Qmachine qm, const char* line, ReState *res)
{
	_M_A_S_Parse(qm, line, MOV, res);
}

static void _ADDParse(Qmachine qm, const char* line, ReState *res)
{
	_M_A_S_Parse(qm, line, ADD, res);
}

static void _SUBParse(Qmachine qm, const char* line, ReState *res)
{
	_M_A_S_Parse(qm, line, SUB, res);
}
*/

static void _J_C_Parse(Qmachine qm, const char* line, enum Opcode OP,
					ReState *res)
{
	char *token;
	const char *value;
	size_t l_s;
	size_t offset = 0;
	int flag = 0, sign = 0;

	token  = _getToken(line);//get op token again, to check sign flag
	line  += strlen(token);
	token += 3;
	sign = _SignIndex(token);
	qfree(token);
	if(sign <= SIGN_START || sign >= SIGN_END)
	{
		Set_ReState(res, QM_PARERR_SIGN);
		return;
	}
	flag |= _SignToFlag(sign);

	line  = _eatDelimiter(line);
	token = _getToken(line);
	line += strlen(token);
	if(token[0] == '\0')
	{
		qfree(token);
		Set_ReState(res, QM_PARERR_DEST);
		return;
	}

	value = _HashGet(qm->labelTable, token);
	qfree(token);
	if(value == NULL)
	{
		Set_ReState(res, QM_PARERR_LEXICAL);
		return;
	}

	line = _eatDelimiter(line);
	if(line[0] != '\r' && line[0] != '\n')//there are more suplurs symbols
	{
		Set_ReState(res, QM_PARERR_LEXICAL);
		return;
	}

	l_s = (size_t)atol(value);
	if(l_s >= qm->bin_top)//need to jump backward, offset should be positive
		flag |= FLAG_TRANS_BACKWARD, offset = l_s - qm->bin_top;
	else
		flag |= FLAG_TRANS_FORWARD,  offset = qm->bin_top - l_s;

	if(!TransRangeLegal(l_s))
		Set_ReState(res, QM_PARERR_OUTRANGE);
	else
	{
		Instruction instr = _MakeInstr2(OP, offset, flag);
		res->state = _binAppend(qm, instr);

		if(res->state == QM_SUCCESS && OP == CAL)
		{
			ValueObj obj;
			obj.type = INT_TYPE;
			obj.value._i = (size_t)qm->bin_top - 1;
			res->state = _stackPush(qm, obj);
		}
		res->msg = _errMsg[Get_errMsg_Index(res->state)];
	}
}

#define _JMPParse(q, l, r) _J_C_Parse((q), (l), JMP, (r))
#define _CALParse(q, l, r) _J_C_Parse((q), (l), CAL, (r))

static void _CMPParse(Qmachine qm, const char* line, ReState *res)
{
	char *token;
	const char *value;
	int p[2], i;
	int flag = 0, sign = 0;

	res->state = QM_SUCCESS;
	token  = _getToken(line);//get op token again, to check sign flag
	line  += strlen(token);
	token += 3;
	sign   = _SignIndex(token);
	qfree(token);
	if(sign <= SIGN_START || sign >= SIGN_END)
	{
		Set_ReState(res, QM_PARERR_SIGN);
		return;
	}
	flag |= _SignToFlag(sign);

	for(i =0; i < 2; i++)
	{
		line  = _eatDelimiter(line);
		token = _getToken(line);
		line += strlen(token);
		if(token[0] == '%')
		{
			if(i == 0)
				flag |= FLAG_OP1_IMMEDIATE;
			else
				flag |= FLAG_OP1_IMMEDIATE;

			if(strlen(token) == 2 && token[1] == '0')//token[0] is '%'
				p[i] = 0;
			else
			{
				p[i] = atoi(token+1);
				if(p[i] == 0)
					res->state =  QM_PARERR_LEXICAL;
			}
		}
		else
		{
			p[i] = _regIndex(token);
			if(p[i] < 0)
				res->state =  QM_PARERR_REG;
		}
		qfree(token);
		res->msg = _errMsg[Get_errMsg_Index(res->state)];
		if(res->state != QM_SUCCESS)
			return;
	}

	line = _eatDelimiter(line);
	if(line[0] != '\r' && line[0] != '\n')//make sure there are no more symbols
	{
		Set_ReState(res, QM_PARERR_SURPLUS);
		return;
	}

	Instruction instr = _MakeInstr3(CMP, p[0], p[1], flag);
	res->state = _binAppend(qm, instr);
	res->msg = _errMsg[Get_errMsg_Index(res->state)];
}

#define ErrPrint(x) printf("error: file:%s line:%lu %s\n", \
							(x).file, (x).line_num, (x).msg)

int _AssemblyParse(Qmachine qm, const char* file)
{
	FILE   *fp;// c file operation, better portability
	char   *line;
	size_t  counter;
	ReState res;
	int errSign;

	res.file = file;
	fp = fopen(file, "rt");//read only mode
	if(fp == NULL)
	{
		Set_ReState(&res, QM_ERR_FILE);
		res.line_num = 0;
		ErrPrint(res);
		return QM_ERR_FILE;
	}

	counter = 0, errSign = 0;
	line = (char*)qmalloc(sizeof(char)*(MAX_LINE_SIZE+1));//one more for '\0'
	while(fgets(line, MAX_LINE_SIZE+1, fp))
	{
		counter++;
		res.line_num = counter;
		if(strlen(line) == MAX_LINE_SIZE && line[MAX_LINE_SIZE-1] != '\r'
			&& line[MAX_LINE_SIZE-1] != '\n')//1023 chars with one more line break is legal
		{
			Set_ReState(&res, QM_PARERR_LONG);
			ErrPrint(res), errSign = 1;

			//skip this too long line
			do{
				if(!fgets(line, MAX_LINE_SIZE+1, fp))
					return QM_ERR_DONE;//this too long line is the last line
				if(strlen(line) != MAX_LINE_SIZE || line[MAX_LINE_SIZE-1] != '\r'
					|| line[MAX_LINE_SIZE-1] != '\n')
					break;
			}while(1);

			continue;
		}

		line = (char*)_eatDelimiter(line);
		char* token = _getToken(line);
		if(token[0] == '\0')
		{
			qfree(token);
			continue; //'\r' and '\n' has been blocked in _getToken()
		}

		int index = _InstrIndex(token);
		switch(index)
		{
		case MOV:
			_MOVParse(qm, line, &res);
			break;
		case ADD:
			_ADDParse(qm, line, &res);
			break;
		case SUB:
			_SUBParse(qm, line, &res);
			break;
		case JMP:
			_JMPParse(qm, line, &res);
			break;
		case CAL:
			_CALParse(qm, line, &res);
			break;
		case CMP:
			_CMPParse(qm, line, &res);
			break;
		default:
			res.line_num = counter;
			line += strlen(token);
			if(_strcmp(token, "END") == 0)//'end' is a label not a instruction to users
				_endToken(qm, &res);
			else
				_labelToken(qm, token, line, &res);
		}
		qfree(token);
		if(res.state != QM_SUCCESS)
			ErrPrint(res), errSign = 1;
	}

	if(errSign)
		return QM_ERR_DONE;
	else
		return QM_SUCCESS;
}

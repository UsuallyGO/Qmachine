
#ifndef _IN_MACHINE_H_
#define _IN_MACHINE_H_

#include "in_miscell.h"
#include "in_hashtable.h"

#define Get_errMsg_Index(x)  ((x) + (-QM_PARERR_SURPLUS))

static const char* _errMsg[]= {
/*0  QM_PARERR_SURPLUS*/"surplus token",
/*1  QM_ERR_LABEL*/"label should be composed by '_' or letter or number",
/*2  QM_PARERR_DUMP*/"duplicate labels",
/*3  QM_PARERR_LONG*/"too much characters in a line(should less than 1024 chars) ",
/*4  QM_PARERR_SEMICOLON*/"label missing semicolon",
/*5  QM_PARERR_REG*/"unkown register label ",
/*6  QM_PARERR_LEXICAL*/"lexical error, illegal symbol ",
/*7  QM_PARERR_DEST*/"missing transfer destination",
/*8  QM_PARERR_OUTRANGE*/"destination out of legal range",
/*9  QM_PARERR_SIGN*/"unknow sign symbol",
/*10  */" ",
/*11  */" ",
/*12  */" ",
/*13 QM_ERR_STACKEMPTY*/"bin is empty, no more instruction",
/*14 QM_ERR_BINFULL*/"bin is full, no more space",
/*15 QM_ERR_STACKEMPTY*/"stack is empty, no more datas",
/*16 QM_ERR_STACKFULL*/"stack is full, no more space",
/*17 QM_ERR_FILE*/"file cannot open",
/*18*/"",
/*19 QM_ERR_DONE*/"nothing has been done",
/*20 QM_SUCCESS*/"Success, no erro", 
};

#define Set_ReState(x, s) do{(x)->state = (s);\
							 (x)->msg = _errMsg[Get_errMsg_Index((x)->state)];\
							}while(0)

enum {REG_START = -1, R0, R1, R2, R3, PC, SB, ST, REG_END};
static const char* _regTable[] = {
	"R0", "R1", "R2", "R3", "PC", "SB", "ST", 
};

typedef ValueObj* StackID;

typedef union _Register Register;
union _Register{
	unsigned int _reg32;
	struct{
		unsigned short L;
		unsigned short H;
	}_reg16;
	Instruction *_isptr;
	StackID skID;
};

struct Qmachine_t{
	Instruction *bin; //just as binary file
	size_t       bin_top;
	size_t       bin_size;
	Register     gen[4]; //R0-R3
	Register     pc; //program counter, position offsetof bin
	Register     sign;//sign flag register
	Register     sbase;//statck base pointer
	size_t       stop; //just a counter, base is sbase
	size_t       sk_size;
	HashTable   *labelTable;

	const char  *file;
};

int _regIndex(const char* reg);

ValueObj _stackPop(Qmachine qm);

int _stackPush(Qmachine qm, ValueObj obj);

int _binAppend(Qmachine qm, Instruction instr);

#endif //!_IN_MACHINE_H_
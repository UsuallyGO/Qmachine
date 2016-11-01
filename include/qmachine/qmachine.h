
#ifndef QMACHINE_H
#define QMACHINE_H

#include <stddef.h>

#define QM_SUCCESS    0
#define QM_ERR_DONE  -1
#define QM_ERR_INVAL -2
#define QM_ERR_FILE  -3
#define QM_ERR_STACKFULL  -4
#define QM_ERR_STACKEMPTY -5
#define QM_ERR_BINFULL  -6
#define QM_ERR_BINEMPTY -7

#define QM_PARERR_SIGN      -11
#define QM_PARERR_OUTRANGE  -12
#define QM_PARERR_DEST      -13
#define QM_PARERR_LEXICAL   -14
#define QM_PARERR_REG       -15
#define QM_PARERR_SEMICOLON -16
#define QM_PARERR_LONG      -17
#define QM_PARERR_DUP       -18
#define QM_PARERR_LABEL     -19
#define QM_PARERR_SURPLUS   -20

#define BIN_INITIAL_SIZE  1024 //1k instructions
#define BIN_MAXIMUM_SIZE  (1024*1024) //1million instructions
#define STACK_INITIAL_SIZE  (1024*1024) //1MB
#define STACK_MAXIMUM_SIZE  (1024*1024*200) //200MB
#define HASH_INITIAL_SIZE   512

//In Qmachine, instruction is 32bits, unsigned long is 64bits in X64 linux
typedef unsigned int Instruction;

//Users cannot see the details of Qmachine_t, they can only get the pointer
typedef struct Qmachine_t* Qmachine;

typedef struct ReState_t ReState;
struct ReState_t{
	int state;
	const char* msg;
	const char* file;
	size_t line_num;
};

//Just for debug, temporary
#define MemoryCheck _MemoryCheck
void _MemoryCheck();
void qm_instrDebug(Qmachine qm);

Qmachine qm_create();
int qm_assemblyTobin(Qmachine qm, const char* file);

#endif //!QMACHINE_H

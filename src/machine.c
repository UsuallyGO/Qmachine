
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "qmachine/qmachine.h"
#include "in_machine.h"
#include "in_hashtable.h"
#include "in_assparse.h"
#include "in_miscell.h"
#include "in_memory.h"
#include "in_instr.h"

Qmachine qm_create()
{
	Qmachine qm;
	qm = (Qmachine)qmalloc(sizeof(struct Qmachine_t));

	qm->bin = (Instruction*)qmalloc(sizeof(Instruction)*(BIN_INITIAL_SIZE+1));
	qm->bin_top = 1;//bin[0] is reserved
	qm->bin_size = BIN_INITIAL_SIZE;

	memset(qm->gen, 0, sizeof(qm->gen));
	qm->pc._isptr  = 0;//points to null
	qm->sign._reg32 = 0;
	qm->sbase.skID = (StackID)qmalloc(sizeof(ValueObj)*STACK_INITIAL_SIZE);
	qm->stop  = 0;
	qm->sk_size = STACK_INITIAL_SIZE;
	memset(qm->sbase.skID, 0, qm->sk_size*sizeof(ValueObj));

	qm->labelTable = (HashTable*)qmalloc(sizeof(HashTable));
	_HashTableInit(qm->labelTable, HASH_INITIAL_SIZE);

	qm->file = "No file";
	return qm;
}

//Generate bin code from assembly file
int qm_assemblyTobin(Qmachine qm, const char* file)
{
	int res;

	qm->file = file;
	res = _AssemblyParse(qm, file);
	return res;
}

int _regIndex(const char* reg)
{
	int index;

	for(index = 0; index < sizeof(_regTable)/sizeof(char*); index++)
		if(_strcmp(reg, _regTable[index]) == 0)
			return index;

	return -1;
}

int _stackPush(Qmachine qm, ValueObj obj)
{
	if(qm->stop >= qm->sk_size)
	{
		if(qm->sk_size >= STACK_MAXIMUM_SIZE)
			return QM_ERR_STACKFULL;

		if(qm->sk_size*2 < STACK_MAXIMUM_SIZE)
			qm->sk_size *= 2;
		else
			qm->sk_size = STACK_MAXIMUM_SIZE;

		qm->sbase.skID = (StackID)qrealloc(qm->sbase.skID, qm->sk_size);
	}

	*(qm->sbase.skID+ qm->stop )= obj;
	qm->stop++;
	return QM_SUCCESS;
}

ValueObj _stackPop(Qmachine qm)
{
	ValueObj obj;
	if(qm->stop == 0)
		obj.type = NULL_TYPE;
	else
	{
		qm->stop--;
		obj = *(qm->sbase.skID + qm->stop);
	}
	return obj;
}

int _binAppend(Qmachine qm, Instruction instr)
{
	if(qm->bin_top >= qm->bin_size)
	{
		if(qm->bin_size >= BIN_MAXIMUM_SIZE)
			return QM_ERR_BINFULL;
		if(qm->bin_size*2 < BIN_MAXIMUM_SIZE)
			qm->bin_size *= 2;
		else
			qm->bin_size = BIN_MAXIMUM_SIZE;

		qm->bin = (Instruction*)qrealloc(qm->bin, qm->bin_size);
	}

	//printf("_binAppend: %s %lu\n", _instrTabel[(instr)&0x1F], qm->bin_top);
	*(qm->bin + qm->bin_top) = instr;
	qm->bin_top++;

	return QM_SUCCESS;
}

void qm_instrDebug(Qmachine qm)
{
	size_t index;
	char *psz;

	if(qm->bin_top == 0)
		printf("No instruction now.\n");
	
	for(index = 1; index < qm->bin_top; index++)
	{
		psz = _InstrDecode(qm->bin[index]);
		printf("No.%lu %s", index, psz);
		qfree(psz);
	}
}

void qm_run(Qmachine qm)
{
   _InstrRun(qm);
}


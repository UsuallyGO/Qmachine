
#ifndef _IN_MISCELLANEOUS_H_
#define _IN_MISCELLANEOUS_H_

#include "qmachine/qmachine.h"

enum { NULL_TYPE = 0, INT_TYPE, CHAR_TYPE, PTR_TYPE,
       FLOAT_TYPE, DOUBLE_TYPE, INSTR_TYPE, };

typedef union DataObj_u DataObj;
union DataObj_u{
	int    _i;
	char   _c;
	void*  _ptr;
	float  _f;
	double _d;

	Instruction _is;
};

typedef struct ValueObj_t ValueObj;
struct ValueObj_t{
	int     type;
	DataObj value;
};

char* _strdup(const char* str);

int   _strcmp(const char*, const char*);

int _strncmp(const char* str1, const char* str2, size_t n);

const char* _sizeToString(size_t size);

#endif //!_IN_MISCELLANEOUS_H_

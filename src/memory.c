
#include <stdlib.h>
#include <stdio.h>

/*Notice: no #include "in_memory.h" here, malloc, realloc, free has been blocked
 *in this file, once this file has been included, you need to undef these macros.
 */
typedef struct _MemoryPack_t _MemoryPack;
struct _MemoryPack_t{
	const char* file;
	size_t line;
	size_t size;
	void  *ptr;

	_MemoryPack *next;
};

_MemoryPack *_memList = NULL;

void* _Malloc(size_t size, const char* file, size_t line)
{
	_MemoryPack *pack;
	void *block;
	
	pack = (_MemoryPack*)malloc(sizeof(_MemoryPack));
	block = (char*)malloc(size);

	pack->file = file;
	pack->line = line;
	pack->size = size;
	pack->ptr = block;
	pack->next = _memList;
	_memList = pack;

	return block;
}

void* _Realloc(void *ptr, size_t ns, const char* file, size_t line)
{
	_MemoryPack *pack;

	for(pack = _memList; pack != NULL; pack = pack->next)
	{
		if(pack->ptr == ptr)
			break;
	}

	if(pack == NULL)
		return NULL;

	pack->ptr = realloc(pack->ptr, ns);
	pack->file = file;
	pack->line = line;
	pack->size = ns;
	return pack->ptr;
}

void _Free(void *ptr)
{
	_MemoryPack *pack, *prev;

	if(_memList == NULL)
		return;

	pack = prev = _memList;
	if(_memList->ptr == ptr)
	{
		_memList = _memList->next;
		free(ptr), free(pack);
		return;
	}

	pack = pack->next;
	while(pack)
	{
		if(pack->ptr == ptr)
		{
			prev->next = pack->next;
			break;
		}

		prev = pack;
		pack = pack->next;
	}

	free(ptr), free(pack);
}

#define ErrMemPrint(x) printf("Memory Leak, file:%s line:%lu, size:%lu\n",\
								(x)->file, (x)->line, (x)->size)
void _MemoryCheck()
{
	_MemoryPack *pack;

	for(pack = _memList; pack != NULL; pack = pack->next)
		ErrMemPrint(pack);
}


#include <stdio.h>
#include <stdlib.h>
#include "qmachine/qmachine.h"
#include "in_hashtable.h"

const char* Key[] = {
	"Hello",
	"Good",
	"Check",
	"YouYou",
	"Cell",
};

const char* Value[] = {
	"World",
	"Bye",
	"Out",
	"Ball",
	"Phone",
};

const char* Test[] = {
	"Hello",
	"What",
	"Sheep",
	"YouYou",
	"Cell",
	"But",
	"Singer",
};

int main()
{
	HashTable *htable;
	const char* psz;
	int size, index;

	htable = (HashTable*)malloc(sizeof(HashTable));
	_HashTableInit(htable, HASH_INITIAL_SIZE);

	for(index = 0; index < sizeof(Key)/sizeof(char*); index++)
		_HashInsert(htable, Key[index], Value[index]);

	for(index = 0; index < sizeof(Test)/sizeof(char*); index++)
	{
		psz = _HashGet(htable, Test[index]);
		if(psz == NULL)
			printf("Key:%s not found.\n", Test[index]);
		else
			printf("Key:%s Value:%s\n", Test[index], psz);
	}

	MemoryCheck();
	
	return 0;
}

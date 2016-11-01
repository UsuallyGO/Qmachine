
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "qmachine/qmachine.h"
#include "in_miscell.h"
#include "in_hashtable.h"
#include "in_memory.h"

static unsigned int _hashPJW(const char *datum, size_t max_size)
{
    const int BITS_IN_int   = sizeof(int)*CHAR_BIT; //CHAR_BIT is defined in limists.h
    const int THREE_QUATERS = (int)((BITS_IN_int*3)/4);
    const int ONE_EIGHTH    = (int)(BITS_IN_int/8);
    const int HIGH_BITS     = (int)(~((unsigned int)(~0) >> ONE_EIGHTH));

    unsigned int hash, index;

    for(hash = 0; *datum; ++datum)
    {
        hash = (hash << ONE_EIGHTH) + *datum;
        if((index = hash&HIGH_BITS) != 0)
            hash = (hash^(index >> THREE_QUATERS))&(~HIGH_BITS);
    }
    return hash % max_size;
}

void _HashTableInit(HashTable *htable, size_t size)
{
	htable->table = (HashNode**)qmalloc(sizeof(HashNode*)*size);
	memset(htable->table, 0, size*sizeof(HashNode*));
    htable->table_size = size;
	htable->elemNum  = 0;
	htable->hashDup  = _strdup;
	htable->hashFunc = _hashPJW;
	htable->hashCmp  = _strcmp;
}

int _HashInsert(HashTable *htable, const char* key, const char* value)
{
	unsigned int index;
	HashNode *node;

	index = htable->hashFunc(key, htable->table_size);
	if(htable->table[index] == NULL)
	{
		node = (HashNode*)qmalloc(sizeof(HashNode));
		node->key   = htable->hashDup(key);
		node->value = htable->hashDup(value);
		node->next  = NULL;
		htable->table[index] = node;
		htable->elemNum++;
		return QM_SUCCESS;
	}
	else
	{
		HashNode *tmp = htable->table[index];
		while(tmp->next)//tmp can't be null
		{
			if(htable->hashCmp(tmp->key, key) == 0)//key is already exist
			{
				qfree(tmp->value);
				tmp->value = htable->hashDup(value);//update the value
				return QM_SUCCESS;
			}
		}
		//key is not in the table and 'tmp' is the last node
		node = (HashNode*)qmalloc(sizeof(HashNode));
		node->key = htable->hashDup(key);
		node->value = htable->hashDup(value);
		node->next = NULL;
		tmp->next = node;
		htable->elemNum++;
		return QM_SUCCESS;
	}
}

const char* _HashGet(HashTable *htable, const char *key)
{
	unsigned int index;
	HashNode *node;

	index = htable->hashFunc(key, htable->table_size);
	for(node = htable->table[index]; node; node = node->next)
		if(htable->hashCmp(key, node->key) == 0)
			return node->value;
	return NULL;
}

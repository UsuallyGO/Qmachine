
#ifndef _IN_HASHTABLE_H_
#define _IN_HASHTABLE_H_

typedef struct HashNode_t HashNode;
struct HashNode_t{
	char* key;
	char* value; //can't be const, we need to free(value) when we update a node
	HashNode *next;
};

//hash chain method to deal with hash collision
typedef struct HashTable_t HashTable;
struct HashTable_t{
	size_t table_size;
	size_t elemNum;
	HashNode **table;
	
	int   (*hashCmp)(const char*, const char*);
	char* (*hashDup)(const char*);
	unsigned int (*hashFunc)(const char*, size_t);
};


void _HashTableInit(HashTable *htable, size_t size);
int _HashInsert(HashTable *htable, const char* key, const char* value);
const char* _HashGet(HashTable *htable, const char *key);

#endif //!_IN_HASHTABLE_H_


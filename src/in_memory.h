
#ifndef _IN_MEMORY_H_
#define _IN_MEMORY_H_

//block the std memory function
#define malloc
#define free
#define realloc

#define qmalloc(x) _Malloc(x, __FILE__, __LINE__)
#define qfree(x)   _Free(x)
#define qrealloc(x, s) _Realloc(x, s, __FILE__, __LINE__)

void* _Malloc(size_t size, const char* file, size_t line);

void _Free(void *ptr);

void* _Realloc(void *ptr, size_t ns, const char* file, size_t line);

#endif //!_IN_MEMORY_H_

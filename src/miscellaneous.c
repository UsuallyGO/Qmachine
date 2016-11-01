
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "in_miscell.h"
#include "in_memory.h"

char* _strdup(const char* str)
{
	char* dp;
	dp = (char*)qmalloc((strlen(str)+1)*sizeof(char));
	strncpy(dp, str, strlen(str));
	dp[strlen(str)] = '\0';
	return dp;
}

int _strcmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
}

int _strncmp(const char* str1, const char* str2, size_t n)
{
	return strncmp(str1, str2, n);
}


const char* _sizeToString(size_t size)
{
	char* str;//even in 64bits system, will not more than 16 chars
	str = (char*)qmalloc(sizeof(char)*20);
	memset(str, 0, sizeof(str));
	sprintf(str, "%lu", size);
	return str;
}


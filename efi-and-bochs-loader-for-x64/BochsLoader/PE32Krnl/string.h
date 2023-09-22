// Copyright by Xiyue87 2022

#include "ctype.h"

#ifndef _INCLUDE_STRING_H_
#define _INCLUDE_STRING_H_

char* strcpy(char* s1, const char* s2);
char* strcat(char* s1, const char* s2);
int memcmp(const void* s1, const void* s2, size_t n);
void* memmove(void* dst, const void* src, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* s, int c, size_t n);

#endif /* _INCLUDE_STRING_H_ */

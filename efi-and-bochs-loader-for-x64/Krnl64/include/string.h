// copyright by Xiyue87 2022

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STRING_H_
#define _STRING_H_


char* strcpy(char* s1, const char* s2);
char* strcat(char* s1, const char* s2);
void* memmove(void* dst, const void* src, size_t n);
void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* s, int c, size_t n);

char* itoh(int v, char* b);
char* itoa(int v, char* b);
int itoa_r(char* buf, uint64_t value, unsigned radix);

#endif /* _STRING_H_ */


#ifdef __cplusplus
}
#endif

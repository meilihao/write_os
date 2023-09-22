// Copyright by Xiyue87 2022

#include "ctype.h"

char* strcpy(char* s1, const char* s2)
{
    char* r;
    char* ps1;
    const char* ps2;

    ps1 = s1;
    ps2 = s2;
    r = s1;

    while ((*ps1++ = *ps2++) != '\0')
        ;

    return (r);
}


char* strcat(char* s1, const char* s2)
{
    char* p = s1;

    while (*s1++ != '\0')
        ;

    s1--;

    while ((*s1++ = *s2++) != '\0')
        ;

    return (p);
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const char* p1;
    const char* p2;

    if (n == 0)
        return 0;

    p1 = (const char*)s1;
    p2 = (const char*)s2;

    while (*p1 == *p2)
    {
        p1++, p2++;
        if (--n == 0)
            return 0;
    }

    return (*p1 - *p2);
}


void* memmove(void* dst, const void* src, size_t n)
{
    char* p1 = (char*)dst;
    char* p2 = (char*)src;

    /* check if it may be overwritten */
    if (p2 > p1)
    {
        while (n)
        {
            *p1 = *p2;
            p1++;
            p2++;
            n--;
        }
    }
    else if (p2 < p1)
    {
        p1 = p1 + n - 1;
        p2 = p2 + n - 1;
        while (n)
        {
            *p1 = *p2;
            p1--;
            p2--;
            n--;
        }
    }

    return (dst);
}


void* memcpy(void* dst, const void* src, size_t n)
{
    return memmove(dst, src, n);
}

void* memset(void* s, int c, size_t n)
{
    unsigned char* p1;

    p1 = (unsigned char*)s;

    while (n > 0)
    {
        *p1++ = (unsigned char)c;
        n--;
    }

    return s;
}

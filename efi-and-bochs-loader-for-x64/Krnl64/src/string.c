// copyright by Xiyue87 2022

#include <ctype.h>

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

int itoa_r(char* buf, uint64_t value, unsigned radix)
{
    int i = 0;
    int head = 0;
    int tail = 0;
    uint64_t ret = value;
    char c = 0;

    while (ret != 0)
    {
        buf[i] = (char)((ret % radix) + '0');
        if (buf[i] > '9')
            buf[i] = (char)(buf[i] + 7);
        ret = ret / radix;
        i++;
    }
    if (i == 0)
        buf[i++] = '0';
    buf[i] = '\0';

    if (i > 0)
        tail = i - 1;

    while (tail > head)
    {
        c = buf[head];
        buf[head] = buf[tail];
        buf[tail] = c;
        tail--;
        head++;
    }

    return (i);
}

char* itoh(int v, char* b)
{
    char buf[40] = { 0 };

    itoa_r(buf, v, 16);

    strcpy(b, buf);

    return b;
}

char* itoa(int v, char* b)
{
    char buf[40] = { 0 };
    int f = 0;

    if (v < 0)
    {
        v = -v;
        f = 1;
    }
    itoa_r(buf, v, 10);

    if (f == 1)
    {
        strcpy(b, "-");
        strcat(b, buf);
    }
    else
    {
        strcpy(b, buf);
    }

    return b;
}

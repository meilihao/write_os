// Copyright by Xiyue87 2022

#include "string.h"
#include "stdio.h"

int itoa_r(char* buf, int value, int radix)
{
    int i = 0;
    int head = 0;
    int tail = 0;
    unsigned int ret = (unsigned int)value;
    char c = 0;

    while (ret != 0)
    {
        buf[i] = (char)((ret % (unsigned)radix) + '0');
        if (buf[i] > '9')
            buf[i] = (char)(buf[i] + 7);
        ret = ret / (unsigned)radix;
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
    char buf[20] = { 0 };

    itoa_r(buf, v, 16);

    strcpy(b, buf);

    return b;
}

char* itoa(int v, char* b)
{
    char buf[20] = { 0 };
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

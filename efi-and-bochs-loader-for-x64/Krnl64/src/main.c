// copyright by Xiyue87 2022

#include <ctype.h>
#include <sys64.h>
#include <siodebug.h>

int main()
{
    SioPuts("Hello long mode\n");
    SioPutsValueWithMsg("RIP: ", get_rip(), "\n");
    SioPutsValueWithMsg("RSP: ", get_rsp(), "\n");
    SioPutsValueWithMsg("CS:  ", get_cs(), "\n");
    SioPutsValueWithMsg("SS:  ", get_ss(), "\n");
    while (1)
    {

    }
    return 0;
}
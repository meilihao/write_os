// copyright by Xiyue87 2022

#include <sys64.h>
#include <string.h>
#include <siodebug.h>

#define UART_BAUD_RATE 115200

#define UART_BASE 0x3f8
#define UART_LCR 3
#define UART_LSR 5

char * ULLToHex(uint64_t Value, char * String, int MinSize, char PadChar)
{
    char* p = String;
    int s;

    s = itoa_r(String, Value, 16);

    if (MinSize > s)
    {
        memmove(String + (MinSize - s), String, s + 1);
        memset(String, PadChar, MinSize - s);
    }

    return String;
}

void SioPutsValueWithMsg(const char* Msg, uint64_t Value, const char* EndStr)
{
    char StrVal[64] = { 0 };
    SioPuts(Msg);
    SioPuts(ULLToHex(Value, StrVal, 16, '0'));
    SioPuts(EndStr);

}

void SioPuts(const char* String)
{
    io_out_byte(UART_BASE + UART_LCR, 0x80);
    io_out_byte(UART_BASE, 0x1c200 / UART_BAUD_RATE);
    io_out_byte(UART_BASE + UART_LCR, 0x3);

    while (*String != '\0')
    {
        while (!(io_in_byte(UART_BASE + UART_LSR) & 0x40));
        if (*String == '\n')
            io_out_byte(UART_BASE, '\r');
        io_out_byte(UART_BASE, *String);
        String++;
    }
}

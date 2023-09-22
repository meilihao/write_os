// Copyright by Xiyue87 2022

#include "sys32.h"

#define UART_BAUD_RATE 115200

#define UART_BASE 0x3f8
#define UART_LCR 3
#define UART_LSR 5

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

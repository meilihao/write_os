// Copyright by Xiyue87 2022

#include "ctype.h"

#ifndef _INCLUDE_SYS32_H_
#define _INCLUDE_SYS32_H_

uint8_t io_in_byte(uint16_t port);
void io_out_byte(uint16_t port, uint8_t data);
void asm_delay();
void io_in_word_string(uint16_t port, uint16_t* data, uint32_t count);

#endif /* _INCLUDE_SYS32_H_ */

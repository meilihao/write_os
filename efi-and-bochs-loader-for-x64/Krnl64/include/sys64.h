// copyright by Xiyue87 2022

#include "ctype.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INCLUDE_SYS64_H_
#define _INCLUDE_SYS64_H_

uint8_t io_in_byte(uint16_t port);
void io_out_byte(uint16_t port, uint8_t data);
void io_out_word(uint16_t port, uint16_t data);
unsigned long get_rip();
unsigned long get_rsp();
unsigned short get_cs();
unsigned short get_ss();
unsigned long get_cr0();
unsigned long get_cr2();
unsigned long get_cr3();
unsigned long get_cr4();
unsigned long read_msr(unsigned int msr_addr);

void lock_int();
void unlock_int();

#endif /* _INCLUDE_SYS64_H_ */

#ifdef __cplusplus
}
#endif

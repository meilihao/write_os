// copyright by Xiyue87 2022

#include "ctype.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SIODEBUG_H_
#define _SIODEBUG_H_

char* ULLToHex(uint64_t Value, char* String, int MinSize, char PadChar);
void SioPuts(const char* String);
void SioPutsValueWithMsg(const char* Msg, uint64_t Value, const char* EndStr);

#endif /* _SIODEBUG_H_ */


#ifdef __cplusplus
}
#endif


/****************************************************************
        Cosmos HAL全局初始化文件halinit.c
*****************************************************************
                彭东
****************************************************************/

#include "cosmostypes.h"
#include "cosmosmctrl.h"


void init_hal()
{

    init_halplaltform();         // 初始化平台
    move_img2maxpadr(&kmachbsp);
    init_halmm();                // 初始化内存 
    init_halintupt();            // 初始化中断
    return;
}

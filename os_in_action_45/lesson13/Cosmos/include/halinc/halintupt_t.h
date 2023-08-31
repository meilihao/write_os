/**********************************************************
        hal层中断处理头文件halintupt_t.h
***********************************************************
                彭东
**********************************************************/
#ifndef _HALINTUPT_T_H
#define _HALINTUPT_T_H



#ifdef CFG_X86_PLATFORM

typedef struct s_ILNEDSC
{
    u32_t ild_modflg;
    u32_t ild_devid;
    u32_t ild_physid;
    u32_t ild_clxsubinr;
}ilnedsc_t;

// 中断异常描述符
typedef struct s_INTFLTDSC
{
    spinlock_t  i_lock;
    u32_t       i_flg;
    u32_t       i_stus;
    uint_t      i_prity;        //中断优先级
    uint_t      i_irqnr;        //中断号
    uint_t      i_deep;         //中断嵌套深度
    u64_t       i_indx;         //中断计数
    list_h_t    i_serlist;      //也可以使用中断回调函数的方式
    uint_t      i_sernr;        //中断回调函数个数
    list_h_t    i_serthrdlst;   //中断线程链表头
    uint_t      i_serthrdnr;    //中断线程个数
    void*       i_onethread;    //只有一个中断线程时直接用指针
    void*       i_rbtreeroot;   //如果中断线程太多则按优先级组成红黑树
    list_h_t    i_serfisrlst;   //也可以使用中断回调函数的方式
    uint_t      i_serfisrnr;    //中断回调函数个数
    void*       i_msgmpool;     //可能的中断消息池
    void*       i_privp;
    void*       i_extp;
}intfltdsc_t;

// 中断可以由线程的方式执行，也可以是一个回调函数，该函数的地址放在s_INTSERDSC结构体中
// 如果内核或者设备驱动程序要安装一个中断处理函数，就要先申请一个 intserdsc_t 结构体，然后把中断函数的地址写入其中，最后把这个结构挂载到对应的 intfltdsc_t 结构中的 i_serlist 链表中
// 不能直接把中断处理函数放在 intfltdsc_t 结构中呢，还要多此一举搞个 intserdsc_t 结构体: 每个设备都可能产生中断，但是中断控制器的中断信号线是有限的, 即存在多个设备共享一根中断信号线的情况. 因此让设备驱动程序来决定，因为它是最了解设备的即设备端的中断，交给设备驱动程序
typedef struct s_INTSERDSC{    
    list_h_t    s_list;        //在中断异常描述符中的链表
    list_h_t    s_indevlst;    //在设备描述描述符中的链表
    u32_t       s_flg;        
    intfltdsc_t* s_intfltp;    //指向中断异常描述符 
    void*       s_device;      //指向设备描述符
    uint_t      s_indx;    
    intflthandle_t s_handle;   //中断处理的回调函数指针
}intserdsc_t;

typedef struct s_KITHREAD
{
    spinlock_t  kit_lock;
    list_h_t    kit_list; 
    u32_t       kit_flg;
    u32_t       kit_stus;
    uint_t      kit_prity;
    u64_t       kit_scdidx;
    uint_t      kit_runmode;
    uint_t      kit_cpuid;
    u16_t       kit_cs;
    u16_t       kit_ss;
    uint_t      kit_nxteip;
    uint_t      kit_nxtesp;
    void*       kit_stk;
    size_t      kit_stksz;
    void*       kit_runadr;
    void*       kit_binmodadr;
    void*       kit_mmdsc;
    void*       kit_privp;
    void*       kit_extp;
}kithread_t;

#endif


#endif // HALINTUPT_T_H

# README
hal_start()
    init_hal()    // 初始化硬件抽象层
        init_halplaltform()    //初始化平台相关
            init_machbstart()    // 将grub启动阶段得到的机器信息结构体machbstart_t拷贝一份到hal全局machbstart_t中，以便后续hal使用
            init_bdvideo()    //初始化显卡驱动（基于hal全局machbstart_t里面的信息）
        move_img2maxpadr()
        init_halmm()    //初始化内存管理
            init_phymmarge()  //根据引导阶段获得的内存信息 -- e820map_t 结构数组，hal扩张建立了自己的内存管理结构 -- phymmarge_t 结构数组，将e820map_t 结构数组的信息拷贝了过来
        init_halintupt()    //hal中断初始化
            init_descriptor()    //设置中断GDT描述符
            init_idt_descriptor()    //设置中断IDT描述符：将中断处理程序的入口地址与对应的中断/异常类型vector绑定
            init_intfltdsc()    //初始化中断管理结构体：管理中断的类型，分发，处理优先级等
            init_i8259()    //初始化中断控制器 i8259
            i8259_enabled_line()
    init_krl()    //初始化内核: 死循环

## 效果
使用lesson10~11的HelloOS/setup.sh测试

qemu-system-x86_64启动是黑屏, 并会自动重启(已解决acpi问题).

> 思考题 1: 内核从完成必要的初始化到用户态程序的过程是怎么样的？尝试描述一下调用关系。

首先执行`_start`函数，设置启动栈后跳转执行`init_c`函数，在`init_c`函数中：
1. 调用`clear_bss`函数清零`.bss`段；
2. 调用`early_uart_init`函数初始化UART；
3. 调用`init_boot_pt`函数配置启动页表；
4. 调用`el1_mmu_activate`函数启用MMU；
5. 调用`start_kernel`函数跳转到高地址；

通过`start_kernel`跳转到高地址后，跳转到内核的`main`函数（位于 kernel/arch/aarch64/main.c）。在`main`函数中：
1. 调用`uart_init`函数初始化uart模块；
2. 调用`mm_init`函数初始化内存管理模块；
3. 调用`arch_interrupt_init`函数初始化异常向量；
4. 调用`create_root_thread`函数创建根进程：
   1. 调用`create_root_cap_group`函数创建第一个用户进程；
   2. 调用`__create_root_thread`函数为第一个用户进程创建一个用户线程；
   3. 调用`obj_get`、`obj_put`、`switch_to_thread`函数将此用户线程设置为当前线程；
5. 调用`switch_context`函数完成上下文切换，为线程或者进程的切换做准备；
6. 调用`eret_to_thread`函数实现从内核态到用户态的切换。

> 思考题 8： ChCore中的系统调用是通过使用汇编代码直接跳转到`syscall_table`中的
> 相应条目来处理的。请阅读`kernel/arch/aarch64/irq/irq_entry.S`中的代码，并简要描述ChCore是如何将系统调用从异常向量分派到系统调用表中对应条目的。

将系统调用号存入x8寄存器，并将函数的0 ~ 7号参数依次存入x0~x7寄存器中，最后使用SVC指令触发异常进入系统调用。
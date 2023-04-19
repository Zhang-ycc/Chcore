
> 思考题 1：阅读汇编代码`kernel/arch/aarch64/boot/raspi3/init/start.S`。说明ChCore是如何选定主CPU，并阻塞其他其他CPU的执行的。

```
    mrs	x8, mpidr_el1
    and	x8, x8,	#0xFF
    cbz	x8, primary
```
只有满足mpidr_el1低八位（Affinity level 0）为0的核被选定为主CPU，进入初始化流程。

```
wait_for_bss_clear:
    adr	x0, clear_bss_flag
    ldr	x1, [x0]
    cmp     x1, #0
    bne	wait_for_bss_clear
```
其他核在`wait_for_bss_clear`处循环，直到主CPU在`init_c`中调用`clear_bss`将`clear_bss_flag`置0，才继续执行。

```
	bl 	arm64_elX_to_el1

	mov	x1, #INIT_STACK_SIZE
	mul	x1, x8, x1
	ldr 	x0, =boot_cpu_stack
	add	x0, x0, x1
	add	x0, x0, #INIT_STACK_SIZE
	mov	sp, x0

wait_until_smp_enabled:
	mov	x1, #8
	mul	x2, x8, x1
	ldr	x1, =secondary_boot_flag
	add	x1, x1, x2
	ldr	x3, [x1]
	cbz	x3, wait_until_smp_enabled

	mov	x0, x8
	bl 	secondary_init_c
```

降低异常等级到el1并设置stack pointer后，在`wait_until_smp_enabled`处循环，直到`secondary_boot_flag`不为0，设置CPU id并执行`secondary_init_c`。


> 思考题 2：阅读汇编代码`kernel/arch/aarch64/boot/raspi3/init/start.S, init_c.c`以及`kernel/arch/aarch64/main.c`，解释用于阻塞其他CPU核心的`secondary_boot_flag`是物理地址还是虚拟地址？是如何传入函数`enable_smp_cores`中，又该如何赋值的（考虑虚拟地址/物理地址）？

是物理地址。

```
BEGIN_FUNC(start_kernel)
    ...
    str     x0, [sp, #-8]!
    ...
    ldr     x0, [sp], #8
    bl      main
END_FUNC(start_kernel)

void main(paddr_t boot_flag){
    ...
    enable_smp_cores(boot_flag);
    ...
}
```

在init_c.c中调用start_kernel，在start_kernel中将寄存在x0中的`secondary_boot_flag`传递到main函数中；在main函数中，调用`enable_smp_cores`，将参数传入`enable_smp_cores`中。

```
void enable_smp_cores(paddr_t boot_flag)
{
        ...
        secondary_boot_flag = (long *)phys_to_virt(boot_flag);
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                ...
                secondary_boot_flag[i] = 1;
                flush_dcache_area((u64)secondary_boot_flag,
                                  (u64)sizeof(u64) * PLAT_CPU_NUM);
                ...
        }
        ...
}
```

`secondary_boot_flag`初始化值为{NOT_BSS, 0, 0, ...}，在`enable_smp_cores`中调用phys_to_virt将物理地址映射到虚拟地址，并进行赋值。


> 思考题 5：在`el0_syscall`调用`lock_kernel`时，在栈上保存了寄存器的值。这是为了避免调用`lock_kernel`时修改这些寄存器。在`unlock_kernel`时，是否需要将寄存器的值保存到栈中，试分析其原因。

不需要。因为unlock_kernel时，已经调用完了syscall，所以不需要将寄存器的值保存到栈中了。exception_exit


> 思考题 6：为何`idle_threads`不会加入到等待队列中？请分析其原因？

空闲进程`idle_threads`的作用是在没有线程就绪时顶位，防止CPU核心发现没有要调度的线程时在内核态忙等，导致持有的大内核锁锁住整个内核，其他CPU拿不到大内核锁，无法进入内核态。所以`idle_threads`不需要加入调度队列。

> 思考题 8：如果异常是从内核态捕获的，CPU核心不会在`kernel/arch/aarch64/irq/irq_entry.c`的`handle_irq`中获得大内核锁。但是，有一种特殊情况，即如果空闲线程（以内核态运行）中捕获了错误，则CPU核心还应该获取大内核锁。否则，内核可能会被永远阻塞。请思考一下原因。

如果空闲线程（以内核态运行）中捕获了错误，但不获取大内核锁，就会在`handle_irq`中调用`eret_to_thread(switch_context())`时放锁，调用`unlock_kernel`执行`lock->owner ++`，导致排号锁的`lock->owner`大于`lock->next`，造成死锁，内核被阻塞。
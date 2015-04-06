#gdb跟踪分析一个系统调用内核函数
**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

上次博客中，我曾经写过一个内联汇编，实现了一个write调用，实现打印字符串的功能，这次我们将使用gdb来完全跟踪分析一下整个系统调用过程。

####gdb跟踪Write系统调用
首先，删掉menu（rm menu -rf），然后把最新的git clone下来，然后修改菜单，把print和print_asm加进去，接着make roofs，过程如下图。
![3](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb/3.png)

![4](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb/4.png)

然后，我们启动gdb和qemu，按如下方式调用，加上断点，然后运行。我们通过查询系统调用表，我们可知道Write系统调用是4号，中断处理函数是sys_write,我们在这里加个断点，b sys_write,然后 c。如下图：
![6](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb/6.png)
我们可通过si指令，在这个函数单执，如下图:
![7](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb/7.png)

我们在qemu里面执行print_asm函数，我们发现它已经把sys_write执行完了。具体这个过程是怎么样的，我们下面将来看源代码来具体分析。

####详解Write系统
一：首先我们可以知道，系统调用也是个中断，中断需要初始化。所以，初始化是这里做的。1）首先，在http://codelab.shiyanlou.com/xref/linux-3.18.6/init/main.c里面的start_kernel里面调用trap_init()做初始化的。接着，在这个文件里，做绑定
http://codelab.shiyanlou.com/xref/linux-3.18.6/arch/x86/kernel/traps.c
把系统调用注册为相应的中断向量,如下：
``` C
8#ifdef CONFIG_X86_32
839	set_system_trap_gate(SYSCALL_VECTOR, &system_call);
840	set_bit(SYSCALL_VECTOR, used_vectors);
841#endif
```
这样，当发生int 0x80时，中断处理函数就会执行相应的中断处理函数，即为执行system_call。
二：当执行int 0x80时，会执行system_call，具体做了哪些呢，下面我们来具体看分析一下源代码。system_call函数在http://codelab.shiyanlou.com/xref/linux-3.18.6/arch/x86/kernel/entry_32.S这里，我们可以看看如下代码：
``` C
1syscall_call:
502	call *sys_call_table(,%eax,4)
503syscall_after_call:
504	movl %eax,PT_EAX(%esp)		# store the return value
505syscall_exit:
506	LOCKDEP_SYS_EXIT
507	DISABLE_INTERRUPTS(CLBR_ANY)	# make sure we don't miss an interrupt
508					# setting need_resched or sigpending
509					# between sampling and the iret
510	TRACE_IRQS_OFF
511	movl TI_flags(%ebp), %ecx
512	testl $_TIF_ALLWORK_MASK, %ecx	# current->work
513	jne syscall_exit_work
514
515restore_all:
516	TRACE_IRQS_IRET
```
我们可以发现了，当执行int 0x80过后，然后就跳到了system_call，然后首先是SAVE_ALL保存现场，然后根据相应的调用号执行相应的中断处理函数， 即call *sys_call_table(,%eax,4)。执行完成了以后，先返回值eax保存起来。然后它会先检查一下，看是否有其他的工作要做，比如处理系统信号或者进程切换，也就是这句话代码syscall_exit,如果有的话，就去处理信号，或者执行的进程切换。当这些都处理完成了以后，先restore_all 恢复现场，然后返回irq_return，系统调用过程返回。

####总结
经过以上，我们一个wirte系统调用的跟踪，和源代码的分析，我们已经大致了解了系统调用的具体过程和步骤，我们可以认为它就是个特殊的系统调用，那么执行int 0x80之后的流程图，我们可以简略的认为是下图。
![8](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb/8.png)
最后本次实验相关的源码，请参考[Menu](www.baidu.com)

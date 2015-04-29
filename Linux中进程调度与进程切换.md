#Linux进程调度与进程切换

**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

在这篇博客里，我们将分析在Linux中进程是如何调度和切换的。进程切换需要进行进程上下文进行调度，注意中断也会有中断上下文，这个跟进程上下文还有有一定区别的，中断上下文，还是在同一个进程，不过进程上下文就是在不同进程了。下面，我们先看一下源代码，分析一下流程。接着我们在做一下实验，验证一下确实是这样走的。最后我们重点来分析一下进程下文切换的代码。
####大概流程
通过前面几周的分析，我们知道调度都需要调用schedule()函数，在这个函数里面有很多不同的调度算法啊，进行调度选择，比如next = pick_next_task(rq, prev);就会根据某个调度算法来选择下一个准备调度到的进程。选择一个后，我们要调用这个函数context_switch进行上下文的相关切换，在这个函数里面，我们先调用prepare_task_switch进行任务切换的相关准备。接着，我们会调用switch_to函数，这个函数是进行寄存器状态和堆栈的切换，这个函数很重要，我们第一周的作业的作业切换就是模仿这段写的，后面，我们将会仔细分析这段代码。
####实验
下面，我们开始要gdb进行相关的调试。
首先，像往常一样，我们先make rootfs，然后打开gdb，进行调试。如下图：
![1](https://github.com/zbh24/LinuxCourseBlog/blob/master/eighth/1.png)
接着，我们在下面几个地方加几个断点，如下图。
![4](https://github.com/zbh24/LinuxCourseBlog/blob/master/eighth/2.png)
再接着，我们执行，一步步进行相关的函数，如下图。
![4](https://github.com/zbh24/LinuxCourseBlog/blob/master/eighth/3.png)
![4](https://github.com/zbh24/LinuxCourseBlog/blob/master/eighth/7.png)
![4](https://github.com/zbh24/LinuxCourseBlog/blob/master/eighth/8.png)
###switch_to代码分析
下面，我们就来分析一下switch_to这段内敛汇编代码。代码如下：
``` C
	asm volatile("pushfl\n\t"		/* save    flags */	\
43		     "pushl %%ebp\n\t"		/* save    EBP   */	\
44		     "movl %%esp,%[prev_sp]\n\t"	/* save    ESP   */ \
45		     "movl %[next_sp],%%esp\n\t"	/* restore ESP   */ \
46		     "movl $1f,%[prev_ip]\n\t"	/* save    EIP   */	\
47		     "pushl %[next_ip]\n\t"	/* restore EIP   */	\
48		     __switch_canary					\
49		     "jmp __switch_to\n"	/* regparm call  */	\
50		     "1:\t"						\
51		     "popl %%ebp\n\t"		/* restore EBP   */	\
52		     "popfl\n"			/* restore flags */	\
53									\
54		     /* output parameters */				\
55		     : [prev_sp] "=m" (prev->thread.sp),		\
56		       [prev_ip] "=m" (prev->thread.ip),		\
57		       "=a" (last),					\
58									\
59		       /* clobbered output registers: */		\
60		       "=b" (ebx), "=c" (ecx), "=d" (edx),		\
61		       "=S" (esi), "=D" (edi)				\
62		       							\
63		       __switch_canary_oparam				\
64									\
65		       /* input parameters: */				\
66		     : [next_sp]  "m" (next->thread.sp),		\
67		       [next_ip]  "m" (next->thread.ip),		\
68		       							\
69		       /* regparm parameters for __switch_to(): */	\
70		       [prev]     "a" (prev),				\
71		       [next]     "d" (next)				\
72									\
73		       __switch_canary_iparam				\
74									\
75		     : /* reloaded segment registers */			\
76			"memory");					\
```
首先保存当前进程的flags，ebp和栈指针esp到当前的栈里，接着切换到下一个进程的内核栈，保存当前的eip，因为这个当前进程当再次被调度到时，就会从这里开始执行。接着，恢复eip，然后eip就会指向标示1f这个位置，恢复ebp，恢复flags，总之这段代码和我们第一周所看的那段代码很相似，功能也差不多，就是进程切换。
####总结
进程调度和进程切换是现代操作系统中非常重要的一个概念，如果没有进程切换和调度，那么系统就会退化到单道处理系统。正因为有了进程切换，才大大提高了cpu的效率，减少了cpu的空闲时间，充分了挖掘了cpu的潜力，提高了计算机的吞吐率。

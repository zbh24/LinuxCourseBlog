#Fork函数对应的创建新进程的处理过程
**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

这篇博客将会详细分析在linux环境中一个新进程是如何创建的，在前面的练习中，我们分析了start_kernel中几个进程的创建，如何从无到创建一个进程，再由这个进程创建更多的进程，本文将从细节方面来叙述。
####进程的结构
首先，让我们来看一下进程的大致内容，它都描述了哪些内容。
``` C
truct task_struct {
1236	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
1237	void *stack;
1238	atomic_t usage;
1239	unsigned int flags;	/* per process flags, defined below */
1240	unsigned int ptrace;
....
....
```
我们可看一下linux源代码，里面描述进程结构体的代码大概有400多行，所以，我们这里就简要地总结一下，它大概描述了哪些内容。通过源码，我们可知，进程描述符大概有进程标识符（pid）,进程的状态，进程链表，进程的父子关系，进程的栈空间地址，打开的文件集合，以及等等。这些信息对进程的正常执行非常重要。
####gdb调试和fork函数的执行流程
我们知道，在linux里面，创建一个新进程是通过fork系统调用来实现的，那下面，我们就来看看，我们通过调试一个程序来具体分析一下。
首先，我的实验是在实验楼做的，我们通过分析fork调用来处理。我们在前面已经分析了系统调用了，大概知道了系统调用的具体流程。那么，下面将具体来跟踪分析fork调用，像以下截图一把内核跑起来。
![3](https://github.com/zbh24/LinuxCourseBlog/blob/master/fork/3.png)
我们知道系统调用保存现场以后，会执行相应的中断处理程序，而在linux中，fork对应的都是sys_clone,那么我们在这里加个断点，接着，我们看内核源码可知道，这个函数接着调用do_fork，再接着这里加个断点，在这个里面会有copy_process,这里加个断点，我们继续看源码，里面会调用dup_task_struct和copy_thread,我们都加个断点，最后，在我们中断返回时要执行的程序ret_from_fork加个断点，下面我们将逐步调试，看程序是不是这样执行的。如以下截图:
![5](https://github.com/zbh24/LinuxCourseBlog/blob/master/fork/5.png)
我们可以看到程序地的确是这样一步步执行的。
这里，我们可以看到fork是如何一次调用，两次返回的。父进程的eax的返回值是在eax中，而子进程的eax是置为0的，当中断返回时，把把这个值复制到eax中，同时子进程的内核内核堆栈也复制了父进程的内核堆栈，所以，把返回时，和父进程执行相同的代码段。通过一步步跟踪，我们发现，执行过程跟我们预期的一样，就是这样做的，等到调度到子进程时，子进程就可以执行。以下跟踪过程截图，最后可以一直跟踪ret_from_fork,直到返回用户态。
跟踪copy_thread直到ret_from_fork
![6](https://github.com/zbh24/LinuxCourseBlog/blob/master/fork/6.png)
![7](https://github.com/zbh24/LinuxCourseBlog/blob/master/fork/7.png)
####创建一个新进程的大体架构
我们现在总结一下，创建一个新进程的步骤，通过fork函数，这个函数有三个类似的系统调用，不管是哪个系统调用，都得调用do_fork，在这个函数里面，我们会进行进程块的设置，会调用copy_process，在这个里面，我们会调用dup_task_struct进行task_struct的复制，我们在copy_process里也会对task_struct进行相应的修改，我们接着会调用copy_thread，进行一些复制内核数据结构的复制，如寄存器的值等等。注意，就是在这里，我们指定了子进程的返回值和调度到子进程会执行的位置
``` C
....
160	childregs->ax = 0;
161	if (sp)
162		childregs->sp = sp;
164	p->thread.ip = (unsigned long) ret_from_fork;
```
所以，我们接着执行，当调度到子进程时，就会到ret_from_frok执行，就是进行一些恢复现场的工作，恢复好现就会返回到用户态正常执行。
####总结
通过以上的分析，我们知道了linux是如何创建一个新进程的了，一般都是通过fork，复制一个已有的进程，进而产生一个子进程。

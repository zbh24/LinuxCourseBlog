#Linux系统的学习心得
**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

这个学期，我们学习了Linux系统的内核。主要学习了进程调度，中断过程和系统调用。可以说，这些都是linux内核中的核心功能，正是由于这些功能的组成才组成了一个Liunx系统。下面，我就简单地回顾一下这几个核心功能。
####中断过程
中断可以说是系统中最重要的功能之一，如果没有中断，那么一个操作系统就将退化成一个单道处理系统。中断大概过程就是主动或者被动发起一个中断，一断系统允许中断，那么首先就会保存CS，SS，FLAGS，SS，SP，这些都由硬件来自动保存，接着进入到内核态。然后保存现场，即是保存通用寄存器，堆栈等等。然后，根据中断号来调用相应的中断处理程序，处理完成以后，恢复现场，中断返回。
####系统调用
系统调用就是特殊的中断过程，它一般由程序主动调用。当一个程序调用int ox80时，便会是一个系统调用，首先，通过中断进入内核太，然后根据向量号（保存在eax中）进行相应的分发。它的相应参数都保存在寄存器中如（ebx，edi，esi），最后，处理完成以后，也会恢复现场，中断返回。
####进程调度
最后，来说一下进程调度。关于进程调度，最重要的便是要理解进程是如何从一个进程切换到另一个进程的。从第一次上课时，我们便写了一个模仿县城切换到的例子，它包括保存当前的eip，esp，然后切换到下一个进程的eip和esp，这中间要保证正确，以便最后能切换回来，具体的代码分析可见我的博客。
####我的博客列表
[计算机程序的运行](https://github.com/zbh24/LinuxCourseBlog/blob/master/%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%A8%8B%E5%BA%8F%E7%9A%84%E8%BF%90%E8%A1%8C.MD)

[基于mykernel实现的时间片轮转调度代码](https://github.com/zbh24/LinuxCourseBlog/blob/master/%E5%9F%BA%E4%BA%8Emykernel%E5%AE%9E%E7%8E%B0%E7%9A%84%E6%97%B6%E9%97%B4%E7%89%87%E8%BD%AE%E8%BD%AC%E8%B0%83%E5%BA%A6%E4%BB%A3%E7%A0%81.md)

[使用gdb跟踪调试内核从start_kernel到init进程启动](https://github.com/zbh24/LinuxCourseBlog/blob/master/%E4%BD%BF%E7%94%A8gdb%E8%B7%9F%E8%B8%AA%E8%B0%83%E8%AF%95%E5%86%85%E6%A0%B8%E4%BB%8Estart_kernel%E5%88%B0init%E8%BF%9B%E7%A8%8B%E5%90%AF%E5%8A%A8.md)

[深入理解操作系统的系统调用](https://github.com/zbh24/LinuxCourseBlog/blob/master/%E6%B7%B1%E5%85%A5%E7%90%86%E8%A7%A3%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E7%9A%84%E7%B3%BB%E7%BB%9F%E8%B0%83%E7%94%A8.md)

[gdb跟踪分析一个系统调用内核函数](https://github.com/zbh24/LinuxCourseBlog/blob/master/gdb%E8%B7%9F%E8%B8%AA%E5%88%86%E6%9E%90%E4%B8%80%E4%B8%AA%E7%B3%BB%E7%BB%9F%E8%B0%83%E7%94%A8%E5%86%85%E6%A0%B8%E5%87%BD%E6%95%B0.md)

[Fork函数对应的创建新进程的处理过程](https://github.com/zbh24/LinuxCourseBlog/blob/master/Fork%E5%87%BD%E6%95%B0%E5%AF%B9%E5%BA%94%E7%9A%84%E5%88%9B%E5%BB%BA%E6%96%B0%E8%BF%9B%E7%A8%8B%E7%9A%84%E5%A4%84%E7%90%86%E8%BF%87%E7%A8%8B.md)

[Exec函数对应的系统调用处理过程](https://github.com/zbh24/LinuxCourseBlog/blob/master/Exec%E5%87%BD%E6%95%B0%E5%AF%B9%E5%BA%94%E7%9A%84%E7%B3%BB%E7%BB%9F%E8%B0%83%E7%94%A8%E5%A4%84%E7%90%86%E8%BF%87%E7%A8%8B.md)

[Linux进程调度与进程切换](https://github.com/zbh24/LinuxCourseBlog/blob/master/Linux%E4%B8%AD%E8%BF%9B%E7%A8%8B%E8%B0%83%E5%BA%A6%E4%B8%8E%E8%BF%9B%E7%A8%8B%E5%88%87%E6%8D%A2.md)

####最后
由于这门课，我对系统的理解更加深入地，明白了真个操作系统具体是怎么运作的，同时也学会了怎样看内核代码。
要说遗憾，我最大的遗憾可能就是没有详细地讲讲内存管理以及文件系统，希望能具体地了解一下内存分配，文件系统的实现。

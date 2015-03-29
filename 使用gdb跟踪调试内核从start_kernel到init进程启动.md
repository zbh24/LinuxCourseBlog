**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

在开始本文的调试过程之前，我们首先要学会使用gdb，那么首先我来介绍一下几个常用而且等会儿要用到的命令：
用gdb来跟踪记住，几个常用的命令：
- b:加断点，可加在函数或者行上
- s:单步进入
- n:单步跳过
- c:继续执行

关于，gdb还有其他常用的命令，具体可以去搜索了解一下。
###计算机的启动过程
计算机一旦加电以后，PC就指向BIOS的一段区域，这段区域会完成硬件自检，当硬件检查完成以后，没有发现问题。便开始从硬盘的一个扇区（大小为512个字节，在Linux中可以认为是grub）读取字节，然后把控制权交给这段代码，这段代码大小比较小，能做的事情比较少，能让用户进行一些选择操作。当选择完成以后，这样这段代码就负责加载内核到内存里了，加载完成以后，控制权就交给操作系统了，这样一个操作系统就加载并开始运行起来了。
###从start_kernel启动看
本文主要从Linux的内核init模块讲起，因为操作系统的一些设置基本初始化就是从这个模块开始的。其中，最先开始执行的更是，其中的main.c函数的start_kernel函数。那下面，我们就开始gdb来逐步跟踪看看。
按照说明，在start_kernel函数加了断点，然后执行c（continue），就运行到了start_kernel函数，如图
我们可以看到这个模块里面，有很多的初始化，如：
trap_init(中断初始化)，ipc_inti(进程通信初始化)，mm_init(内存管理初始化)，sched_init()(进程调度初始化)等等。最后，我们来看一下 rest_inti()函数。我们在这里加个断点，然后进去看看。
如下图。

![进程切换](https://github.com/zbh24/LinuxCourseBlog/blob/master/png/%E7%AC%AC%E4%B8%89%E6%AC%A1%E4%BD%9C%E4%B8%9A/1.png)

首先，我们发现了它又创建了一个新的进程：
``` C
kernel_thread(kernel_init, NULL, CLONE_FS);
```
kernel_init,根据我们上周的作业，这个对用的就是1号进程了。这是个内核进程，是用来做内核初始化的。接下去，继续看，又创建了一个2号进程，
``` C
kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
```
看名字，应该是与多线程有关的。
我们知道内核一开始运行，就有个0号进程，就是idle。如果没有其他进程，那么操作系统就会一直执行这个进程。这里是先执行调度，去处理一些事情，具体我们不用管，然后执行0号进程。
``` C
cpu_startup_entry(CPUHP_ONLINE);
```
如下图：

![进程切换](https://github.com/zbh24/LinuxCourseBlog/blob/master/png/%E7%AC%AC%E4%B8%89%E6%AC%A1%E4%BD%9C%E4%B8%9A/2.png)

然后执行cpu_idle_loop()函数，我们可以看到这是个循环，这样cpu就会不停地执行0号进程。

![进程切换](https://github.com/zbh24/LinuxCourseBlog/blob/master/png/%E7%AC%AC%E4%B8%89%E6%AC%A1%E4%BD%9C%E4%B8%9A/3.png)

####总结
现在，我们回头来，仔细看这个启动过程，我们来重点看一下0号进程（idle），和1号进程，是怎么来的。0号进程，是在这里初始化set_task_stack_end_magic(&init_task)
而1号进程则是在这里创建的kernel_thread(kernel_init, NULL, CLONE_FS)。
这篇文章就分析到这儿了。

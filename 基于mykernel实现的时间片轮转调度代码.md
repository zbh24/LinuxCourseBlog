#基于mykernel实现的时间片轮转调度代码
**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**
首先，我的环境使用的实验楼的环境，所有的配置已经部署好了。
下面，我就直接开始分析代码了。
这是一个提供了时钟中断(time_handler)的周期性执行my_time_handler中断处理程序。
###进程的设置
首先，开始多进程调度执行之前，首先，这个程序开始设置环境来模拟进程。
它定义了一个task[MAX_TASK_NUM]的进程数组。本身tPCB也是一个结构体，里面数据结构有关于进程的进程号（pid），进程的状态（state），进程的栈顶指针（thread.sp），以及指向下一个进程的指针（next）。具体描述如下：
``` C
typedef struct PCB{
    int pid;
    volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
    char stack[KERNEL_STACK_SIZE];
    /* CPU-specific state of this task */
    struct Thread thread;
    unsigned long	task_entry;
    struct PCB *next;
}tPCB;
```

###具体代码分析
首先，程序是从my_start_kernel开始执行的，它先创建一个0号进程，并设置相应的状态：赋予进程号，设置状态为可执行，设置代码的入口地址（即IP指向的地址），相应地也设置其他进程的这些参数，但是注意一下其它进程的状态的不可运行（-1），并相应地把这些进程组织为一个循环链表。具体为以下代码
``` C
	task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].task_entry = task[pid].thread.ip = (unsigned long)my_process;
    task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];
    task[pid].next = &task[pid];
    /*fork more process */
    for(i=1;i<MAX_TASK_NUM;i++)
    {
        memcpy(&task[i],&task[0],sizeof(tPCB));
        task[i].pid = i;
        task[i].state = -1;
        task[i].thread.sp = (unsigned long)&task[i].stack[KERNEL_STACK_SIZE-1];
        task[i].next = task[i-1].next;
        task[i-1].next = &task[i];
    }
```
接下来的代码，即为马上要第一个运行的0号进程进行设置，我们来看一下这段内联汇编。
``` C
	my_current_task = &task[pid];
	asm volatile(
    	"movl %1,%%esp\n\t" 	/* set task[pid].thread.sp to esp */
    	"pushl %1\n\t" 	        /* push ebp */
    	"pushl %0\n\t" 	        /* push task[pid].thread.ip */
    	"ret\n\t" 	            /* pop task[pid].thread.ip to eip */
    	"popl %%ebp\n\t"
    	: 
    	: "c" (task[pid].thread.ip),"d" (task[pid].thread.sp)	/* input c or d mean %ecx/%edx*/
	);
}   
```
首先，把esp寄存器指向0号进程的栈空间的栈顶，然后把ebp寄存器的值压入0号栈，这样就形成了一个函数运行栈的栈帧了。接着把0号进程的ip压栈，再接着ret，相当与popl %eip，这样就把eip寄存器指向了0号进程的ip值，即入口函数my_process()。（注意一下，最后一句popl %ebp没有执行。）下面，就开始0号进程的运行过程了，执行my_processs()函数。

###进程的切换
下面就会讲到本文的核心了：进程切换，由于my_process每隔10万次就会检查一下有没有进程调度（my_need_sched值是否等于1），而my_time_handler这个中断处理函数每隔1000次就会检查一下这个值，如果不为1，就把置为1，从而进行进程调度。当0号进程检查到my_need_sched等于1了，就会跳到my_schedule()函数，进行进程切换。
下面，我们来分析my_need_sched函数关于进程切换的内联汇编代码。
``` C
 if(next->state == 0)/* -1 unrunnable, 0 runnable, >0 stopped */
    {
    	/* switch to next process */
    	asm volatile(	
        	"pushl %%ebp\n\t" 	    /* save ebp */
        	"movl %%esp,%0\n\t" 	/* save esp */
        	"movl %2,%%esp\n\t"     /* restore  esp */
        	"movl $1f,%1\n\t"       /* save eip */	
        	"pushl %3\n\t" 
        	"ret\n\t" 	            /* restore  eip */
        	"1:\t"                  /* next process start here */
        	"popl %%ebp\n\t"
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	); 
    	my_current_task = next; 
    	printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);   	
    }
    else
    {
        next->state = 0;
        my_current_task = next;
        printk(KERN_NOTICE ">>>switch %d to %d<<<\n",prev->pid,next->pid);
    	/* switch to new process */
    	asm volatile(	
        	"pushl %%ebp\n\t" 	    /* save ebp */
        	"movl %%esp,%0\n\t" 	/* save esp */
        	"movl %2,%%esp\n\t"     /* restore  esp */
        	"movl %2,%%ebp\n\t"     /* restore  ebp */
        	"movl $1f,%1\n\t"       /* save eip */	
        	"pushl %3\n\t" 
        	"ret\n\t" 	            /* restore  eip */
        	: "=m" (prev->thread.sp),"=m" (prev->thread.ip)
        	: "m" (next->thread.sp),"m" (next->thread.ip)
    	);          
    }  
```
####所有进程第一次执行时
首先，它设置了两个变量，当前进程为prev，下一个进程为next。然后，判断下一个进程的状态是否为可运行（即是否为0），由于除了0号进程以外，其它进程都是第一次执行，因此它们的状态都为不可运行，即为-1，走else路径。

我们看看else路径，首先，它即将执行的1号进程状态置为运行状态，然后用内联汇编进行进程切换了。首先，它保存0号进程的ebp寄存器值到0号进程的栈里，然后把esp寄存器值保存0号进程的thread.ip里。接着，把esp寄存器值指向1号进程的esp值，即1号进程栈的栈顶，接着把ebp值也只想同样的位置。我们知道，当一个函数刚开始执行时（即完成了push ebp；mov esp ，ebp），esp和ebp是指向同样的位置的。接着，执行这句，mov $1f，%1,这是保存0号进程的eip的值，让再度切换到0号进程时可以继续执行，1f是个在if 分支里面的地址，等我们分析到if分支时，我们再详细说明，这里要明白这是保存了0号进程的ip值就好。 再接着，把1号进程的入口值压栈，再ret，弹出给eip，这样1号进行就开始执行了，执行my_process函数。
类似地，当1号进程去切换到2号进程时，过程也类似，直到所有进程都执行了一遍，从最后一个进程切换到0号进程时，情况不一样了。

####所有进程非第一次执行时
当最后一个进程切换到0号进程时，由于0号进程已经可以运行了，所以它走if分支。那我们来分析一下切换的内联汇编代码。
首先，把当前进程的ebp值压栈进行保存，然后把当前进程的esp值存入到对应的trhead.sp里面。再接着，开始切换栈了，把下一个进程也就是1号进程的esp值赋給esp寄存器，也就是让esp切换到0号进程的栈空间。再接着mov 1f，%1，即保存当前进程的eip值，而1f是这段内敛汇编最后一条指令（popl %ebp）的值，这样当再次切换回来时，第一条执行的语句就是pop ebp。再接着，把下一个进程也就是0号进程的ip值压栈，再ret，弹给EIP,这样就切换到下一个进程了，也就是切换回0号进程了，然后0号进程第一句就是popl %ebp。
以后，所有的进程切换都会执行if分支了，过程跟上面描述的类似。注意，这段代码，可能有个难点就是 （movl $1f,%1）1f是最后一条指令的地址，这句就是保存eip的地址，这样让所有进程切换回来的时候，第一句就执行popl %ebp。


###最后小结
mykernel模仿进程的切换大致过程就如上述所分析的，进程和进程之间的切换，重点是上下文的保存，esp和ebp的保存，以及当进程切换回来以后能回到正确的位置执行，所以eip也要正确保存，由于所有的进程都运行一个简单的my_process程序，所以，就不要保存和还原通用寄存器了。但是，这个mykernel还是能够很好地说明进程之间切换是怎么回事。
最后附上一张图，看看进程的切换。
完整的mykernel源代码见本站的mykernel。

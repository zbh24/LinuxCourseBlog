#Exec函数对应的系统调用处理过程
**张必红原创作品转载请注明出处《Linux内核分析》MOOC课程http://mooc.study.163.com/course/USTC-1000029000**

关于系统调用，前两周已经详细说明了，这次就主要讲讲exec函数对应的系统调用的不同之处。还有，exec对应的程序是怎样加载到内存里并且运行的。
####Exec对应的中断处理程序
在这里我们首先来说明一下，在linux里，一个程序是如何加载运行的。一般，我们是在shell里执行一条命令，然后就执行了一个程序。那么shell的原理是什么呢？一般shell，都是首先先fork一个子进程，可是，fork的子进程是和父进程执行相同的程序，这不符合我们的要求，所以就需要execlp函数，用子进程来加载执行我们想要执行的程序。这就是shell的基本原理。
那么下面呢，我们就来分析一下exec系统调用。首先，exec会调用sys_execve，然后呢，调用do_execve,再调用do_execve_common,这个函数会把函数参数和系统环境传进来进行相应的处理。然后调用exec_binprm来执行相应的程序。这个函数里面有很多重要的代码。其中，很重要的一点就是会调用search_binary_handler,这个函数会调用各种不同的格式来识别相应的文件，直到识别为止，比如linux中可执行文件为ELF，它就会识别出elf文件。我们可以分析一段这段代码.
``` C
1369	list_for_each_entry(fmt, &formats, lh) {
1370		if (!try_module_get(fmt->module))
1371			continue;
1372		read_unlock(&binfmt_lock);
1373		bprm->recursion_depth++;
1374		retval = fmt->load_binary(bprm);
1375		read_lock(&binfmt_lock);
1376		put_binfmt(fmt);
1377		bprm->recursion_depth--;
1378		if (retval < 0 && !bprm->mm) {
1379		/* we got to flush_old_exec() and failed after it*/
1380			read_unlock(&binfmt_lock);
1381			force_sigsegv(SIGSEGV, current);
1382			return retval;
1383		}
1384		if (retval != -ENOEXEC || !bprm->file) {
1385			read_unlock(&binfmt_lock);
1386			return retval;
1387		}
1388	}
```
首先，先找出对应的格式，直到找到为止。接着，找到以后，retval = fmt->load_binary(bprm)把对应的文件以对应的格式加载到内存里面（fmt->load_binary(bprm))，这就是这段代码的作用。然后，根据相应的格式，因为是linux，所以格式是elf，所以会执行对应的load_elf_binary,然后在这个函数里面有一个函数start_thread，这个函数会复制内核堆栈，同时会设置新的进程的执行位置，即会设置新的eip，把eip指向新程序的入口位置。
####Gdb调试程序
我们一边做实验一边来说明，首先，我们像往常一样开始调试，exec对应的系统调用的中断处理程序为sys_execve，我们在这里加个断点。如下图。
![10](https://github.com/zbh24/LinuxCourseBlog/blob/master/seventh/10.png)
然后，我们选择S，一步一步执行，会发现程序会sys_execve->do_execve -> do_execve_common ->  exec_binprm....
![14](https://github.com/zbh24/LinuxCourseBlog/blob/master/seventh/14.png)
到了这里注意，我们发现设置了新的eip，我们看一下这个新的eip是什么位置，我们选择po eip,同时 read -h hello，我们发现hello的入口位置就是这个eip的值。如图：
![16](https://github.com/zbh24/LinuxCourseBlog/blob/master/seventh/16.png)
最后，选择继续执行C，hello成功运行。
![17](https://github.com/zbh24/LinuxCourseBlog/blob/master/seventh/17.png)
####程序的静态和动态链接
程序有静态链接和动态链接，动态链接又分为装载时链接和运行时链接。
所谓静态链接呢，就是在链接时，就已经把库文件打包好到源程序了，这时候装载到内存里就可以直接运行了，不在需要做什么工作了。像对于sys_execve系统调用load_elf_interp指的就是程序的入口地址。装载时链接，就是在程序装入时内存时，进行链接，我们一般，最常使用的方式就是这种。而运行时链接是在运行时动态加载的，这种一般需要使用几个函数如，dlopen，dlsym等等。装载时链接等加载源程序结束后load_elf_interp接着把控制权交给了链接器ld。
####总结
本周，我们主要学习了execve系统调用，结合上周的fork系统调用，我们就可以成功地运行一个新程序了。简单地说，fork负责创建一个和父进程一样的子进程来运行，而execve负责把子进程来运行新的程序，这就是一个新程序执行的基本方法。

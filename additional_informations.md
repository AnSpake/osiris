# Welcome !

## Update: Feb 15th 7pm
The bug on the hanging recv is solved, you can test the project on your computer if you want
> make all
> cd io_uring_server
> ./io_uring_server 127.0.0.1 8000

in another terminal
> echo toto | netcat 127.0.0.1 8000

## Disclaimer
This is a ongoing project, it is not finished yet.  
This project needs at least a linux kernel 5.9 or later.  
Make install will actually install liburing on your computer, beware if you don't want this.  

## What is io_uring
I will try to resume this quickly.  
Its a new Async linux API, the main point is that you don't need "classic" syscalls thus no more context switches therefore
more performance and no more issues with the TLB.  
In reality this is one of the multiple mode of io_uring, there is the interruption and polling mode (it works closely like epoll), the mode we were talking about is called kernel mode.  
The point of this project is to do a benchmarking of the different modes available.

## Basis
io_uring uses a submission and a completions queue.  
You, as a user will create request that you will send to the submission queue. For example if I want to read something, I will have to prepare a "Read Request" to send to the submission queue. Then io_uring will get us the result of our read from the completion queue (a return value following the behavior of the read syscall).  
That seems a bit complex for a simple read, and that's where liburing comes to the rescue (kind of).  
Liburing is a library to use io_uring. They made lots of wrapper to helps us out !  
Even with liburing, the code looks "weird", I mean we are coding up to 5 lines juste to make a read (cf: io_uring_server/io_uring_server.c:submit_read), but trust me this is better than using io_uring directly ° u °"

As for the code itself, I've seen some magic about IORING_FEAT_FAST_POLL and the automatic buffer selection.  
Theses are boniis to gain some more performance.  
The automatic buffer selection can also resolve the issues of having a buffer per connections.

## More details
The kernel polling mode needs you to "register" every file descriptor with io_uring_register which is no fun, but a solution exists "IORING_FEAT_SQPOLL_NONFIXED".  
The thing is, its not actually used in the source code which probably means that its not available yet and thus the project is a bit stuck for now.

How does this work ? (the kernel polling mode) Well, io_uring will actually use 2 syscalls to set up everything and in one of them it will start a kernel thread.  
On this kernel thread, a submission and a completion queue will be shared.  
If no I/O operations are going on, the kernel thread will go idle and io_uring will use another of its syscall to wake it up again.  
Thats the only times where io_uring will use syscalls (thus context switches)

If you want to know more, the manpages documentation won't help much, but the source code (and its comments) is actually easier to read and understand than some part of the documentation.
(https://kernel.dk/io_uring.pdf this is the official documentation)

Since io_uring is quite new, there is a lot of new feature coming up often and its tight up to your kernel version.  
Make sure you're using a recent enough linux kernel in order to have the needed features

## Thanks !
In any case, thank you for reading all of this ~

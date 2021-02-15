# Osiris

## Disclaimer
This is a ongoing project. 
Benchmarking results are not out yet.  
This project uses latest master of https://github.com/axboe/liburing/commits/master
as of Feb 13, 2021, commits 5671af3

## io_uring benchmarking
This repository contains: 
- epoll server with Level and Edge trigger modes
- io_uring server with interrupt, poll and kernel polling modes
- benchmarking results (faio)

## Difficulties
- kernel polling: IORING_FEAT_SQPOLL_NONFIXED doesn't seems to be available yet
- prep_read is making io_uring_submit_and_wait blocking indefinitely in latest master branch

## io_uring additionnal informations
- Kernel polling mode will only cost 2 context switches for the server (and additionnal
context switches to wake up the kernel thread)
- Kernel polling mode only works with "registered file descriptor" unless you use the
IORING_FEAT_SQPOLL_NONFIXED
- Interrupt and polling modes can use the flag IORING_FEAT_FAST_POLL to gain
more performance (https://github.com/axboe/liburing/blob/master/man/io_uring_setup.2#L255)
- This io_uring server uses automatic buffer selection (https://lwn.net/Articles/815491/)

## TODO
- [ ] debug read (urgent)
- [ ] Kernel polling mode available
- [ ] io_uring server handling multiple clients (c++ comeback ?)
- [ ] benchmarking routines

### Authors
Amandine Nassiri <amandine.nassiri@epita.fr>  
https://github.com/AnSpake/neith

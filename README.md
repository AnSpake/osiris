# Osiris

## Disclaimer
This is a ongoing project. 
Benchmarking results are not out yet.  
This project uses latest master of https://github.com/axboe/liburing/commits/master
as of Feb 13, 2021, commits 5671af3
YOU NEED TO BE ON LINUX KERNEL 5.11 OR LATER.

## io_uring benchmarking
This repository contains: 
- epoll server with Level and Edge trigger modes
- io_uring server with interrupt, poll and kernel polling modes
- benchmarking results (faio)

## Difficulties
- kernel polling: IORING_FEAT_SQPOLL_NONFIXED doesn't seems to be available yet -> it's on linux kernel 5.10
- ~~prep_read is making io_uring_submit_and_wait blocking indefinitely in latest master branch~~ (solved)

## io_uring additionnal informations
- Kernel polling mode will only cost 2 context switches for the server (and additionnal
context switches to wake up the kernel thread)
- Kernel polling mode only works with "registered file descriptor" unless you use the
IORING_FEAT_SQPOLL_NONFIXED
- Interrupt and polling modes can use the flag IORING_FEAT_FAST_POLL to gain
more performance (https://github.com/axboe/liburing/blob/master/man/io_uring_setup.2#L255)
- This io_uring server uses automatic buffer selection (https://lwn.net/Articles/815491/)
- IORING_FEAT_SINGLE_MMAP enable the SQ and CQ rings to be mapped in one mmap call,
the SQEs must still be allocated separately.
This brings the necessary mmap calls down from 3 to 2.

## TODO
- [x] debug read (urgent)
- [ ] Kernel polling mode available (register fds for now)
- [ ] io_uring server handling multiple clients (c++ comeback ?) (need research as we might not need it thanks to Automatic buffer selection)
- [ ] benchmarking routines
- [ ] Kernel polling mode with FEAT_SQPOLL_NONFIXED flag when it's available

### Authors
Amandine Nassiri <amandine.nassiri@epita.fr>  
https://github.com/AnSpake/osiris

all: install io_uring_srv epoll_srv benchmark

bin: io_uring_srv epoll_srv

install:
	@echo "Cloning axboe/liburing/5671af3..."
	git clone https://github.com/axboe/liburing.git
	cd liburing && git checkout -b dev-liburing 5671af3
	@echo "Installing liburing"
	make -C liburing
	sudo make install -C liburing

io_uring_srv:
	make -C io_uring_server
	@echo "Setting CAP_SYS_NICE capabilities to io_uring_server_sqpoll"
	sudo cap_sys_nice=eip io_uring_server/io_uring_server_sqpoll

epoll_srv:
	make -C epoll_server

benchmark:
	@echo "Cloning haraldh/rust_echo_bench/920f8a5"
	git clone https://github.com/haraldh/rust_echo_bench.git
	cd rust_echo_bench && git checkout -b dev 920f8a5
	@echo "Launching the benchmarking"
	cp benchmarking.sh rust_echo_bench/.
	cd rust_echo_bench && ./benchmarking.sh

clean:
	make clean -C io_uring_server
	make clean -C epoll_server
	rm -rf liburing rust_echo_bench

.PHONY: clean benchmark install all

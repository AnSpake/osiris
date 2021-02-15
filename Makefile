all: install io_uring epoll benchmark

install:
	echo "Cloning axboe/liburing/5671af3..."
	git clone https://github.com/axboe/liburing.git
	cd liburing && git checkout -b dev-liburing 5671af3
	echo "Installing liburing"
	make -C liburing
	sudo make install -C liburing

io_uring_srv:
	make -C io_uring_server

epoll_srv:
	make -C epoll_server

benchmark:
	echo "Launching the benchmark"
	echo "Not available yet..."

clean:
	make clean -C io_uring_server
	make clean -C epoll_server
	rm -rf liburing

.PHONY: clean benchmark install all

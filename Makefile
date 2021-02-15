all: install io_uring epoll benchmark

install:
	echo "Cloning axboe/liburing/5671af3..."
	git clone https://github.com/axboe/liburing.git
	cd liburing && git checkout -b dev-liburing 5671af3
	echo "Installing liburing"
	make -C liburing
	sudo make install -C liburing

io_uring:
	make -C io_uring

epoll:
	make -C epoll

benchmark:
	echo "Launching the benchmark"
	echo "Not available yet..."

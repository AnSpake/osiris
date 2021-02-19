#!/bin/bash

IP="127.0.0.1"
PORT="8000"

benchmark()
{
    BIN="$1"

    for bytes in 1 128 512
    do
        for connections in 1 50 150
        do
            # Start up the echo server
            ./"$BIN" "$IP" "$PORT" &
            PID=$(lsof -itcp:"$PORT" | sed -n -e 2p | awk '{print $2}')
            taskset -cp 0 $PID

            echo "Context switches BEFORE test with $connections connections and $bytes bytes"
            grep ctxt /proc/"$PID"/status
            cargo run --release -- --address "$IP:$PORT" --number $connections --duration 60 --length $bytes

            echo "Context switches AFTER test with $connections connections and $bytes bytes"
            grep ctxt /proc/"$PID"/status

            # Reset the echo server
            kill "$PID"
            sleep 5
        done
    done
}

# 1: epoll_level_triggered
echo "Benchmarking epoll in level triggered mode"
benchmark "epoll_server/epoll_server_lt"

# 2: epoll_edge_triggered
echo "Benchmarking epoll in edge triggered mode"
benchmark "epoll_server/epoll_server_et"

# 3: io_uring interrupt driven
echo "Benchmarking io_uring in interrupt driven mode"
benchmark "io_uring_server/io_uring_server"

# 4: io_uring kernel polling
echo "Benchmarking io_uring in kernel polling mode"
benchmark "io_uring_server/io_uring_server_sqpoll"

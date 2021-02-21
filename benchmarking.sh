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
            sleep 5
            PID=$(sudo lsof -itcp:"$PORT" | sed -n -e 2p | awk '{print $2}')
            sudo taskset -cp 0 $PID
            echo "Started $BIN, pid: $PID"

            echo "Context switches BEFORE test with $connections connections and $bytes bytes"
            grep ctxt /proc/"$PID"/status
            cargo run --release -- --address "$IP:$PORT" --number $connections --duration 60 --length $bytes

            echo "Context switches AFTER test with $connections connections and $bytes bytes"
            grep ctxt /proc/"$PID"/status
            echo -e "=======================================================================================================\n"

            # Reset the echo server
            sudo kill "$PID"
            sleep 10
        done
    done
}

ROOT_PATH=".."
pkill epoll_server
pkill io_uring_server

# 1: epoll_level_triggered
echo "---- Benchmarking epoll in level triggered mode ----"
benchmark "$ROOT_PATH/epoll_server/epoll_server_lt"

# 2: epoll_edge_triggered
echo "---- Benchmarking epoll in edge triggered mode ----"
benchmark "$ROOT_PATH/epoll_server/epoll_server_et"

# 3: io_uring interrupt driven
echo "---- Benchmarking io_uring in interrupt driven mode ----"
benchmark "$ROOT_PATH/io_uring_server/io_uring_server"

# 4: io_uring kernel polling
echo "---- Benchmarking io_uring in kernel polling mode ----"
sudo setcap cap_sys_nice=eip "$ROOT_PATH/io_uring_server/io_uring_server_sqpoll"
benchmark "$ROOT_PATH/io_uring_server/io_uring_server_sqpoll"

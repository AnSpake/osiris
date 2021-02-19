#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: ./benchmark.sh IP PORT"
    exit 1
fi

IP="$1"
PORT="$2"

PID=$(lsof -itcp:"$PORT" | sed -n -e 2p | awk '{print $2}')
taskset -cp 0 $PID

for bytes in 1 128 512
do
    for connections in 1 50 150
    do
        echo "Context switches BEFORE test with $connections connections and $bytes bytes"
        grep ctxt /proc/"$PID"/status
        cargo run --release -- --address "$IP:$PORT" --number $connections --duration 60 --length $bytes

        echo "Context switches AFTER test with $connections connections and $bytes bytes"
        grep ctxt /proc/"$PID"/status
        sleep 5
    done
done

#!/bin/bash

### This script is used to init n workers in background

exe_dir=/home/nikola/projects/Worker/bin/x64/Debug

for id in $(seq 1 $(expr $1 - 1))
do
    $exe_dir/Worker.out $1 $id &    # Run worker in background and pass arguments
    echo "Worker $id started with PID $!"
done
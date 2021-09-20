#!/bin/bash

### This script is used to init n workers in background
### First arg is numbe of workers
### Second arg means which worker NOT to initialize

exe_dir=~/projects/Worker/bin/x64/Debug

for id in $(seq 0 $(($1-1)) )
do
     if [[ $id != $2 ]]
     then
         $exe_dir/Worker.out $1 $id $3 &    # Run worker in background and pass arguments
         echo "Worker $id started with PID $!"
    fi
done

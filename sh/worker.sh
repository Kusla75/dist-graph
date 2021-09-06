#!/bin/bash

### This script is automates Worker.out execution
### It checks if machine architecture is ARM or x86_64

machine_info=$(uname -a)
exe_dir=/home/nikola/projects/Worker/bin/

if [[ $machine_info =~ "arm" ]]
then
    $exe_dir/ARM/Debug/Worker.out $1 $2

elif [[ $machine_info =~ "x86_64" ]]
then
    $exe_dir/x64/Debug/Worker.out $1 $2

else
    echo ""
fi
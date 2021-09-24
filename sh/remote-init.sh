#!/bin/bash

# 192.168.0.21 - Main PC
# 192.168.0.11 - Laptop
# 192.168.0.26 - Raspberry
# 192.168.0.10 - Phone

sshpass_cmd="sshpass -p nikola123"
exe_cmd="sh/worker.sh 3"

data_directory=socfb-caltech/N3_K1_rand/ # <-------

$exe_cmd 0 $data_directory &
$sshpass_cmd ssh nikola@192.168.0.11 $exe_cmd 1 $data_directory &
$sshpass_cmd ssh nikola@192.168.0.26 $exe_cmd 2 $data_directory &

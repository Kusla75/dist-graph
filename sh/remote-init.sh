#!/bin/bash

### Initializes workers remotly using ssh

# 192.168.0.21 - Main PC
# 192.168.0.22 - Laptop
# 192.168.0.23 - Raspberry
# 192.168.0.24 - Phone

sshpass_cmd="sshpass -p nikola123"
num_workers="4" # <------- CHANGE

data_directory=fb-pages-food/N4_K2_rand/ # <------- CHANGE

# Local machine init
sh/worker.sh $num_workers 0 $data_directory &
# Remote machine init
$sshpass_cmd ssh nikola@192.168.0.22 sh/worker.sh $num_workers 1 $data_directory &
$sshpass_cmd ssh nikola@192.168.0.23 sh/worker.sh $num_workers 2 $data_directory &
$sshpass_cmd ssh nikola@192.168.0.24 -p 8022 sh/worker.sh $num_workers 3 $data_directory &

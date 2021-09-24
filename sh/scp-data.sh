#!/bin/bash

### This script is used to copy necessary repo files
### to other remote machines

dir=~/
ip_file=/home/nikola/ip_addrs.txt
sshpass_cmd="sshpass -p nikola123"

while read -r ip
do
    $sshpass_cmd scp -r -q $dir/projects $dir/partitions/ $dir/py/ $dir/sh/ $dir/solutions/ $ip_file nikola@$ip:$dir/

done < $ip_file

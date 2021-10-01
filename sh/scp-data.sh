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

#$sshpass_cmd scp -P 8022 -r -q partitions/ sh/ py/ solutions/ ip_addrs.txt u0_a310@192.168.0.10:/data/data/com.termux/files/home

# $sshpass_cmd scp -r -q $dir/projects $dir/partitions/ $dir/py/ $dir/sh/ $dir/solutions/ $ip_file nikola@192.168.0.17:$dir/
# $sshpass_cmd scp -r -q $dir/projects $dir/partitions/ $dir/py/ $dir/sh/ $dir/solutions/ $ip_file nikola@192.168.0.26:$dir/
# $sshpass_cmd scp -P 8022 -r -q partitions/ sh/ py/ solutions/ ip_addrs.txt u0_a310@192.168.0.10:/data/data/com.termux/files/home

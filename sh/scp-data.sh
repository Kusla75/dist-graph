#!/bin/bash

### This script is used to copy necessary repo files
### to other remote machines

dir=/home/nikola
ip_file=/home/nikola/ip_addrs.txt

while read -r ip
do
    scp -r -q $dir/partitions/ $dir/py/ $dir/sh/ $dir/solutions/ $ip_file nikola@$ip:$dir/

done < $ip_file
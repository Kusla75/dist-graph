#!/bin/bash

### This script is used to copy partitions for workers, shell scripts, python scripts and other data

repo=/mnt/c/Users/Nikola/Desktop/dist-graph
dest=~/

src=$repo/partitions
cp -rf $src $dest

src=$repo/sh
cp -rf $src $dest

src=$repo/py
cp -rf $src $dest

src=$repo/solutions
cp -rf $src $dest

cp -rf $repo/ip_addrs.txt $dest

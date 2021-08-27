#!/bin/bash

### This script is used to copy partitions for workers, shell and python scripts

repo=/mnt/c/Users/Nikola/Desktop/dist-graph
dest=/home/nikola/

src=$repo/partitions
cp -rf $src $dest

src=$repo/sh
cp -rf $src $dest

src=$repo/py
cp -rf $src $dest

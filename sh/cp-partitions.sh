#!/bin/bash

### This script is used to copy partitions for workers and shell scripts

repo=/mnt/c/Users/Nikola/Desktop/dist-graph

src=$repo/partitions
dest=/home/nikola/

cp -rf $src $dest

src=$repo/sh

cp -rf $src $dest

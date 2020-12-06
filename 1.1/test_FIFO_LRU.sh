#!/bin/bash

gcc -std=c99 vm.c -o vm.out

# 使用FIFO策略
echo "FIFO Strategy Result:"
./vm.out -f BACKING_STORE.bin addresses.txt | tail -n 1


echo "LRU Strategy Result:"
# 使用LRU策略
./vm.out -l BACKING_STORE.bin addresses.txt | tail -n 1



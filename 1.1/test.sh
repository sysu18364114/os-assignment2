#!/bin/bash

echo "Compiling..."
gcc vm.c -o vm -std=c99 # 以C99标准编译文件

echo "Running vm: mode LRU"
./vm -l BACKING_STORE.bin addresses.txt 1>out.txt 2>>out.txt # 运行编译后的可执行文件，并重定向输出到文件out.txt
echo "Comparing with correct.txt"
diff out.txt correct.txt # 将输出与标准文件correct.txt比较



#!/bin/bash

echo "Compling..."
gcc -xc -std=c99 vm.c -o vm.o

echo "Mode: FIFO Memory: Locality"
./vm.o -f BACKING_STORE.bin addresses-locality.txt 1>FIFO_out_addresses-locality.txt
tail -n 1 FIFO_out_addresses-locality.txt

echo "Mode: FIFO Memory: Random"
./vm.o -f BACKING_STORE.bin addresses-random.txt 1>FIFO_out_addresses-random.txt
tail -n 1 FIFO_out_addresses-random.txt

echo "Mode: LRU Memory: Locality"
./vm.o -l BACKING_STORE.bin addresses-locality.txt 1>LRU_out_addresses-locality.txt
tail -n 1 LRU_out_addresses-locality.txt

echo "Mode: LRU Memory: Random"
./vm.o -l BACKING_STORE.bin addresses-random.txt 1>LRU_out_addresses-random.txt
tail -n 1 LRU_out_addresses-random.txt

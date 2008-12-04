#!/bin/bash
export TABLE_LIMIT=2
gcc -lpthread -g -Wall binarr.c shmem.c socket.c washer.c -o washer
gcc -lpthread -g -Wall binarr.c shmem.c socket.c dryer.c -o dryer

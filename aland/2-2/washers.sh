#!/bin/bash
gcc -lpthread -g -Wall binarr.c shmem.c socket.c washer.c -o washer
gcc -lpthread -g -Wall binarr.c shmem.c socket.c dryer.c -o dryer

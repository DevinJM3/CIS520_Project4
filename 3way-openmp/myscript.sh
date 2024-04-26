#!/bin/sh
gcc -o g1.out -fopenmp main.c
OMP_NUM_THREADS=40 ./g1.out

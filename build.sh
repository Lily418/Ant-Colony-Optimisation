#!/usr/bin/env bash
g++ -I . -std=c++0x -c main.cpp mongoose/mongoose.c
nvcc -I /bham/pd/packages/SL6/x86_64/cuda-4.0.17-sdk/C/common/inc/ -c gpu.cu
nvcc gpu.o main.o mongoose.o


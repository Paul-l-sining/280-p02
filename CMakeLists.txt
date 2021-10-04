cmake_minimum_required(VERSION 3.20)
project(p02 C)

set(CMAKE_C_STANDARD 99)

add_executable(p02 main.c simplified_des.c simplified_des.h sdes-encrypt.c sdes-decrypt.c test-sdes.c mut.h)

cmake_minimum_required(VERSION 3.20)

project(src)

set(CMAKE_C_STANDARD 17)

add_executable(src  process_generator.c test_generator.c clk.c scheduler.c process.c Data_Structures/linked_list.c Data_Structures/pqueue.c)

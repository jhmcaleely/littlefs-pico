cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(host-uf2 C CXX ASM)

add_executable(host-uf2 main.c block_device.c bdfs_lfs_hal.c ../littlefs/lfs.c ../littlefs/lfs_util.c)
add_compile_definitions(LFS_YES_TRACE)
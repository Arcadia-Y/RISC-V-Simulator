cmake_minimum_required(VERSION 3.16)

project (RISC-V-Simulator)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")

include_directories(include)
aux_source_directory(src DIR_SRCS)
add_executable(code ${DIR_SRCS})

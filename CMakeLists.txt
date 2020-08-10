cmake_minimum_required(VERSION 3.5)
project(rdma-nnmsg)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Debug)

# Organize built files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include and link libraries
include_directories(.)
include_directories(./nanomsg-1.1.4/include)
link_directories(./nanomsg-1.1.4/lib)
link_libraries(ibverbs nanomsg pthread)

# Configuration + Scripts files
configure_file(ifconfig.txt ifconfig.txt COPYONLY)
configure_file(run_resilient_db.py run_resilient_db.py COPYONLY)

# Build executable
add_executable(trans trans.cpp)

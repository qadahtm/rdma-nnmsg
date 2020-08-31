# rdma-nnmsg
Nanomsg and ibverbs based implementation of RDMA for Distributed Communication  

Specify NODE_CNT in rsupport.cpp
Implement desired functionalities in trans.cpp

# Requirements:

nanomsg, ibverbs libraries installed

# Compilation and Execution:

1. Install nanomsg and ibverbs library
2. Create ifconfig.txt file with all the IP addresses

g++ trans.cpp -o trans -pthread -lnanomsg -libverbs && 
./trans <NODE_ID>

# Features
1. Supports RDMA_Send and RDMA_Recv
2. Supports RDMA_Remote_Write and RDMA_Remote_Reads
3. Now supports Multi Memory Registrations and tested with Multi-threading

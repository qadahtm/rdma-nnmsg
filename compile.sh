#!/bin/bas

g++ -g trans.cpp -o trans -lnanomsg -libverbs -pthread
scp trans shubhamp@pronghorn.rc.unr.edu:~/sockets/

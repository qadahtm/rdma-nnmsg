#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <getopt.h>
#include <infiniband/verbs.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

char* buf_print(char* buf){
    return buf;
}

int main(){
    char* buf, buf2;
    buf = (char *)malloc(256);
    cout << "Previously: "<< buf_print(buf) << endl;
    memset(buf, '*', 5);
    cout << "Buffer: " << buf_print(buf) << endl;
    return 0;
}
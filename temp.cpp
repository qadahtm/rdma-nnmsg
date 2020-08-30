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
    char **buf;
    buf = (char **)malloc(10 * sizeof(char *));
    for(int i = 0; i< 10; i++){
        buf[i] = (char *)malloc(256);
    }
    for(int i = 0; i< 10; i++){
        snprintf(buf[i], 256,"This is message: %d", i);
    }
    for(int i = 0; i< 10; i++){
        cout << buf[i] << endl;
    }
    return 0;
}
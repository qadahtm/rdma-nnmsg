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
    char msg[256];
    char* buf, buf2;
    buf = (char *)malloc(256);
    memset(buf, '-1', 5);
    if(buf[0] == '-1'){
        cout << "Is -1" << endl;
        cout << sizeof(buf) << endl;
    }
    cout << "Previously: "<< buf_print(buf) << endl;
    memset(buf, '*', 52);
    strcpy(msg, buf);
    cout << sizeof(msg) << " " << msg << endl;
    return 0;
}
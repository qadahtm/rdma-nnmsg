#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include <vector>
#include <set>
#include <iterator>
#include <string.h>

using namespace std;

int ports[5] = {17000,17001,17002,17003,17004};

int node(const int argc, const char **argv)
{   
    set <std::string> setofids, ended;
    int sock = nn_socket(AF_SP, NN_BUS);
    char myurl[22];
    int id = atoi(argv[1]);
    snprintf(myurl, 22,"tcp://127.0.0.1:%d",ports[id]);
    assert(sock >= 0);
    assert(nn_bind(sock, myurl) >= 0);
    for(int i = 0;i < 5; i++){
        if(id == i){
            continue;
        }
        char url[22];
        snprintf(url, 22,"tcp://127.0.0.1:%d",ports[i]);
        assert(nn_connect(sock, url) >= 0);
    }
    int to = 100,count = 0;
    assert(nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
    int sz_n = strlen(argv[1]) + 1;
    printf("%s: SENDING '%s' ONTO BUS\n", argv[1], argv[1]);
    // int send = nn_send(sock, argv[1], sz_n, 0);
    // assert(send == sz_n);
    printf("Sent\n");
    while (setofids.size() < 4 && ended.size() <= 4)
    {
        char *buf = NULL;
        int recv = nn_recv(sock, &buf, NN_MSG, 0);
        if (recv >= 0 && buf != "END")
        {   
            setofids.insert(buf);
            nn_freemsg(buf);
        }
        else if(recv >= 0 && buf == "END"){
            ended.insert("END");
        }
        if(setofids.size()< 4){
            int send = nn_send(sock, argv[1], sz_n, 0);
            assert(send == sz_n);
        }
        else{
            char *bufs = strcat((char *)argv[1], "END");
            int send = nn_send(sock, bufs, sizeof("END"), 0);
            assert(send == sz_n);
        }
    }
    for (std::set<std::string>::iterator it=setofids.begin(); it!=setofids.end(); ++it)
        std::cout << ' ' << *it;
    return nn_shutdown(sock, 0);
}

int main(const int argc, const char **argv)
{
    if (argc >= 1)
        node(argc, argv);
    else
    {
        fprintf(stderr, "Usage: bus <node-id>...\n");
        return 1;
    }
}
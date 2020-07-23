#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include <vector>
#include <set>
#include <iterator>
#include <string.h>
#include <unistd.h>

using namespace std;

int ports[5] = {17000,17001,17002,17003,17004};

int node(const int argc, const char **argv)
{   
    set <std::string> setofids, acks;

    char myurl_o[22], myurl_e[22];
    int id = atoi(argv[1]);
    int to = 100,count = 0;
    snprintf(myurl_o, 22,"tcp://127.0.0.1:%d",ports[id]);
    snprintf(myurl_e, 22,"tcp://127.0.0.1:%d",10 + ports[id]);

    int sock_odd = nn_socket(AF_SP, NN_BUS);
    assert(sock_odd >= 0);
    int sock_even = nn_socket(AF_SP, NN_BUS);
    assert(sock_even >= 0);
    assert(nn_bind(sock_odd, myurl_o) >= 0);
    assert(nn_bind(sock_even, myurl_e) >= 0);

    assert(nn_setsockopt(sock_odd, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
    assert(nn_setsockopt(sock_odd, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to)) >= 0);
    assert(nn_setsockopt(sock_even, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
    assert(nn_setsockopt(sock_even, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to)) >= 0);

    sleep(10);
    for(int i = 0;i < 5; i++){
        if(id == i){
            continue;
        }
        char url[22];
        if(i % 2 == 0){
            snprintf(url, 22,"tcp://127.0.0.1:%d",10 + ports[i]);
            assert(nn_connect(sock_odd, url) >= 0);
        }
        else{
            snprintf(url, 22,"tcp://127.0.0.1:%d",ports[i]);
            assert(nn_connect(sock_even, url) >= 0);
        }
    }
    sleep(10);
    int sz_n = strlen(argv[1]) + 1;
    printf("%s: SENDING '%s' ONTO BUS\n", argv[1], argv[1]);
    // int send = nn_send(sock, argv[1], sz_n, 0);
    // assert(send == sz_n);
    printf("Sent\n");
    while (count < 500)
    {
        char *buf_o = NULL;
        char *buf_e = NULL;

        int recv = nn_recv(sock_odd, &buf_o, NN_MSG, 0);
        if (recv >= 0)
        {   
            setofids.insert(buf_o);
            sleep(0.5);
            nn_freemsg(buf_o);
        }
        int send = nn_send(sock_odd, argv[1], sz_n, 0);
        assert(send == sz_n);

        recv = nn_recv(sock_even, &buf_e, NN_MSG, 0);
        if (recv >= 0)
        {   
            setofids.insert(buf_e);
            sleep(0.5);
            nn_freemsg(buf_e);
        }
        send = nn_send(sock_even, argv[1], sz_n, 0);
        assert(send == sz_n);
        if(setofids.size() > 3){
            if(count == 0) 
                cout << "Recieved all: " << setofids.size() << endl;
            count++;
            // cout << count << endl;
        }
    }
    for (std::set<std::string>::iterator it=setofids.begin(); it!=setofids.end(); ++it)
        std::cout << ' ' << *it;
    nn_shutdown(sock_odd, 0);
    nn_shutdown(sock_even, 0);
    return 1;
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
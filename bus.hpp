#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <vector>
#include <set>
#include <iterator>
#include <string.h>
#include <unistd.h>
#include "rsupport.hpp"
#include <fstream>
#include <thread>
#include <bits/stl_map.h>

using namespace std;
#define sz_n sizeof(stag)

char **ifaddr;
std::map<int, int> send_sockets, recv_sockets;
std::map<int, stag> req_stags, resp_stags;

uint64_t get_port_id(uint64_t src_node_id, uint64_t dest_node_id)
{
    uint64_t port_id = 17000;
    port_id += NODE_CNT * dest_node_id;
    port_id += src_node_id;
    return port_id;
}
void read_ifconfig(const char *ifaddr_file)
{
    ifaddr = new char *[NODE_CNT];

    uint64_t cnt = 0;
    printf("Reading ifconfig file: %s\n", ifaddr_file);
    ifstream fin(ifaddr_file);
    string line;
    while (getline(fin, line))
    {
        //memcpy(ifaddr[cnt],&line[0],12);
        ifaddr[cnt] = new char[line.length() + 1];
        strcpy(ifaddr[cnt], &line[0]);
        printf("%ld: %s\n", cnt, ifaddr[cnt]);
        cnt++;
    }
    assert(cnt == NODE_CNT);
}

void recv_thread(int id)
{
    cout << "In recv" << endl;
    int count = 0, i = 0;
    set<int> counter;
    while (count <= 4)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > 4)
        {
            i = 0;
            continue;
        }
        int recv = nn_recv(recv_sockets[i], &resp_area_stag[i], sz_n + 1, 0);
        if (recv > 1)
        {
            count++;
            counter.insert(i);
            cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == 4)
        {
            break;
        }
    }
    cout << "Rec: res_area_stags" << endl;
    count = 0;
    i = 0;
    counter.clear();
    while (count <= 4)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > 4)
        {
            i = 0;
            continue;
        }
        int recv = nn_recv(recv_sockets[i], &req_area_stag[i], sz_n + 1, 0);
        if (recv > 1)
        {
            count++;
            counter.insert(i);
            cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == 4)
        {
            break;
        }
    }
    cout << "Rec: req_area_stags" << endl;
    while (count <= 4)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > 4)
        {
            i = 0;
            continue;
        }
        int recv = nn_recv(recv_sockets[i], &req_area_stag[i], sz_n + 1, 0);
        if (recv > 1)
        {
            count++;
            counter.insert(i);
            cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == 4)
        {
            break;
        }
    }
}

void send_thread(int id)
{
    cout << "In Send" << endl;
    int i = 0, count = 0;
    set<int> counter;
    while (count <= 4)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > 4)
        {
            i = 0;
            continue;
        }
        int send = nn_send(send_sockets[i], &resp_area_stag[id], sz_n + 1, 0);
        if (send > 0)
        {
            cout << "Sent to " << i << endl;
            counter.insert(i);
            count++;
        }
        i++;
        if (counter.size() == 4)
        {
            break;
        }
    }
    count = 0;
    i = 0;
    counter.clear();
    while (count <= 4)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > 4)
        {
            i = 0;
            continue;
        }
        int send = nn_send(send_sockets[i], &req_area_stag[id], sz_n + 1, 0);
        if (send > 1)
        {
            cout << "Sent to" << i << endl;
            counter.insert(i);
            count++;
        }
        i++;
        if (counter.size() == 4)
        {
            break;
        }
    }
    cout << "Done Send" << endl;
}

int node(const int argc, const char **argv)
{
    int id = atoi(argv[1]), i;
    char myurl[22], url[22];
    int to = 100, count = 0;

    resp_area_stag[id].rkey = resp_area_mr[id]->rkey;
    resp_area_stag[id].size = 256 * KB;
    resp_area_stag[id].id = id;

    req_area_stag[id].id = id;
    req_area_stag[id].rkey = req_area_mr[id]->rkey;
    req_area_stag[id].size = 256 * KB;

    // char my_serialized_stag[sizeof(struct stag)+1];
    // serialize(node_stags[id], my_serialized_stag);

    for (i = 0; i < NODE_CNT; i++)
    {
        if (i == id)
            continue;
        int temp_sock = nn_socket(AF_SP, NN_PAIR);
        uint64_t p = get_port_id(id, i);
        snprintf(url, 22, "tcp://%s:%ld", ifaddr[i], p);
        nn_bind(temp_sock, url);
        cout << "Connected to :" << url << endl;
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to)) >= 0);
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
        send_sockets.insert(make_pair(i, temp_sock));
        // cout << send_sockets.size() <<endl;
    }

    for (i = 0; i < NODE_CNT; i++)
    {
        if (i == id)
            continue;
        int temp_sock = nn_socket(AF_SP, NN_PAIR);
        uint64_t p = get_port_id(i, id);
        snprintf(myurl, 22, "tcp://%s:%ld", ifaddr[id], p);
        cout << "Binded to :" << myurl << endl;
        nn_connect(temp_sock, myurl);
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to)) >= 0);
        recv_sockets.insert(make_pair(i, temp_sock));
        // cout << recv_sockets.size() <<endl;
    }
    thread one(send_thread, id);
    thread two(recv_thread, id);

    one.join();
    two.join();
    cout << "resp_area" << endl;

    for (int i = 0; i < NODE_CNT; i++)
    {
        if (i == id)
            continue;
        print_stag(resp_area_stag[i]);
    }
    cout << "req_area" << endl;
    for (int i = 0; i < NODE_CNT; i++)
    {
        if (i == id)
            continue;
        print_stag(req_area_stag[i]);
    }
}
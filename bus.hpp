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
#include "nn.hpp"
#include <fstream>
#include <thread>
#include <bits/stl_map.h>

using namespace std;
using namespace nn;
#define sz_n sizeof(struct stag)
#define S_QPA sizeof(struct qp_attr)

char **ifaddr;
std::map<int, int> send_sockets, recv_sockets;
std::map<int, stag> cli_req_stags, signed_req_stags, replica_mem_stags;

void print_qp_attr(struct qp_attr dest)
{
	fflush(stdout);
	printf("\t LID: %d QPN: %d PSN: %d\n", dest.lid, dest.qpn, dest.psn);
}

uint64_t get_port_id(uint64_t src_node_id, uint64_t dest_node_id)
{
    uint64_t port_id = 20000;
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

void recv_thread(int id, struct context *ctx)
{
    // cout << "In recv" << endl;
    int count = 0, i = 0;
    set<int> counter;
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = recv_sockets.find(i)->second;
        int r = nn_recv(sock, &client_req_stag[i], sizeof(struct stag), 1);
        if (r > 1)
        {
            count++;
            counter.insert(i);
            // cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    // cout << "Rec: res_area_stags" << endl;
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = recv_sockets.find(i)->second;
        int r = nn_recv(sock, &signed_req_stag[i], sizeof(struct stag), 1);
        if (r > 1)
        {
            count++;
            counter.insert(i);
            // cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = recv_sockets.find(i)->second;
        int r = nn_recv(sock, &replica_mem_stag[i], sizeof(struct stag), 1);
        if (r > 1)
        {
            count++;
            counter.insert(i);
            // cout << "Recieved from " << i << endl;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    // cout << "Rec: req_area_stags" << endl;
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = recv_sockets.find(i)->second;
        if(nn_recv(sock, &ctx->remote_qp_attrs[i], S_QPA, 0) > 1) {
            counter.insert(i);
            count++;
		}
        else{
		    fprintf(stderr, "ERROR reading qp attrs from socket");
        }        
		printf("Server %d <-- Node %d's qp_attr: ", ctx->id, i);
		print_qp_attr(ctx->remote_qp_attrs[i]);
		
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    // cout << "Recvd QPs" << endl;
}

void send_thread(int id, struct context *ctx)
{
    // cout << "In Send" << endl;
    int i = 0, count = 0;
    set<int> counter;
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = send_sockets.find(i)->second;
        int s = nn_send(sock, &client_req_stag[id], sizeof(struct stag), 1);
        /*if(send < 0)
        {
			fprintf(stderr, "send failed: %s\n", strerror(errno));
        }
        else*/
        if (s > 0)
        {
            // cout << "Sent to " << i << endl;
            counter.insert(i);
            count++;
        }
        
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    // cout << "Resp area sent" << endl;
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = send_sockets.find(i)->second;
        int s = nn_send(sock, &signed_req_stag[id], sizeof(struct stag), 1);
        if (s > 1)
        {
            // cout << "Sent to" << i << endl;
            counter.insert(i);
            count++;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = send_sockets.find(i)->second;
        int s = nn_send(sock, &replica_mem_stag[id], sizeof(struct stag), 1);
        if (s > 1)
        {
            // cout << "Sent to" << i << endl;
            counter.insert(i);
            count++;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    count = 0;
    i = 0;
    counter.clear();
    while (count <= NODE_CNT - 1)
    {
        if (i == id || counter.count(i) > 0)
        {
            i++;
            continue;
        }
        if (i > NODE_CNT - 1)
        {
            i = 0;
            continue;
        }
        int sock = send_sockets.find(i)->second;
        int s = nn_send(sock, &ctx->local_qp_attrs[i], S_QPA, 1);
        if (s > 1)
        {
            counter.insert(i);
            count++;
        }
        i++;
        if (counter.size() == NODE_CNT - 1)
        {
            break;
        }
    }
    // cout << "QPs Sent" << endl;
    // cout << "Done Send" << endl;
}

int node(const int argc, const char **argv, struct context *ctx)
{
    int id = atoi(argv[1]), i;
    char myurl[30], url[30];
    int to = 100, count = 0;

    for(int i = 0; i< NODE_CNT; i++){
        signed_req_stag[id].rkey[i] = signed_req_mr[i]->rkey;
        signed_req_stag[id].buf[i] = (uintptr_t) signed_req_area[i];
    }
    signed_req_stag[id].size = MSG_SIZE;
    signed_req_stag[id].id = id;

    for(int i = 0; i< NODE_CNT ; i++){
        client_req_stag[id].rkey[i] = client_req_mr[i]->rkey;
        client_req_stag[id].buf[i] = (uintptr_t) client_req_[i];
    }
    client_req_stag[id].size = MSG_SIZE;
    client_req_stag[id].id = id;

    for(int i = 0; i< NODE_CNT; i++){
        replica_mem_stag[id].rkey[i] = replica_mem_mr[i]->rkey;
        replica_mem_stag[id].buf[i] = (uintptr_t) replica_mem_area[i];
    }
    replica_mem_stag[id].size = MSG_SIZE;
    replica_mem_stag[id].id = id;

    // char my_serialized_stag[sizeof(struct stag)+1];
    // serialize(node_stags[id], my_serialized_stag);

    for (i = 0; i < NODE_CNT; i++)
    {
        if (i == id)
            continue;
        int temp_sock = nn_socket(AF_SP, NN_PAIR);
        uint64_t p = get_port_id(id, i);
        snprintf(url, 30, "tcp://%s:%ld", ifaddr[i], p);
        nn_connect(temp_sock, url);
        // cout << "Connected to :" << url << endl;
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
        snprintf(myurl, 30, "tcp://%s:%ld", ifaddr[id], p);
        int rc = nn_bind(temp_sock, myurl);
        // cout << rc << endl;
        // cout << "Binded to :" << myurl << endl;
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
        assert(nn_setsockopt(temp_sock, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to)) >= 0);
        recv_sockets.insert(make_pair(i, temp_sock));
        // cout << recv_sockets.size() <<endl;
    }
    thread one(send_thread, id, ctx);
    thread two(recv_thread, id, ctx);

    one.join();
    two.join();
    //DEBUG
    // cout << "DEBUG: client_area stags:" << endl;

    // for (int i = 0; i < NODE_CNT; i++)
    // {
    //     if (i == id)
    //         continue;
    //     print_stag(client_req_stag[i]);
    // }
    // cout << "DEBUG: signed_area stags:" << endl;
    // for (int i = 0; i < NODE_CNT; i++)
    // {
    //     if (i == id)
    //         continue;
    //     print_stag(signed_req_stag[i]);
    // }
    // cout << "DEBUG: replica_area stags:" << endl;
    // for (int i = 0; i < NODE_CNT; i++)
    // {
    //     if (i == id)
    //         continue;
    //     print_stag(replica_mem_stag[i]);
    // }
    //DEBUG
    for (int i = 0; i < NODE_CNT ; i++){
        if (i == id){
            continue;
        }
        nn_shutdown(recv_sockets[i], 0);
        nn_shutdown(send_sockets[i], 0);
    }
}
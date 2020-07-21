#include <stdio.h>
#include <string.h>
#include "rdma_.h"

#define NODE_CNT 5

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: node <id>\n");
        return -1;
    }
    int send_ports[NODE_CNT],recv_ports[NODE_CNT],i;
    int id = atoi(argv[1]);
    for(int i = 0; i < NODE_CNT; i++){
        send_ports[i] = 17000 + i;
    }
    for(int i = 0; i < NODE_CNT; i++){
        recv_ports[i] = 17000 + NODE_CNT + i;
    }
    int res = -1;
    char send_url[21] = "tcp://127.0.0.1:"; //tcp_connect
    char recv_url[21] = "tcp://127.0.0.1:"; //tcp_bind

    char all_sendrs[NODE_CNT][21];
    for(int i = 0; i< NODE_CNT;i++){
        if(i == id)
            continue;
        snprintf(send_url, 21, "tcp://127.0.0.1:%d",i);
        strcpy(all_sendrs[i], send_url);
    }
    
    // strcat(recv_url, to_string(recv_ports[id]));
    // struct application_data* data = init_application_data(recv_url, 1, 65535, 1, NULL, NULL, NULL);

    // // struct context *ctx = init_ctx(data);
    // // set_local_connection_attr(ctx, data);

    // tcp_bind(data);



    // if(tcp_exchange_qp_info(data) != 1){
    //     printf("Cannot exchange remote info\n");
    //     return -1;
    // }
    // printf("Able to exchange data\n");
    // return 0;
}
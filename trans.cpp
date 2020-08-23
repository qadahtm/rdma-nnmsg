#include "bus.hpp"

void rdma_send_thread(struct context *ctx, int total){
	*req_area = 121;
	int co = 0;
	while(co < total){
		for(int i = 0; i < NODE_CNT ;i++){
			if(i == ctx->id){
				continue;
			}
			int dest = i;
			rdma_send(ctx, dest, req_area, req_area_mr->lkey,0,MSG_SIZE);
		}
		co++;
	}
}

void rdma_recv_thread(struct context *ctx, int total){
	int co = 0, rc = 0;
	while(co < total){
		int i = 0;
		for(int i = 0; i < NODE_CNT; i++){
			if(i == ctx->id){
				continue;
			}
			int src = i;
		 	rdma_recv(ctx, 1, src, resp_area, resp_area_mr->lkey,MSG_SIZE);
			cout << "RDMA Recieved Data:" << *resp_area << endl;
		}
		co++;
	}
}

void singleThreadTest(struct context *ctx){
	if(ctx->id == 0){
		// *req_area = (uintptr_t) resp_area;
		sprintf((char *)req_area, "Hello from the other side: %d \n", 123);
		int dest = 1, count = 0;
		// rdma_send(ctx, dest, req_area, req_area_mr->lkey,0,sizeof(int64_t));
		// post_read(ctx, 1, req_area, req_area_mr->lkey, resp_area_stag[dest].buf, resp_area_stag[dest].rkey, 0, MSG_SIZE);
		// cout << "sent" << *req_area << endl;
		post_write(ctx, 1, req_area, req_area_mr->lkey, resp_area_stag[dest].buf, resp_area_stag[dest].rkey, 1, MSG_SIZE);
		poll_cq(ctx->cq[dest], 1);
		cout << "write: " << req_area << endl;
	}
	else {
		int src = 0;
		// poll_cq(ctx->cq[src], 1);
		// sprintf((char *)resp_area, "Hello from the other side: %d \n", 123);
		// poll_cq(ctx->cq[src], 1);
		// cout << "Polled" << endl;
		// rdma_recv(ctx, 1, src, resp_area, resp_area_mr->lkey,sizeof(int64_t));
		sleep(10);
		cout << "I have: " << resp_area << endl;
	}
}

int main(const int argc, const char **argv)
{
    read_ifconfig("./ifconfig.txt");
    int i;
	struct ibv_device **dev_list;
	struct ibv_device *ib_dev;
	struct context *ctx;
	// char *buf = "Message";

	srand48(getpid() * time(NULL));		//Required for PSN
	ctx = (context *) malloc(sizeof(struct context));
    ctx->id = atoi(argv[1]);
    ctx->local_qp_attrs = (struct qp_attr *) malloc(
			NODE_CNT * sizeof(struct qp_attr));
	ctx->remote_qp_attrs = (struct qp_attr *) malloc(
			NODE_CNT * sizeof(struct qp_attr));
    
	dev_list = ibv_get_device_list(NULL);

    dev_list = ibv_get_device_list(NULL);
	CPE(!dev_list, "Failed to get IB devices list", 0);

	ib_dev = dev_list[0];
	//ib_dev = dev_list[0];
	CPE(!ib_dev, "IB device not found", 0);

	init_ctx(ctx, ib_dev);
	CPE(!ctx, "Init ctx failed", 0);

    cout << "Context Initialized" << endl;

	setup_buffers(ctx);

	union ibv_gid my_gid = get_gid(ctx->context);

	for(i = 0; i < ctx->num_conns; i++) {
		ctx->local_qp_attrs[i].id = my_gid.global.interface_id;
		ctx->local_qp_attrs[i].lid = get_local_lid(ctx->context);
		ctx->local_qp_attrs[i].qpn = ctx->qp[i]->qp_num;
		ctx->local_qp_attrs[i].psn = lrand48() & 0xffffff;
		printf("Local address of RC QP %d: ", i);
		print_qp_attr(ctx->local_qp_attrs[i]);
	}

    if (argc >= 1)
        node(argc, argv, ctx);
        //return 0;
    else
    {
        fprintf(stderr, "Usage: bus <node-id>...\n");
        return 1;
    }
	cout << "Exchange done!" << endl;
	for(int i = 0;i < NODE_CNT; i++){
		connect_ctx(ctx, ctx->local_qp_attrs[i].psn, ctx->remote_qp_attrs[i], ctx->qp[i], 0);
	}
	//qp_to_rtr(ctx->qp[i], ctx);
	// if(ctx->id == 1){
	// 	post_recv(ctx, 1, 0, resp_area, resp_area_mr->lkey, MSG_SIZE);
	// }
	cout << "QPs Connected" << endl;
	
	singleThreadTest(ctx);

    // thread rrecv(rdma_recv_thread, ctx, 10);
	// thread rsend(rdma_send_thread, ctx, 10);

    // rsend.join();
    // rrecv.join();

}
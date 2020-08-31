#include "bus.hpp"


//For Testing send and Recv
// void rdma_send_thread(struct context *ctx, int total){
// 	*req_area = 121;
// 	int co = 0;
// 	while(co < total){
// 		for(int i = 0; i < NODE_CNT ;i++){
// 			if(i == ctx->id){
// 				continue;
// 			}
// 			int dest = i;
// 			rdma_send(ctx, dest, req_area, req_area_mr->lkey,0,MSG_SIZE);
// 		}
// 		co++;
// 	}
// }
// void rdma_recv_thread(struct context *ctx, int total){
// 	int co = 0, rc = 0;
// 	while(co < total){
// 		int i = 0;
// 		for(int i = 0; i < NODE_CNT; i++){
// 			if(i == ctx->id){
// 				continue;
// 			}
// 			int src = i;
// 		 	rdma_recv(ctx, 1, src, resp_area, resp_area_mr->lkey,MSG_SIZE);
// 			cout << "RDMA Recieved Data:" << *resp_area << endl;
// 		}
// 		co++;
// 	}
// }

void remote_writer_thread(){
	if(ctx->id < 4){
		mtx.lock();
		char *msg = (char *)malloc(256);
		snprintf(msg, 256, "This response is from %d", ctx->id);
		strcpy(resp_area[4], msg);
		rdma_remote_write(ctx, 4, resp_area[4], resp_area_mr[4]->lkey, resp_area_stag[4].buf[ctx->id], resp_area_stag[4].rkey[ctx->id]);
		mtx.unlock();
	}
	if(ctx->id == 4){
		mtx.lock();
		char *msg = (char *)malloc(256);
		snprintf(msg, 256, "CLIENT REQUEST from %d", ctx->id);
		strcpy(req_area[0], msg);
		rdma_remote_write(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[ctx->id], req_area_stag[0].rkey[ctx->id]);
		mtx.unlock();
	}
}

void local_reader_thread(){
	sleep(1);
	for(int i = 0; i< NODE_CNT; i++) {
		if(req_area[i][0] != 0)
			cout << "REQ from node: " << i << " : " << req_area[i] << endl;
		if(resp_area[i][2] != 0)
			if(ctx -> id == 4) {
				cout << "RESP from node: " << i << " : " << resp_area[i] << endl;
			} else {
				cout << "RESP to node: " << i << " : " << resp_area[i] << endl;
			}
	}
}

void remote_reader_thread(){
	if(ctx->id > 0 && ctx->id < 4){
		mtx.lock();
		while(req_area[0] && req_area[0][0] == 0){
			rdma_remote_read(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[4], req_area_stag[0].rkey[4]);
		}
		mtx.unlock();
		// cout << "Request read from node " << 0 << " : " << req_area[0] << endl;
	}
}

void singleThreadTest(struct context *ctx){
	if (ctx->id == 0){
		snprintf(resp_area[0], 64,"This is a message in node %d", ctx->id);
		snprintf(resp_area[1], 64,"This is a message to be written by node %d", ctx->id);
		snprintf(resp_area[2], 64,"This is a message to be written by node %d", ctx->id);
		rdma_remote_write(ctx, 1, resp_area[1], resp_area_mr[1]->lkey, resp_area_stag[1].buf[0], resp_area_stag[1].rkey[0]);
		rdma_remote_write(ctx, 2, resp_area[2], resp_area_mr[2]->lkey, resp_area_stag[2].buf[0], resp_area_stag[2].rkey[0]);
		sleep(5);
	}
	else{
		cout << "Single Thread test" << endl;
		rdma_remote_read(ctx, 0, resp_area[ctx->id], resp_area_mr[ctx->id]->lkey, resp_area_stag[0].buf[0], resp_area_stag[0].rkey[0]);
		cout << "Node: " << 0 << " had message " << resp_area[ctx->id] << endl;
		sleep(8);
		cout << "I recieved messaged from " << resp_area[0] << endl;
	}
}

int main(const int argc, const char **argv)
{
    read_ifconfig("./ifconfig.txt");
    int i;
	struct ibv_device **dev_list;
	struct ibv_device *ib_dev;
	
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

    // cout << "Context Initialized" << endl;

	setup_buffers(ctx);
	//set the memory buffers to some default value

	union ibv_gid my_gid = get_gid(ctx->context);

	for(i = 0; i < ctx->num_conns; i++) {
		ctx->local_qp_attrs[i].id = my_gid.global.interface_id;
		ctx->local_qp_attrs[i].lid = get_local_lid(ctx->context);
		ctx->local_qp_attrs[i].qpn = ctx->qp[i]->qp_num;
		ctx->local_qp_attrs[i].psn = lrand48() & 0xffffff;
		// printf("Local address of RC QP %d: ", i);
		// print_qp_attr(ctx->local_qp_attrs[i]);
	}

    if (argc >= 1)
        node(argc, argv, ctx);
        //return 0;
    else
    {
        fprintf(stderr, "Usage: bus <node-id>...\n");
        return 1;
    }
	// cout << "Exchange done!" << endl;
	for(int i = 0;i < NODE_CNT; i++){
		connect_ctx(ctx, ctx->local_qp_attrs[i].psn, ctx->remote_qp_attrs[i], ctx->qp[i], 0, i);
	}
	
	cout << "QPs Connected" << endl;
	
	// singleThreadTest(ctx);
	if(ctx->id == 4){
		strcpy(req_area[0], "Message from the client");
	}

	thread remote_reader(remote_reader_thread);
	thread remote_writer(remote_writer_thread);
	thread local_reader(local_reader_thread);


	remote_reader.join();
	remote_writer.join();
	local_reader.join();
}
#include "bus.hpp"
#include <mutex>


//For Testing send and Recv
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
std::mutex mtx;

void rdma_remote_read_thread(struct context *ctx){
	for(int i = 0; i< NODE_CNT; i++) {
		if(i == ctx->id) continue;
		mtx.lock();
		rdma_remote_read(ctx, i, req_area, req_area_mr->lkey, req_area_stag[i].buf, req_area_stag[i].rkey);
		cout << "REMOTE READ: Found : " << req_area << " on req_area of Node: " << i << endl;
		mtx.unlock();
	}
}

void rdma_remote_write_thread(struct context *ctx){
	for(int i = 0; i< NODE_CNT; i++) {
		if(i == ctx->id) continue;
		mtx.lock();
		rdma_remote_write(ctx, i, resp_area, resp_area_mr->lkey, resp_area_stag[i].buf, resp_area_stag[i].rkey);
		cout << "REMOTE WRITE: Wrote : " << resp_area << " on resp_area of Node: " << i << endl;
		mtx.unlock();
	}
}

void local_read_thread(struct context *ctx){
	for(int i = 0; i< NODE_CNT; i++) {
		if(i == ctx->id) continue;
		mtx.lock();
		while ((resp_area != NULL) && (resp_area[0] == '\0')) {
   			continue;
		}
		cout << "LOCAL READ: Recieved :" << resp_area << " from Node: " << i << endl;
		mtx.unlock();
	}
}

void singleThreadTest(struct context *ctx){
	if(ctx->id == 0){
		int dest = 1, count = 0;
		sprintf((char *)req_area, "Hello from the other side, I am node: %d \n", ctx->id);
		rdma_remote_write(ctx, 1, req_area, req_area_mr->lkey, resp_area_stag[dest].buf, resp_area_stag[dest].rkey);
		cout << "Written: " << req_area << endl;
		while ((resp_area != NULL) && (resp_area[0] == '\0')) {
   			continue;
		}
		cout << "Node 2 wrote:" << resp_area <<" With size: " << sizeof(resp_area)<< endl;
		rdma_remote_read(ctx, 2, req_area, req_area_mr->lkey, req_area_stag[2].buf, req_area_stag[2].rkey);
		cout << "On Node 2's memory, I found: " << req_area << endl;
	}
	if(ctx -> id == 1) {
		int src = 0;
		char *buf = (char *)malloc(MSG_SIZE);
		cout << resp_area << endl;
		while ((resp_area != NULL) && (resp_area[0] == '\0')) {
   			continue;
		}
		cout << "Other node wrote:" << resp_area <<" With size: " << sizeof(resp_area)<< endl;
		memset(resp_area, '\0', sz_n);
		if ((resp_area != NULL) && (resp_area[0] == '\0')) {
   			cout << "Resp area is empty" << endl;
		}
	}
	if(ctx->id == 2){
		int dest = 0;
		sprintf((char *)req_area, "Hello from the other side, I am node: %d \n", ctx->id);
		rdma_remote_write(ctx, 0, req_area, req_area_mr->lkey, resp_area_stag[dest].buf, resp_area_stag[dest].rkey);
		cout << "Written: " << req_area << endl;
		rdma_remote_write(ctx, 1, req_area, req_area_mr->lkey, resp_area_stag[1].buf, resp_area_stag[1].rkey);
		cout << "Written: " << req_area << endl;
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
	snprintf(req_area, MSG_SIZE, "This is from Node: %d", ctx->id);
	snprintf(resp_area, MSG_SIZE, "This is from Node: %d", ctx->id);

	thread remote_reader(rdma_remote_read_thread, ctx);
	thread remote_writer(rdma_remote_write_thread, ctx);
	thread local_reader(local_read_thread, ctx);


	remote_reader.join();
	remote_writer.join();
	local_reader.join();
}
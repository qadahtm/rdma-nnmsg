#include "bus.hpp"

void singleThreadTest(struct context *ctx){
	if(ctx->id == 0){
		cout << "Before: " << (int)local_area[0] << endl;
		rdma_cas(ctx, 1, local_area, local_area_mr->lkey, client_req_stag[1].buf[0], client_req_stag[1].rkey[0], 0, 2);
		// rdma_remote_write(ctx, 1, local_area, local_area_mr->lkey, client_req_stag[1].buf[0], client_req_stag[1].rkey[0]);
		poll_cq(ctx->cq[1],1);
		cout << "Before read: " << (int)local_area[0] << endl;
		rdma_remote_read(ctx, 1, local_area, local_area_mr->lkey, client_req_stag[1].buf[0], client_req_stag[1].rkey[0]);
		cout << "After: " << (int)local_area[0] << endl;
	}
	if(ctx->id == 1){
		cout << (int)client_req_[0][0] << endl;
		sleep(10);
		cout << (int)client_req_[0][0] << endl;
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
	
	singleThreadTest(ctx);
	// if(ctx->id == 4){
	// 	strcpy(req_area[0], "Message from the client");
	// }

	// thread remote_reader(remote_reader_thread);
	// thread remote_writer(remote_writer_thread);
	// thread local_reader(local_reader_thread);

	// remote_reader.join();
	// remote_writer.join();
	// local_reader.join();
}
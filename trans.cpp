#include "bus.hpp"
#include "message.hpp"
#define MILLION 1000000
#define PRIMARY 0
#define CLIENT 4

using namespace std::chrono;

long long getTime(){
	milliseconds ms = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );
	return ms.count();
}

int start_time = 0, final_time = 0;

//1. client_req_mem -> Primary.client_req_mem : Write thread
//2. Primary.client_req_mem -> signed_req_mem : local write thread
//3. replica.replica_mem <- Primary.signed_req_mem : remote read thread DONE
//4. client.rep_mem <- replica.rep_mem : Write Thread DONE

// void reader(struct context *ctx){
// 	if(ctx->id != 0 and ctx->id != 4){
// 		while(replica_mem_area[0][0] == 0)
// 			rdma_remote_read(ctx, 0, replica_mem_area[0], replica_mem_mr[0]->lkey, signed_req_stag[0].buf[0], signed_req_stag[0].rkey[0]);
// 		cout << "This is " << ctx->id << " with message " << get_message(replica_mem_area[0]) << endl;
// 		rdma_remote_write(ctx, 4, replica_mem_area[0], replica_mem_mr[0]->lkey, replica_mem_stag[4].buf[ctx->id], replica_mem_stag[4].rkey[ctx->id]);
// 	}
// 	if(ctx->id == 4){
// 		for(int i = 1; i < NODE_CNT - 1; i++){
// 			while(replica_mem_area[i][0] == 0){
// 				continue;
// 			}
// 			cout << "Recieved message from node" << i << " " << get_message(replica_mem_area[i]) << endl;
// 		}
// 	}
// }

// void writer(struct context *ctx){
// 	if(ctx->id == 4){
// 		char* curr_msg = create_message("Message", 0);
// 		cout << get_sequence(curr_msg) << get_message(curr_msg)<< endl;
// 		strcpy(local_area, curr_msg);
// 		rdma_remote_write(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[ctx->id], client_req_stag[0].rkey[ctx->id]);
// 		cout << "CLIENT: Message written on Primary " << get_message(local_area) << endl;
// 	}
// 	if(ctx->id == 0){
// 		while(client_req_[4][0] == 0){
// 			continue;
// 		}
// 		strcpy(signed_req_area[ctx->id],client_req_[4]);
// 		cout << "Signed message: " << get_message(client_req_[4]) << "to: " << get_message(signed_req_area[ctx->id]) << endl; 
// 		sleep(1);
// 	}
// }

void singleThreadTest(struct context *ctx){
	if(ctx->id == 1){
		bool var = false;
		while(var == false){
			var = rdma_cas(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[0], client_req_stag[0].rkey[0], 0, 1);
		}
		rdma_remote_read(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[0], client_req_stag[0].rkey[0], MSG_SIZE);
		cout << get_message(deserialize(local_area)) << " :DEBUG: " << get_sequence(deserialize(local_area)) << endl;
	}
	if(ctx->id == 0){
		struct msg* temp = create_message("Message LOL", 123);
    	char *sbuf = serialise(temp);
		memcpy(client_req_[0] + 8, sbuf, 248);
		cout << get_message(deserialize(client_req_[0])) << " :DEBUG: " << get_sequence(deserialize(client_req_[0])) << endl;
		sleep(2);
		cout << "CAS Value:" << (int)client_req_[0][0] << endl;
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

	// thread remote_reader(reader, ctx);
	// thread remote_writer(writer, ctx);

	// remote_reader.join();
	// remote_writer.join();
}
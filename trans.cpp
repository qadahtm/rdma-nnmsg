#include "bus.hpp"
#include "message.hpp"
#define MILLION 1000000
#define PRIMARY 0
#define CLIENT 4

using namespace std::chrono;

bool local_cas(char a, int c, int s){
	return (__sync_val_compare_and_swap(&a, c, s) == (u_char)c);
}

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

void generate_request(int seq_id, char *addr){
	char *sbuf = serialise(create_message("Message", seq_id));
	memcpy(addr, sbuf, MSG_SIZE);
}
//Do not put the lock in this function in addr (start the pointer from 8th address)
char* check_request(char *addr){
	struct msg* tmsg = deserialize(addr);
	cout << "DEBUG: " << get_message(tmsg);
	return get_message(tmsg);
}

void reader(struct context *ctx){
	cout << "Reader Working" << endl;
	sleep(10);
}
/*CAS:
0: No Message
1: Message
2: Locked
*/
std::mutex bufMTX;
void rsend(struct context *ctx){
	for(int dest = 0; dest < NODE_CNT; dest++){
		if(dest == ctx->id){
			continue;
		}
		bufMTX.lock();
		char *buf = (char *)malloc(MSG_SIZE - 8);
		sprintf(buf, "Message %d", ctx->id);
		memcpy(client_req_[dest], buf, MSG_SIZE - 8);
		while(!rdma_cas(ctx, dest, cas_area, cas_area_mr->lkey, signed_req_stag[dest].buf[ctx->id], signed_req_stag[dest].rkey[ctx->id], 0, 2)){
			continue;
		}
		memset(cas_area, 0, MSG_SIZE);
		rdma_remote_write(ctx, dest, client_req_[dest], client_req_mr[dest]->lkey, (signed_req_stag[dest].buf[ctx->id] + 8), signed_req_stag[dest].rkey[ctx->id], MSG_SIZE - 8);
		while(!rdma_cas(ctx, dest, cas_area, cas_area_mr->lkey, signed_req_stag[dest].buf[ctx->id], signed_req_stag[dest].rkey[ctx->id], 2, 1)){
			continue;
		}
		bufMTX.unlock();
	}
}

void rrecv(struct context *ctx){
	sleep(5);
	int i = 0;
	while(i < NODE_CNT){
		if(ctx->id == i){
			i++;
			continue;
		}
		char *buf = (char *)malloc(MSG_SIZE);
		strcpy(buf, signed_req_area[i] + 8);
		while(!local_cas(signed_req_area[i][0], 1, 0)){
			continue;
		}
		cout << "DEBUG: SIGNED CAS RES: " << i << " " << (int)signed_req_area[i][0] << endl;
		// cout << "DEBUG: SIGNED WRITE RES: " << i << " " << buf << endl;
		fflush(stdout);
		i++;
	}
	// uint32_t node = 0;
    // char *buf;
    // bool flag = false;
    // fflush(stdout);
	// int count = 0;
    // while(flag == false && count < 4){
    //     bufMTX.lock();
    //     if(node == ctx->id){
    //         node++;
    //         continue;
    //     }
	// 	cout << "DEBUG: Trying to read: " << node << endl;
    //     if(!local_cas(signed_req_area[node][0], 1, 2)){
    //         node++;
    //         if(node > 5){
    //             node = 0;
    //         }
    //         continue;
    //     }
    //     else {
    //         buf = (char *)malloc(MSG_SIZE - 8);
    //         memcpy(buf, signed_req_area + 8, MSG_SIZE - 8);
	// 		cout << buf << endl;
    //         local_cas(signed_req_area[node][0], 2, 0);
    //     }
    //     bufMTX.unlock();
    //     flag = true;
	// 	count += 1;
    // }
    // cout <<"SP --> DEBUG: MSG RECVD" << endl;
    // fflush(stdout);
}

// void singleThreadTest(struct context *ctx){
// 	if(ctx->id == 1){
// 		bool var = false;
// 		while(var == false){
// 			var = rdma_cas(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[0], client_req_stag[0].rkey[0], 0, 1);
// 		}
// 		rdma_remote_read(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[0], client_req_stag[0].rkey[0], MSG_SIZE);
// 		// cout << get_message(deserialize(local_area)) << " :DEBUG: " << get_sequence(deserialize(local_area)) << endl;
// 	}
// 	if(ctx->id == 0){
// 		struct msg* temp = create_message("Message LOL", 123);
//     	char *sbuf = serialise(temp);
// 		memcpy(client_req_[0] + 8, sbuf, 248);
// 		// cout << get_message(deserialize(client_req_[0])) << " :DEBUG: " << get_sequence(deserialize(client_req_[0])) << endl;
// 		sleep(2);
// 		cout << "CAS Value:" << (int)client_req_[0][0] << endl;
// 	}
// }

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
	// if(ctx->id == 4){
	// 	strcpy(req_area[0], "Message from the client");
	// }

	thread remote_reader(rsend, ctx);
	thread remote_writer(rrecv, ctx);

	remote_reader.join();
	remote_writer.join();
}
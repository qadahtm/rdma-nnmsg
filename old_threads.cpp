// // void remote_writer_thread(){
// // 	if(ctx->id < 4){
// // 		mtx.lock();
// // 		char *msg = (char *)malloc(256);
// // 		snprintf(msg, 256, "This response is from %d", ctx->id);
// // 		strcpy(resp_area[4], msg);
// // 		rdma_remote_write(ctx, 4, resp_area[4], resp_area_mr[4]->lkey, resp_area_stag[4].buf[ctx->id], resp_area_stag[4].rkey[ctx->id]);
// // 		mtx.unlock();
// // 	}
// // 	if(ctx->id > 0 && ctx->id < 4){
// // 		mtx.lock();
// // 		char *msg = (char *)malloc(256);
// // 		snprintf(msg, 256, "This response is from %d", ctx->id);
// // 		strcpy(resp_area[4], msg);
// // 		rdma_remote_write(ctx, 0, resp_area[0], resp_area_mr[0]->lkey, resp_area_stag[0].buf[ctx->id], resp_area_stag[0].rkey[ctx->id]);
// // 		mtx.unlock();
// // 	}
// // 	if(ctx->id == 4){
// // 		mtx.lock();
// // 		char *msg = (char *)malloc(256);
// // 		snprintf(msg, 256, "CLIENT REQUEST from %d", ctx->id);
// // 		strcpy(req_area[0], msg);
// // 		rdma_remote_write(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[ctx->id], req_area_stag[0].rkey[ctx->id]);
// // 		mtx.unlock();
// // 	}
// // }

// // void local_reader_thread(){
// // 	sleep(3);
// // 	for(int i = 0; i< NODE_CNT; i++) {
// // 		if(req_area[i][0] != 0)
// // 			cout << "REQ from node: " << i << " : " << req_area[i] << endl;
// // 		if(resp_area[i][2] != 0){
// // 			if(ctx -> id == 4) {
// // 				cout << "RESP from node: " << i << " : " << resp_area[i] << endl;
// // 			} else {
// // 				cout << "RESP to node: " << i << " : " << resp_area[i] << endl;
// // 			}
// // 		}
// // 	}
// // }

// // void remote_reader_thread(){
// // 	if(ctx->id > 0 && ctx->id < 4){
// // 		mtx.lock();
// // 		while(req_area[0] && req_area[0][0] == 0){
// // 			rdma_remote_read(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[4], req_area_stag[0].rkey[4]);
// // 		}
// // 		mtx.unlock();
// // 		// cout << "Request read from node " << 0 << " : " << req_area[0] << endl;
// // 	}
// // }

// // void singleThreadTest(struct context *ctx){
// // 	if (ctx->id == 0){
// // 		snprintf(resp_area[0], 64,"This is a message in node %d", ctx->id);
// // 		snprintf(resp_area[1], 64,"This is a message to be written by node %d", ctx->id);
// // 		snprintf(resp_area[2], 64,"This is a message to be written by node %d", ctx->id);
// // 		rdma_remote_write(ctx, 1, resp_area[1], resp_area_mr[1]->lkey, resp_area_stag[1].buf[0], resp_area_stag[1].rkey[0]);
// // 		rdma_remote_write(ctx, 2, resp_area[2], resp_area_mr[2]->lkey, resp_area_stag[2].buf[0], resp_area_stag[2].rkey[0]);
// // 		sleep(5);
// // 	}
// // 	else{
// // 		cout << "Single Thread test" << endl;
// // 		rdma_remote_read(ctx, 0, resp_area[ctx->id], resp_area_mr[ctx->id]->lkey, resp_area_stag[0].buf[0], resp_area_stag[0].rkey[0]);
// // 		cout << "Node: " << 0 << " had message " << resp_area[ctx->id] << endl;
// // 		sleep(8);
// // 		cout << "I recieved messaged from " << resp_area[0] << endl;
// // 	}
// // }



// //////////////////////................../////////////////////

// void reader(struct context *ctx){
// 	if(ctx->id == 0){
// 		cout << "Client Req: " << (int)client_req_[0][0] << endl;
// 		int i = 0;
// 		sleep(5);
// 		while(i < MSG_SIZE){
// 			cout << (int)client_req_[0][i];
// 			i++;
// 		}
// 		// int slot = 0, count_msg = 0;
// 		// while(slot < 5 || count_msg == 5){
// 		// 	// while(true){
// 		// 	// 	if(local_cas(client_req_[i][0], 0, 1) == false){
// 		// 	// 		cout << "Checking for message CAS" << i << endl;
// 		// 	// 		continue;
// 		// 	// 	} 
// 		// 	// 	if(client_req_[i][8] == 0){
// 		// 	// 		cout << "Checking for message" << i << endl;
// 		// 	// 		local_cas(client_req_[i][0], 1, 0);
// 		// 	// 		continue;
// 		// 	// 	} else {
// 		// 	// 		break;
// 		// 	// 	}
// 		// 	// }
// 		// 	mtx.lock();
// 		// 	struct msg *rbuf = deserialize(client_req_[slot]);
// 		// 	char *m = get_message(rbuf);
// 		// 	unsigned seq = get_sequence(rbuf);
// 		// 	cout << "DEBUG: " << m << " seq: " << seq << endl; 
// 		// 	struct msg *cbuf = create_message(m, seq);
// 		// 	memcpy(signed_req_area[slot] + 8, serialise(cbuf), MSG_SIZE);
// 		// 	count_msg++;
// 		// 	mtx.unlock();
// 		// 	local_cas(client_req_[slot][0], 1, 0);
// 		// }
// 	}
// 	// if(ctx->id < 4 && ctx->id != 0){
// 	// 	for(int i = 0; i< NODE_CNT; i++){
// 	// 		bool var = false;
// 	// 		while(var == false){
// 	// 			mtx.lock();
// 	// 			var = rdma_cas(ctx, 0, local_area, local_area_mr->lkey, signed_req_stag[0].buf[i], signed_req_stag[0].rkey[i], 0, 1);
// 	// 			mtx.unlock();
// 	// 		}
// 	// 		mtx.lock();
// 	// 		rdma_remote_read(ctx, 0, replica_mem_area[i], replica_mem_mr[0]->lkey, signed_req_stag[0].buf[i], signed_req_stag[0].rkey[i], MSG_SIZE);
// 	// 		memset(local_area, 0, MSG_SIZE);
// 	// 		rdma_cas(ctx, 0, local_area, local_area_mr->lkey, signed_req_stag[0].buf[i], signed_req_stag[0].rkey[i], 1, 0);
// 	// 		mtx.unlock();
// 	// 	}
// 	// }
// 	// if(ctx->id == 4){
// 	// 	for(int i = 1; i < NODE_CNT - 1; i++){
// 	// 		while(replica_mem_area[i][0] == 0){
// 	// 			continue;
// 	// 		}
// 	// 		cout << "Recieved message from node" << i << " " << get_message(replica_mem_area[i]) << endl;
// 	// 	}
// 	// }
// }

// // void writer(struct context *ctx){
// // 	if(ctx->id == 4){
// // 		cout << "Here" << endl;
// // 		for(int i = 0; i< NODE_CNT - 1; i++){
// // 			bool var = false;
// // 			// rdma_remote_read(ctx, 0, local_area, local_area_mr->lkey,  client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 8);
// // 			// cout << "DEBUG READ:  " << (int)local_area[0] << endl;
// // 			while(var == false){
// // 				mtx.lock();
// // 				var = rdma_cas(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 0, 1);
// // 				mtx.unlock();
// // 			}
// // 			// cout << "DEBUG CAS while: " << i << (int)local_area[0] << endl;
// // 			mtx.lock();
// // 			memset(local_area, 0, MSG_SIZE);
// // 			struct msg* curr_msg = create_message("Message", i);
// // 			cout << "DEBUG: " << get_sequence(curr_msg) << " " << get_message(curr_msg)<< endl;
// // 			memcpy(local_area, serialise(curr_msg), 256);
// // 			int lol = 0;
// // 			while(lol < MSG_SIZE){
// // 				cout << (int)local_area[i];
// // 				lol++;
// // 			}
// // 			rdma_remote_write(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[i] + 8, client_req_stag[0].rkey[i]);
// // 			// rdma_remote_read(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[i] + 8, client_req_stag[0].rkey[i], MSG_SIZE);
// // 			cout << endl;
// // 			memset(local_area, 0, MSG_SIZE);
// // 			rdma_cas(ctx, 0, local_area, local_area_mr->lkey, client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 1, 0);
// // 			mtx.unlock();
// // 			// cout << "DEBUG: " << (int)local_area[0] << endl;
// // 		}
// // 	}
// // 	// if(ctx->id < 4 && ctx->id != 0){
// // 	// 	for(int i = 0; i < NODE_CNT; i++){
// // 	// 		while()
// // 	// 	}
// // 	// }
	
// // }


/* ANOTHER WRITER THREAD */

// if(ctx->id == 4){
// 		int i = 0;
// 		while(i < NODE_CNT){
// 			char *buf = (char *)malloc(MSG_SIZE - 8);
// 			generate_request(i, buf);
// 			cout << "DEBUG: 1" << i << endl;
// 			while(true){
// 				bool var1 = rdma_cas(ctx, 0, cas_area, cas_area_mr->lkey, client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 0, 2);
// 				cout << "DEBUG: 2" << i << endl;
// 				bool var2 = rdma_cas(ctx, 0, cas_area, cas_area_mr->lkey, client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 1, 2);
// 				cout << "DEBUG: 3" << i << endl;
// 				if(var1 || var2){
// 					break;
// 				}
// 			}
// 			memcpy(local_area,buf,MSG_SIZE-8);
// 			rdma_remote_write(ctx, 0, local_area, local_area_mr->lkey, replica_mem_stag[0].buf[i], replica_mem_stag[0].rkey[i], replica_mem_stag[0].size);
// 			cout << "DEBUG: 4" << i << endl;
// 			rdma_cas(ctx, 0, cas_area, cas_area_mr->lkey, client_req_stag[0].buf[i], client_req_stag[0].rkey[i], 2, 1);
// 			char *a = get_message(deserialize(local_area));
// 			cout << "DEBUG: WRITTEN: " << a << endl;
// 			memset(local_area, 0, MSG_SIZE);
// 			memset(cas_area, 0, MSG_SIZE);
// 			i++;
// 			if(i == NODE_CNT){
// 				i = 0;
// 			}
// 		}
// 	}
// 	else{
// 		sleep(10);
// 	}

//END OF WRITER THREAD
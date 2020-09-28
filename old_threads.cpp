// void remote_writer_thread(){
// 	if(ctx->id < 4){
// 		mtx.lock();
// 		char *msg = (char *)malloc(256);
// 		snprintf(msg, 256, "This response is from %d", ctx->id);
// 		strcpy(resp_area[4], msg);
// 		rdma_remote_write(ctx, 4, resp_area[4], resp_area_mr[4]->lkey, resp_area_stag[4].buf[ctx->id], resp_area_stag[4].rkey[ctx->id]);
// 		mtx.unlock();
// 	}
// 	if(ctx->id > 0 && ctx->id < 4){
// 		mtx.lock();
// 		char *msg = (char *)malloc(256);
// 		snprintf(msg, 256, "This response is from %d", ctx->id);
// 		strcpy(resp_area[4], msg);
// 		rdma_remote_write(ctx, 0, resp_area[0], resp_area_mr[0]->lkey, resp_area_stag[0].buf[ctx->id], resp_area_stag[0].rkey[ctx->id]);
// 		mtx.unlock();
// 	}
// 	if(ctx->id == 4){
// 		mtx.lock();
// 		char *msg = (char *)malloc(256);
// 		snprintf(msg, 256, "CLIENT REQUEST from %d", ctx->id);
// 		strcpy(req_area[0], msg);
// 		rdma_remote_write(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[ctx->id], req_area_stag[0].rkey[ctx->id]);
// 		mtx.unlock();
// 	}
// }

// void local_reader_thread(){
// 	sleep(3);
// 	for(int i = 0; i< NODE_CNT; i++) {
// 		if(req_area[i][0] != 0)
// 			cout << "REQ from node: " << i << " : " << req_area[i] << endl;
// 		if(resp_area[i][2] != 0){
// 			if(ctx -> id == 4) {
// 				cout << "RESP from node: " << i << " : " << resp_area[i] << endl;
// 			} else {
// 				cout << "RESP to node: " << i << " : " << resp_area[i] << endl;
// 			}
// 		}
// 	}
// }

// void remote_reader_thread(){
// 	if(ctx->id > 0 && ctx->id < 4){
// 		mtx.lock();
// 		while(req_area[0] && req_area[0][0] == 0){
// 			rdma_remote_read(ctx, 0, req_area[0], req_area_mr[0]->lkey, req_area_stag[0].buf[4], req_area_stag[0].rkey[4]);
// 		}
// 		mtx.unlock();
// 		// cout << "Request read from node " << 0 << " : " << req_area[0] << endl;
// 	}
// }

// void singleThreadTest(struct context *ctx){
// 	if (ctx->id == 0){
// 		snprintf(resp_area[0], 64,"This is a message in node %d", ctx->id);
// 		snprintf(resp_area[1], 64,"This is a message to be written by node %d", ctx->id);
// 		snprintf(resp_area[2], 64,"This is a message to be written by node %d", ctx->id);
// 		rdma_remote_write(ctx, 1, resp_area[1], resp_area_mr[1]->lkey, resp_area_stag[1].buf[0], resp_area_stag[1].rkey[0]);
// 		rdma_remote_write(ctx, 2, resp_area[2], resp_area_mr[2]->lkey, resp_area_stag[2].buf[0], resp_area_stag[2].rkey[0]);
// 		sleep(5);
// 	}
// 	else{
// 		cout << "Single Thread test" << endl;
// 		rdma_remote_read(ctx, 0, resp_area[ctx->id], resp_area_mr[ctx->id]->lkey, resp_area_stag[0].buf[0], resp_area_stag[0].rkey[0]);
// 		cout << "Node: " << 0 << " had message " << resp_area[ctx->id] << endl;
// 		sleep(8);
// 		cout << "I recieved messaged from " << resp_area[0] << endl;
// 	}
// }
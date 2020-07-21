#include <stdio.h>
#include <stdlib.h>
#include <infiniband/verbs.h>
#include <sys/time.h>
#include <netdb.h>
#include <malloc.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <nanomsg/nn.h>
#include <nanomsg/bus.h>
#include <nanomsg/pair.h>

union ibv_gid get_gid(struct ibv_context *context);
uint16_t get_local_lid(struct ibv_context *context);
static int poll_cq(struct ibv_cq *cq, int num_completions);

struct context {
	struct ibv_context *context;
	struct ibv_pd *pd;
	struct ibv_mr *mr;
	struct ibv_cq *rcq;
	struct ibv_cq *scq;
	struct ibv_qp *qp;
	struct ibv_comp_channel *ch;
	void *buf;
	unsigned size;
	int tx_depth;
	struct ibv_sge sge_list;
	struct ibv_send_wr wr;
};
/*
 *	lid - Local Identifier, 16 bit address assigned to end node by subnet manager 
 *	qpn - Queue Pair Number, identifies qpn within channel adapter (HCA)
 *  psn - Packet Sequence Number, used to verify correct delivery sequence of packages (similar to ACK)
 *	rkey - Remote Key, together with 'vaddr' identifies and grants access to memory region
 *	vaddr - Virtual Address, memory address that peer can later write to
 */
struct connection_attr {
    int lid;
    int qpn;
    int psn;
	unsigned rkey; // needs to be in a stag
	unsigned long long vaddr; //needs to be in a stag
};

struct application_data{
	char recv_url[21];
	int ib_port;
	unsigned size;
	int tx_depth;
	int sockfd;
	char *nodename;
	struct connection_attr local_con_info;
	struct connection_attr *remote_con_info;
	struct ibv_device *ib_dev;
};

//TO DO:
static int poll_cq(struct ibv_cq *cq, int num_completions);

static struct application_data* init_application_data(char r_u[21], int ib_port, int size, int tx_depth 
								,char* name
								,struct connection_attr* remote_qp
								,struct ibv_device *dev){

                                    struct application_data *temp;
									temp = (struct temp *) malloc(sizeof *temp);
									temp->ib_dev = dev;
									strncpy(temp->recv_url, r_u, 21);
									temp->ib_port = ib_port;
									temp->size = size;
									temp->tx_depth = tx_depth;
									temp->nodename = name;
									temp->remote_con_info = remote_qp;
                                    return temp;    
                                }


static int tcp_connect(char send_url[21]){
    int sock = nn_socket(AF_SP, NN_PAIR);
    int newSock = nn_connect(sock, send_url);
    if(newSock < 0){
        printf("Connect: Fatal Error\n");
        return -1;
    }
    return newSock;
}

static int tcp_bind(struct application_data* data){
	int sock = nn_socket(AF_SP, NN_PAIR);
    assert(sock >= 0);
    assert(nn_bind(sock, data->recv_url) >= 0);
	return sock;
}

static struct context* init_ctx(struct application_data* data){
    struct context *ctx;
    struct ibv_device **dev_list;
    ctx = (struct context *) malloc(sizeof *ctx);
    memset(ctx, 0, sizeof(&ctx));
    ctx->size = data->size;
    ctx->tx_depth = data->tx_depth;
    posix_memalign(&ctx->buf ,sysconf(_SC_PAGESIZE), ctx->size * 2);
    memset(ctx->buf, 0, ctx->size * 2);
    dev_list = ibv_get_device_list(NULL);
    if(!dev_list){
        printf("Fatal: Device List\n");
        return 0;
    }
	data->ib_dev = dev_list[0];
    ctx->context = ibv_open_device(data->ib_dev);
    if(!ctx->context){
        printf("Fatal: Context Error\n");
        return 0;
    }
    ctx->pd = ibv_alloc_pd(ctx->context);
    if(!ctx->pd){
        printf("Fatal: PD Error\n");
        return 0;
    }
    ctx->mr = ibv_reg_mr(ctx->pd,ctx->buf, ctx->size * 2
                        ,IBV_ACCESS_REMOTE_WRITE 
                        | IBV_ACCESS_LOCAL_WRITE
                        | IBV_ACCESS_REMOTE_READ);
    if(!ctx->mr){
        printf("Fatal: MR\n");
        return 0;
    }
    ctx->ch = ibv_create_comp_channel(ctx->context);
    if(!ctx->ch){
        printf("Fatal: CH\n");
        return 0;
    }
    //Check Parameters
    ctx->rcq = ibv_create_cq(ctx->context, 1, NULL
                ,NULL, 0);
    if(!ctx->rcq){
        printf("Fatal: RCQ\n");
        return 0;
    }
    ctx->scq = ibv_create_cq(ctx->context, ctx->tx_depth, NULL
                ,NULL, 0);
    if(!ctx->scq){
        printf("Fatal: SCQ\n");
        return 0;
    }

    struct ibv_qp_init_attr qp_init_attr = {
        .send_cq = ctx->scq,
        .recv_cq = ctx->rcq,
        .qp_type = IBV_QPT_RC,
        .cap = {
            .max_send_wr = ctx->tx_depth,
			.max_recv_wr = 1,
			.max_send_sge = 1,
			.max_recv_sge = 1,
			.max_inline_data = 0
        }
    };
    ctx->qp = ibv_create_qp(ctx->pd, &qp_init_attr);
    if(!ctx->qp){
        printf("Fatal: QP\n");
        return 0;
    }
    qp_to_init(ctx->qp, data);
    return ctx;
}

static void destroy_ctx(struct context *ctx);
static void set_local_con_attr(struct context *ctx, struct application_data *data){
    struct ibv_port_attr attr;
    ibv_query_port(ctx->context,data->ib_port,&attr);
    data->local_con_info.lid = attr.lid;
    data->local_con_info.qpn = ctx->qp->qp_num;
	data->local_con_info.psn = lrand48() & 0xffffff;
	data->local_con_info.rkey = ctx->mr->rkey;
	data->local_con_info.vaddr = (uintptr_t)ctx->buf + ctx->size;
}


static int tcp_exchange_qp_info(struct application_data *data){
    char msg[sizeof("0000:000000:000000:00000000:0000000000000000")];
    int parsed;
	int rc;
    struct connection_attr *local = &data->local_con_info;
    sprintf(msg, "%04x:%06x:%06x:%08x:%016Lx", 
				local->lid, local->qpn, local->psn, local->rkey, local->vaddr);
    while(rc != sizeof(msg)){
        rc = nn_send(data->sockfd, msg, sizeof(msg), NN_DONTWAIT);
    }
    if(rc != sizeof(msg)){
        rc = nn_recv(data->sockfd, msg, sizeof(msg), NN_DONTWAIT);
    }

    if(!data->remote_con_info){
        free(data->remote_con_info);
    }
    data->remote_con_info = malloc(sizeof(struct connection_attr));
    struct connection_attr *remote = data->remote_con_info;
    parsed = sscanf(msg, "%x:%x:%x:%x:%Lx", 
						&remote->lid, &remote->qpn, &remote->psn, &remote->rkey, &remote->vaddr);
    if(parsed != 5){
        printf("Error in Parsing\n");
        free(data->remote_con_info);
        return -1;
    }
    return 1;
}

void qp_to_init(struct ibv_qp *qp, struct application_data* data){
    struct ibv_qp_attr *attr;
    attr = malloc(sizeof(*attr));
    memset(attr,0,sizeof(*attr));
    attr->qp_state = IBV_QPS_INIT;
    attr->pkey_index = 0;
    attr->port_num = data->ib_port;
    attr->qp_access_flags = IBV_ACCESS_REMOTE_WRITE
                            | IBV_ACCESS_REMOTE_READ
                            | IBV_ACCESS_LOCAL_WRITE;

    ibv_modify_qp(qp, attr,
                            IBV_QP_STATE        |
                            IBV_QP_PKEY_INDEX   |
                            IBV_QP_PORT         |
                            IBV_QP_ACCESS_FLAGS);
}

/*
*Changes Queue Pair status to RTR (Ready to receive)
*/
static int qp_to_rtr(struct ibv_qp *qp, struct application_data *data){
    struct ibv_qp_attr *attr;

    attr =  malloc(sizeof *attr);
    memset(attr, 0, sizeof *attr);

    attr->qp_state              = IBV_QPS_RTR;
    attr->path_mtu              = IBV_MTU_2048;
    attr->dest_qp_num           = data->remote_con_info->qpn;
    attr->rq_psn                = data->remote_con_info->psn;
    attr->max_dest_rd_atomic    = 1;
    attr->min_rnr_timer         = 12;
    attr->ah_attr.is_global     = 0;
    attr->ah_attr.dlid          = data->remote_con_info->lid;
    attr->ah_attr.sl            = 1; //may have to change
    attr->ah_attr.src_path_bits = 0;
    attr->ah_attr.port_num      = data->ib_port;

    ibv_modify_qp(qp, attr,
                IBV_QP_STATE                |
                IBV_QP_AV                   |
                IBV_QP_PATH_MTU             |
                IBV_QP_DEST_QPN             |
                IBV_QP_RQ_PSN               |
                IBV_QP_MAX_DEST_RD_ATOMIC   |
                IBV_QP_MIN_RNR_TIMER);

	free(attr);
	
	return 0;
}

/*
 *  Changes Queue Pair status to RTS (Ready to send)
 *	QP status has to be RTR before changing it to RTS
*/
static int qp_to_rts(struct ibv_qp *qp, struct application_data *data){
    qp_to_rtr(qp,data);
    struct ibv_qp_attr *attr;

    attr =  malloc(sizeof *attr);
    memset(attr, 0, sizeof *attr);

	attr->qp_state              = IBV_QPS_RTS;
    attr->timeout               = 14;
    attr->retry_cnt             = 7;
    attr->rnr_retry             = 7;    /* infinite retry */
    attr->sq_psn                = data->local_con_info.psn;
    attr->max_rd_atomic         = 1;

    ibv_modify_qp(qp, attr,
                IBV_QP_STATE            |
                IBV_QP_TIMEOUT          |
                IBV_QP_RETRY_CNT        |
                IBV_QP_RNR_RETRY        |
                IBV_QP_SQ_PSN           |
                IBV_QP_MAX_QP_RD_ATOMIC);

    free(attr);

    return 0;
}

static int poll_cq(struct ibv_cq *cq, int num_completions){
    struct timespec end;
	struct timespec start;	
	struct ibv_wc *wc = (struct ibv_wc *) malloc(
		num_completions * sizeof(struct ibv_wc));
	int completions= 0, i = 0;
	clock_gettime(CLOCK_REALTIME, &start);
    while(completions < num_completions){
        int new_comps = ibv_poll_cq(cq, num_completions - completions, wc);
		if(new_comps != 0) {
			completions += new_comps;
			for(i = 0; i < new_comps; i++) {
				if(wc[i].status < 0) {
					perror("poll_recv_cq error");
					exit(0);
				}
			}
        }
        clock_gettime(CLOCK_REALTIME, &end);
		double seconds = (end.tv_sec - start.tv_sec) +
				(double) (end.tv_nsec - start.tv_nsec) / 1000000000;
		if (seconds>1){
            printf("Polling Completed"); 
            fflush(stdout); 
            break;
        }
    }
}

static int post_recv(struct context *ctx){
    int ret;
    struct ibv_sge list = {
        .addr = (uintptr_t)ctx->buf,
        .length = ctx->size,
        .lkey = ctx->mr->lkey,
    };
    struct ibv_recv_wr wr = {
        .next = &list,
        .num_sge = 1,
    };
    struct ibv_recv_wr *bad_wr;
    ret = ibv_post_recv(ctx->qp, &wr,&bad_wr);
    if(ret){
        printf("Failed to post\n");
        return -1;
    }
    else{
        printf("Posted");
    }
    return ret;
}

static int post_send(struct context *ctx, int opcode){

    struct ibv_recv_wr *bad_wr;
    ctx->sge_list.addr = (uintptr_t) ctx->buf;
    ctx->sge_list.lkey = ctx->mr->lkey;
    ctx->wr.opcode = IBV_WR_SEND;
    ctx->wr.send_flags |= IBV_SEND_SIGNALED;
    ctx->wr.sg_list = &ctx->sge_list;
    ctx->wr.sg_list->length = ctx->size;
    ctx->wr.num_sge = 1;
    int ret = ibv_post_send(ctx->qp, &ctx->wr, &bad_wr);
    return ret;
}

static void detroy_ctx(struct context *ctx){
    if(ctx->qp){
        free(ctx->qp);
    }
    if(ctx->mr){
        free(ctx->mr);
    }
    if(ctx->buf){
        free(ctx->buf);
    }
    if(ctx->rcq){
        free(ctx->rcq);
    }
    if(ctx->scq){
        free(ctx->scq);
    }
    if(ctx->context){
        ibv_close_device(ctx->context);
    }
}

struct udp_broadcast_context {
int sock;
struct sockaddr_in* sock_in;
};

static void heartbeat_cb(u16 sender_id, u8 len, u8 msg_[], void *context);

void udp_bridge_setup(sbp_zmq_rx_ctx_t *rx_ctx, sbp_zmq_tx_ctx_t *tx_ctx, struct udp_broadcast_context* context);

/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Gareth McMullin <gareth@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <libpiksi/sbp_zmq_pubsub.h>
#include <libpiksi/logging.h>
#include <libpiksi/util.h>
#include <libsbp/system.h>
#include <getopt.h>
#include "sbp_udp_bridge.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUF 65536

#define PROGRAM_NAME "sbp_udp_bridge"


static void heartbeat_cb(u16 sender_id, u8 len, u8 msg_[], void *context) {
  struct udp_broadcast_context* sbp_context = (struct udp_broadcast_context*) context;
  static int num_heartbeats;
  static char buffer[256];
  int buflen;
  int status;
  (void)sender_id;
  (void) len;
  msg_heartbeat_t *msg = (msg_heartbeat_t *) msg_;
  sprintf(buffer, "Received %d heartbeats. Currenet flags are %u", num_heartbeats, msg->flags);
  buflen = strlen(buffer);
  status = sendto(sbp_context->sock, buffer, buflen, 0, (struct sockaddr *) sbp_context->sock_in,  sizeof(sbp_context->sock_in));
  if (status != 0) printf("Error in sendto %d", status);
}

void udp_bridge_setup(sbp_zmq_rx_ctx_t *rx_ctx, sbp_zmq_tx_ctx_t *tx_ctx, struct udp_broadcast_context* context)
{
  sbp_zmq_rx_callback_register(rx_ctx, SBP_MSG_HEARTBEAT,
                               heartbeat_cb, tx_ctx, (void * ) context);
}

int main(int argc, char *argv[])
{
  logging_init(PROGRAM_NAME);
  int status; 
  /* Prevent czmq from catching signals */
  zsys_handler_set(NULL);
  int sock;  
  struct sockaddr_in sock_in;

  memset(&sock_in, 0, sizeof(struct sockaddr_in));

  sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in.sin_port = htons(0);
  sock_in.sin_family = PF_INET;

  status = bind(sock, (struct sockaddr *) &sock_in, sizeof(struct sockaddr_in));
  piksi_log(LOG_INFO,"Bind Status = %d\n", status);
  int broadcastPermission = 1;
  status = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(int) );
  piksi_log(LOG_INFO, "Setsockopt Status = %d\n", status);

  /* -1 = 255.255.255.255 this is a BROADCAST address,
     a local broadcast address could also be used.
     you can comput the local broadcat using NIC address and its NETMASK 
  */ 

  sock_in.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
  if (argc != 1) {
  printf("Error usage: sbp_upd_bridge PORT_NUMBER)");
  return -1;
  }
  sock_in.sin_port = htons(atoi(argv[1])); /* port number */
  sock_in.sin_family = PF_INET;
  struct udp_broadcast_context  cb_context;
  cb_context.sock = sock;
  cb_context.sock_in = &sock_in;
  sbp_zmq_pubsub_ctx_t *ctx = sbp_zmq_pubsub_create("tcp://127.0.0.1:43030", "tcp://127.0.0.1:43031");
  if (ctx == NULL) {
    exit(EXIT_FAILURE);
  }

  udp_bridge_setup(sbp_zmq_pubsub_rx_ctx_get(ctx),
                   sbp_zmq_pubsub_tx_ctx_get(ctx), &cb_context);

  zmq_simple_loop(sbp_zmq_pubsub_zloop_get(ctx));
  
  shutdown(sock, 2);
  close(sock);
  sbp_zmq_pubsub_destroy(&ctx);
  exit(EXIT_SUCCESS);
}

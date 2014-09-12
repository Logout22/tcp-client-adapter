#ifndef __NWFNS_H__
#define __NWFNS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>
#include <event2/event.h>
#include "tcpbridge_options.h"

typedef struct bridge_client {
    tcpbridge_address address;
    int server_socket;
    struct bufferevent *client_bev;
    struct bridge_client *opposite_client;
} bridge_client;

bridge_client *allocate_bridge_client();
void free_bridge_client(void *arg);

struct event_base *setup_network(tcpbridge_options *opts);
int bind_socket(char const *address, uint16_t const port, bool use_ipv6);

#endif


#ifndef __NWFNS_H__
#define __NWFNS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>
#include <event2/event.h>
#include "tcpbridge_options.h"

typedef struct bridge_client {
    int server_socket;
    struct bufferevent *client_bev;

    /* reference pointers: */
    struct event_base *evbase;
    tcpbridge_address *address;
    bool use_ipv6;
    struct bridge_client *opposite_client;
} bridge_client;

bridge_client *allocate_bridge_client(void);
void free_bridge_client(void *arg);

void initialise_clients(bridge_client **clients,
        struct event_base *evbase, tcpbridge_options *opts);
int establish_socket(tcpbridge_address *address, bool use_ipv6);
struct event_base *setup_network(tcpbridge_options *opts);

#endif


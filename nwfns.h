#ifndef __NWFNS_H__
#define __NWFNS_H__

#include <inttypes.h>
#include <stdbool.h>
#include "tcpbridge_types.h"

int bind_socket(char const *address, uint16_t const port, bool use_ipv6);
void connect_sockets(socket_pair *serversocks);
void tcpbridge_free_eventbase(void *base);
void tcpbridge_free_event(void *ev);

#endif


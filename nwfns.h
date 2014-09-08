#ifndef __NWFNS_H__
#define __NWFNS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>
#include <event2/event.h>
#include "tcpbridge_options.h"

struct event_base *setup_network(tcpbridge_options *opts);
int bind_socket(char const *address, uint16_t const port, bool use_ipv6);

#endif


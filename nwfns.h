#ifndef __NWFNS_H__
#define __NWFNS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>
#include <event2/event.h>
#include "tcpbridge_options.h"

typedef struct socket_pair {
    int sock1;
    int sock2;
} socket_pair;

struct event_base *setup_network(tcpbridge_options *opts);

#endif


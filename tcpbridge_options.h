#ifndef __TCPBRIDGE_OPTIONS_H__
#define __TCPBRIDGE_OPTIONS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>

typedef struct tcpbridge_address {
    char *conn_id_string;
    char *address_str;
    uint16_t port;
}

typedef struct tcpbridge_options {
    bool use_ipv6;
    tcpbridge_address connection_endpoints[2];
} tcpbridge_options;

tcpbridge_options *alloc_tcpbridge_options();
void free_tcpbridge_options(void *t);

char *usage_text(char const *progname);
tcpbridge_options *evaluate_options(int argc, char *argv[]);

#endif //__TCPBRIDGE_OPTIONS_H__


#ifndef __TCPBRIDGE_OPTIONS_H__
#define __TCPBRIDGE_OPTIONS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>

typedef struct tcpbridge_address {
    char *address_str;
    uint16_t port;
} tcpbridge_address;

tcpbridge_address *allocate_tcpbridge_address(void);
void free_tcpbridge_address(void *t);

#define NUMBER_OF_ENDPOINTS 2
typedef struct tcpbridge_options {
    bool use_ipv6;
    tcpbridge_address *connection_endpoints[NUMBER_OF_ENDPOINTS];
} tcpbridge_options;

tcpbridge_options *allocate_tcpbridge_options(void);
void free_tcpbridge_options(void *t);

char *usage_text(char const *progname);
tcpbridge_options *evaluate_options(int argc, char *argv[]);

#endif


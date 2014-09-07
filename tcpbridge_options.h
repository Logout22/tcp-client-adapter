#ifndef __TCPBRIDGE_OPTIONS_H__
#define __TCPBRIDGE_OPTIONS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>

typedef struct tcpbridge_options {
    bool use_ipv6;
    char *first_address_str;
    char *second_address_str;
    uint16_t first_port;
    uint16_t second_port;
} tcpbridge_options;

tcpbridge_options *alloc_tcpbridge_options();
void free_tcpbridge_options(void *t);

char *usage_text(char const *progname);
tcpbridge_options *evaluate_options(int argc, char *argv[]);

#endif //__TCPBRIDGE_OPTIONS_H__


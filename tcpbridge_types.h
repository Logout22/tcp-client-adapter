#ifndef __TCPBRIDGE_TYPES_H__
#define __TCPBRIDGE_TYPES_H__

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
void free_tcpbridge_options(void *target);

typedef struct socket_pair {
    int sock1;
    int sock2;
} socket_pair;

socket_pair *alloc_socket_pair();
void free_socket_pair(void *target);

#endif


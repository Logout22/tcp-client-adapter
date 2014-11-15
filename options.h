#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "common.h"
#include <inttypes.h>
#include <stdbool.h>

typedef struct tca_address {
    char *address_str;
    uint16_t port;
} tca_address;

tca_address *allocate_tca_address(void);
void free_tca_address(void *t);

#define NUMBER_OF_ENDPOINTS 2
typedef struct tca_options {
    bool use_ipv6;
    tca_address *connection_endpoints[NUMBER_OF_ENDPOINTS];
} tca_options;

tca_options *allocate_tca_options(void);
void free_tca_options(void *t);

char *usage_text(char const *progname);
tca_options *evaluate_options(int argc, char *argv[]);

#endif


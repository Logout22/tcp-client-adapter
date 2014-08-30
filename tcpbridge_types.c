#include "tcpbridge_types.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

tcpbridge_options *alloc_tcpbridge_options() {
    tcpbridge_options *result =
        (tcpbridge_options*) calloc(1, sizeof(tcpbridge_options));
    assert(result != NULL);

    result->use_ipv6 = false;
    result->first_address_str = NULL;
    result->second_address_str = NULL;
    result->first_port = 0;
    result->second_port = 0;
    return result;
}

void free_tcpbridge_options(void *t) {
    tcpbridge_options *target = (tcpbridge_options *) t;
    free(target->second_address_str);
    free(target->first_address_str);
    free(target);
}

socket_pair *alloc_socket_pair() {
    socket_pair *result =
        (socket_pair*) calloc(1, sizeof(socket_pair));
    assert(result != NULL);

    result->sock1 = -1;
    result->sock2 = -1;
    return result;
}

void free_socket_pair(void *t) {
    socket_pair *target = (socket_pair *) t;
    close(target->sock1);
    close(target->sock2);
    free(target);
}


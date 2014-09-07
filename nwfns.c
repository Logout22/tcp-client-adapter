#include "nwfns.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freeatexit.h"

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

int bind_socket(char const *address, uint16_t const port, bool use_ipv6) {
    struct addrinfo *addresses = NULL;
    struct addrinfo hints = {
        .ai_socktype = SOCK_STREAM,
        .ai_family = use_ipv6 ? AF_INET6 : AF_INET,
    };
    char port_str[6];
    snprintf(port_str, 6, "%d", port);
    int res;
    if ((res = getaddrinfo(address, port_str, &hints, &addresses)) != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(res));
        exit(EXIT_FAILURE);
    }

    int sock = -1;
    struct addrinfo *current_ai;
    for (current_ai = addresses;
            current_ai;
            current_ai = current_ai->ai_next) {
        sock = socket(
                current_ai->ai_family,
                current_ai->ai_socktype,
                current_ai->ai_protocol);
        if (sock < 0) {
            continue;
        }

        if (bind(sock, current_ai->ai_addr, current_ai->ai_addrlen) == 0) {
            res = listen(sock, 50);
            assert(res == 0);
            break;
        } else {
            warn("bind() failed");
        }
    }

    freeaddrinfo(addresses);

    if (!current_ai) {
        errx(ENOENT, "Could not bind to any address, exiting");
    }

    return sock;
}

void connect_sockets(struct event_base *evbase, socket_pair *serversocks) {  }

void tcpbridge_free_eventbase(void *base) {
    event_base_free((struct event_base*) base);
}

void tcpbridge_free_event(void *ev) {
    event_free((struct event*) ev);
}

struct event_base *setup_network(tcpbridge_options *opts) {
    struct event_base *evbase = event_base_new();
    if (!evbase) err(1, "Something something dark side");
    free_object_at_exit(tcpbridge_free_eventbase, evbase);

    socket_pair *server_sockets = alloc_socket_pair();
    free_object_at_exit(free_socket_pair, server_sockets);

    server_sockets->sock1 = bind_socket(
            opts->first_address_str,
            opts->first_port,
            opts->use_ipv6);
    server_sockets->sock2 = bind_socket(
            opts->second_address_str,
            opts->second_port,
            opts->use_ipv6);

    connect_sockets(evbase, server_sockets);

    return evbase;
}


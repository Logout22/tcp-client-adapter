#include "nwfns.h"
#include <stdio.h>
#include <err.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int bind_socket(char const *address, uint16_t const port, bool use_ipv6) {
    struct addrinfo *addresses = NULL;
    struct addrinfo hints = {
        .ai_socktype = SOCK_STREAM;
    };
    if (use_ipv6) {
        hints.ai_family = AF_INET6;
    } else {
        hints.ai_family = AF_INET;
    }
    char port_str[6];
    snprintf(port_str, 6, "%d", port);
    if (getaddrinfo(address, port_str, &hints, &addresses) != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int sock = -1;
    struct addrinfo *current_ai;
    for (current_ai = addresses;
            current_ai;
            current_ai = current_ai->next) {
        sock = socket(
                current_ai->ai_family,
                current_ai->ai_socktype,
                current_ai->ai_protocol);
        if (sock < 0) {
            continue;
        }

        if (bind(sock, current_ai->ai_addr, current_ai->ai_addrlen) == 0) {
            listen(sock, 1);
            break;
        }
    }

    freeaddrinfo(addresses);

    if (!current_ai) {
        errx("Could not bind to any of the supplied addresses, exiting");
    }

    return sock;
}

void connect_sockets(socket_pair *serversocks) { return 0; }

void tcpbridge_free_eventbase(void *base) {
    event_base_free((struct event_base*) base);
}

void tcpbridge_free_event(void *ev) {
    event_free((struct event*) ev);
}


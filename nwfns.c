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

void readcb(struct bufferevent *bev, void *ctx) {
    bufferevent_read_buffer(bev, bufferevent_get_output(sth ctx));
}

void writecb(struct bufferevent *bev, void *ctx) {
    bufferevent_write_buffer(bev, bufferevent_get_input(sth ctx));
}

void eventcb() {
    struct info *inf = ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    int finished = 0;

    if (events & BEV_EVENT_EOF) {
        size_t len = evbuffer_get_length(input);
        printf("Got a close from %s.  We drained %lu bytes from it, "
            "and have %lu left.\n", inf->name,
            (unsigned long)inf->total_drained, (unsigned long)len);
        finished = 1;
    }
    if (events & BEV_EVENT_ERROR) {
        printf("Got an error from %s: %s\n",
            inf->name, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = 1;
    }
    if (finished) {
        free(ctx);
        bufferevent_free(bev);
    }
}

void new_client_cb(evutil_socket_t sock1, short what, void *s2) {
    buffered_socket_pair *bla = allocate...
    struct bufferevent **other_sock = (struct bufferevent **) s2;
    int cltsock = accept(sock1);
    assert(cltsock >= 0);
    free_object_at_exit(free_socket, cltsock);
    evutil_make_socket_nonblocking(cltsock);

    struct bufferevent *clt_bufevent =
        bufferevent_socket_new(
                evbase,
                cltsock,
                BEV_OPT_CLOSE_ON_FREE |  BEV_OPT_DEFER_CALLBACKS);
    free_object_at_exit(tcpbridge_free_bufferevent, clt_bufevent);
    bufferevent_setcb(bev, readcb, writecb, eventcb, newstruct);
    bufferevent_enable(bev, EV_READ|EV_WRITE);

}

void connect_sockets(struct event_base *evbase, socket_pair *serversocks) {
    evutil_make_socket_nonblocking(serversock);
    struct event *sock_event = event_new(evbase, serversock, EV_READ,
            new_client_cb, (void*) other_server_sock);
    free_object_at_exit(tcpbridge_free_event, sock_event);
    event_add(sock_event, NULL);
    serversocks->sock1
}

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


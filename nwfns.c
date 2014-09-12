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

void tcpbridge_free_eventbase(void *base);
void tcpbridge_free_event(void *ev);

bridge_client *allocate_bridge_client() {
    bridge_client *result = (bridge_client*) calloc(1, sizeof(bridge_client));
    assert(result != NULL);

    result->conn_id_string = NULL;
    result->server_socket = -1;
    result->client_bev = NULL;
    result->opposite_client = NULL;

    return result;
}

void free_bridge_client(void *arg) {
    bridge_client *client = (bridge_client*) arg;

    bufferevent_free(client->client_bev);
    close(client->server_socket);
    free(client->conn_id_string);
    free(client);
}

void connect_clients(struct event_base *evbase);
void setup_client(struct event_base evbase, bridge_client *client);

struct event_base *setup_network(tcpbridge_options *opts) {
    struct event_base *evbase = event_base_new();
    assert(evbase != NULL);
    free_object_at_exit(tcpbridge_free_eventbase, evbase);

    connect_clients(evbase, opts);

    return evbase;
}

void connect_clients(struct event_base *evbase, tcpbridge_options *opts) {
    bridge_client *client1 = allocate_bridge_client();
    free_object_at_exit(free_bridge_client, client1);
    bridge_client *client2 = allocate_bridge_client();
    free_object_at_exit(free_bridge_client, client2);

    client1->opposite_client = client2;
    client2->opposite_client = client1;

    setup_client(evbase, client1, opts);
    setup_client(evbase, client2, opts);
}

int bind_socket(char const *address, uint16_t const port, bool use_ipv6);
void register_server_callback(
        struct event_base *evbase, bridge_client *client);

void setup_client(struct event_base evbase, bridge_client *client) {
    asprintf(client->conn_id_string,
    client->server_socket = bind_socket(
            opts->first_address_str,
            opts->first_port,
            opts->use_ipv6);

    register_server_callback(evbase, client);
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

void new_client_cb(evutil_socket_t sock1, short what, void *s2);

void register_server_callback(
        struct event_base *evbase, bridge_client *client) {
    evutil_make_socket_nonblocking(client->server_socket);
    struct event *sock_event = event_new(
            evbase,
            client->server_socket,
            EV_READ,
            new_client_cb, (void*) client);
    assert(sock_event);
    free_object_at_exit(tcpbridge_free_event, sock_event);
    event_add(sock_event, NULL);
}

void readcb(struct bufferevent *bev, void *ctx);
void writecb(struct bufferevent *bev, void *ctx);
void eventcb(struct bufferevent *bev, short error, void *ctx);

void new_client_cb(evutil_socket_t sock1, short what, void *arg) {
    bridge_client *client = (bridge_client*) arg;

    int cltsock = accept(sock1);
    if (cltsock < 0) {
        err(errno, "accept");
    } else if (cltsock > FD_SETSIZE) {
        errx(ENOENT, "FD_SETSIZE exceeded. "
                    "You are likely target of a DoS attack. Exiting.");
    }
    evutil_make_socket_nonblocking(cltsock);

    client->client_bev = bufferevent_socket_new(evbase, cltsock,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    bufferevent_setcb(bev, readcb, writecb, eventcb, (void*) client);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

void readcb(struct bufferevent *bev, void *ctx) {
    bridge_client *client = (bridge_client*) ctx;
    assert(client->client_bev == bev);
    bufferevent_read_buffer(bev,
            bufferevent_get_output(client->opposite_client->client_bev));
}

void writecb(struct bufferevent *bev, void *ctx) {
    bridge_client *client = (bridge_client*) ctx;
    assert(client->client_bev == bev);
    bufferevent_read_write(bev,
            bufferevent_get_input(client->opposite_client->client_bev));
}

void eventcb(struct bufferevent *bev, short error, void *ctx) {
    bridge_client *inf = (bridge_client*) ctx;

    if (events & BEV_EVENT_EOF) {
        errx(0, "Connection closed.");
    }
    if (events & BEV_EVENT_ERROR) {
        printf("Got an error from %s: %s\n",
            inf->name, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = 1;
    }
}

void tcpbridge_free_eventbase(void *base) {
    event_base_free((struct event_base*) base);
}

void tcpbridge_free_event(void *ev) {
    event_free((struct event*) ev);
}


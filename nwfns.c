#include "nwfns.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <event2/bufferevent.h>

#include "freeatexit.h"

#define PORT_STR_LEN 6

void tcpbridge_free_eventbase(void *base);
void tcpbridge_free_event(void *ev);

bridge_client *allocate_bridge_client() {
    ALLOCATE(bridge_client, result);

    result->server_socket = -1;

    return result;
}

void free_bridge_client(void *arg) {
    bridge_client *client = (bridge_client*) arg;

    if (client->client_bev) {
        bufferevent_free(client->client_bev);
    }
    if (client->server_socket >= 0) {
        close(client->server_socket);
    }

    free(client);
}

void initialise_clients(bridge_client **clients,
        struct event_base *evbase, tcpbridge_options *opts);
void connect_clients(bridge_client **clients);

struct event_base *setup_network(tcpbridge_options *opts) {
    struct event_base *evbase = event_base_new();
    assert(evbase != NULL);
    free_object_at_exit(tcpbridge_free_eventbase, evbase);

    bridge_client *clients[NUMBER_OF_ENDPOINTS];
    initialise_clients(clients, evbase, opts);
    connect_clients(clients);

    return evbase;
}

void initialise_clients(bridge_client **clients,
        struct event_base *evbase, tcpbridge_options *opts) {
    int i;
    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        clients[i] = allocate_bridge_client();
        free_object_at_exit(free_bridge_client, clients[i]);
    }

    /* NOTE: change this when NUMBER_OF_ENDPOINTS changes */
    clients[0]->opposite_client = clients[1];
    clients[1]->opposite_client = clients[0];
    /* END NOTE */

    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        clients[i]->evbase = evbase;
        clients[i]->address = opts->connection_endpoints[i];
        clients[i]->use_ipv6 = opts->use_ipv6;
    }
}

void register_server_callback(bridge_client *client);

void connect_clients(bridge_client **clients) {
    int i;

    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        clients[i]->server_socket = establish_socket(
                clients[i]->address, clients[i]->use_ipv6);
        register_server_callback(clients[i]);
    }
}

struct addrinfo *lookup_address(tcpbridge_address *address, bool use_ipv6);
int create_socket(struct addrinfo *address);
void show_socket_warning(struct addrinfo *address);
bool bind_socket(int sock, struct addrinfo *address);
void show_bind_warning(struct addrinfo *address);

int establish_socket(tcpbridge_address *address, bool use_ipv6) {
    int sock;

    struct addrinfo *addresses = lookup_address(address, use_ipv6);
    while (addresses) {
        sock = create_socket(addresses);
        if (sock < 0) {
            show_socket_warning(addresses);
            continue;
        }
        if (bind_socket(sock, addresses)) {
            break;
        }

        show_bind_warning(addresses);
        addresses = addresses->ai_next;
    }

    if (addresses == NULL) {
        errx(ENOENT, "Could not bind to any address, exiting");
    }

    return sock;
}

void tcpbridge_free_addrinfo(void *info);

struct addrinfo *lookup_address(tcpbridge_address *address, bool use_ipv6) {
    struct addrinfo *result = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = use_ipv6 ? AF_INET6 : AF_INET;
    char port_str[PORT_STR_LEN];
    snprintf(port_str, PORT_STR_LEN, "%d", address->port);
    int res;
    if ((res = getaddrinfo(
                    address->address_str,
                    port_str,
                    &hints,
                    &result)) != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(res));
        exit(EXIT_FAILURE);
    }
    free_object_at_exit(tcpbridge_free_addrinfo, result);

    return result;
}

int create_socket(struct addrinfo *address) {
    return socket(
            address->ai_family,
            address->ai_socktype,
            address->ai_protocol);
}

void show_socket_warning(struct addrinfo *address) {
    warn("socket() failed: family %d, type %d, protocol %d",
            address->ai_family,
            address->ai_socktype,
            address->ai_protocol);
}

bool bind_socket(int sock, struct addrinfo *address) {
    if (bind(sock, address->ai_addr, address->ai_addrlen) != 0) {
        return false;
    }

    if (listen(sock, 50) != 0) {
        err(errno, "listen() failed");
    }

    evutil_make_socket_nonblocking(sock);
    return true;
}

char const *convert_address(struct addrinfo *address, char *dest);
void convert_port(struct addrinfo *address, char *dest);

void show_bind_warning(struct addrinfo *address) {
    char address_dump[INET6_ADDRSTRLEN];
    if (convert_address(address, address_dump) != address_dump) {
        err(errno, "Could not convert IP address to string");
    }
    char port_str[PORT_STR_LEN];
    convert_port(address, port_str);

    warn("bind() failed on %s:%s", address_dump, port_str);
}

char const *convert_address(struct addrinfo *address, char *dest) {
    char const *check_ptr = NULL;
    if (address->ai_family == AF_INET) {
        check_ptr = inet_ntop(
                address->ai_family,
                &((struct sockaddr_in*) address->ai_addr)->sin_addr.s_addr,
                dest, INET6_ADDRSTRLEN);
    } else if (address->ai_family == AF_INET6) {
        check_ptr = inet_ntop(
                address->ai_family,
                &((struct sockaddr_in6*) address->ai_addr)->sin6_addr.s6_addr,
                dest, INET6_ADDRSTRLEN);
    }
    return check_ptr;
}

void convert_port(struct addrinfo *address, char *dest) {
    int written = 0;
    if (address->ai_family == AF_INET) {
        written = snprintf(dest, PORT_STR_LEN, "%d",
                ntohs(((struct sockaddr_in*) address->ai_addr)->sin_port));
    } else if (address->ai_family == AF_INET6) {
        written = snprintf(dest, PORT_STR_LEN, "%d",
                ntohs(((struct sockaddr_in6*) address->ai_addr)->sin6_port));
    }
    assert(written > 0);
}

void new_client_cb(evutil_socket_t sock1, short what, void *s2);

void register_server_callback(bridge_client *client) {
    struct event *sock_event = event_new(
            client->evbase,
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

    assert(what == EV_READ);
    int cltsock = accept(sock1, NULL, NULL);
    if (cltsock < 0) {
        err(errno, "accept");
    } else if (cltsock > FD_SETSIZE) {
        errx(ENOENT, "FD_SETSIZE exceeded. "
                    "You are likely target of a DoS attack. Exiting.");
    }
    evutil_make_socket_nonblocking(cltsock);

    client->client_bev = bufferevent_socket_new(client->evbase, cltsock,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    assert(client->client_bev);
    bufferevent_setcb(
            client->client_bev,
            readcb, writecb, eventcb,
            (void*) client);
    bufferevent_enable(client->client_bev, EV_READ|EV_WRITE);
}

void readcb(struct bufferevent *bev, void *ctx) {
    bridge_client *client = (bridge_client*) ctx;
    assert(client->client_bev == bev);
    if (client->opposite_client->client_bev) {
        bufferevent_read_buffer(bev,
                bufferevent_get_output(client->opposite_client->client_bev));
    }
}

void writecb(struct bufferevent *bev, void *ctx) {
    bridge_client *client = (bridge_client*) ctx;
    assert(client->client_bev == bev);
    if (client->opposite_client->client_bev) {
        bufferevent_write_buffer(bev,
                bufferevent_get_input(client->opposite_client->client_bev));
    }
}

void eventcb(struct bufferevent *bev, short error, void *ctx) {
    bridge_client *client = (bridge_client*) ctx;
    assert(client->client_bev == bev);

    if (error & BEV_EVENT_EOF) {
        warnx("Connection closed on %s:%d.",
                client->address->address_str,
                client->address->port);
    }
    if (error & BEV_EVENT_ERROR) {
        errx(errno, "Got an error from %s:%d: %s\n",
            client->address->address_str,
            client->address->port,
            evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
    }
}

void tcpbridge_free_addrinfo(void *info) {
    freeaddrinfo((struct addrinfo*) info);
}

void tcpbridge_free_eventbase(void *base) {
    event_base_free((struct event_base*) base);
}

void tcpbridge_free_event(void *ev) {
    event_free((struct event*) ev);
}


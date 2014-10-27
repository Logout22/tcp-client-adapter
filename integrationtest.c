#include "tcpbridge_options.h"
#include "tcpbridge_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

#include <unistd.h>

typedef struct test_client {
    int sock;
    struct sockaddr_in6 connection_address;
} test_client;

typedef struct integtest_fixture {
    tcpbridge_options *opts;
    struct event_base *evbase;
    test_client *client[2];
} integtest_fixture;

static integtest_fixture fixture;

int create_test_socket(void) {
    int result = socket(AF_INET, SOCK_STREAM, 0);
    if (result < 0) {
        err(errno, "Could not create socket");
    }
    return result;
}

void init_test_clients(test_client **clients) {
    int i;
    for (i = 0; i < 2; i++) {
        clients[i] = calloc(1, sizeof(test_client));
        clients[i]->sock = create_test_socket();

        memset(&clients[i]->connection_address, 0,
                sizeof(clients[i]->connection_address));
        clients[i]->connection_address.sin6_family = AF_INET6;
        clients[i]->connection_address.sin6_port =
            htons(fixture.opts->connection_endpoints[source]->port);
        inet_pton(AF_INET6,
                fixture.opts->connection_endpoints[source]->address_str,
                &clients[i]->connection_address.sin6_addr);
    }
}

void teardown_test_clients(test_client **clients) {
    int i;
    for (i = 0; i < 2; i++) {
        if (clients[i]->sock >= 0) {
            close(clients[i]->sock);
        }
        free(clients[i]);
    }
}

tcpbridge_options *init_tcpbridge_options(void) {
    tcpbridge_options *result = allocate_tcpbridge_options();
    result->use_ipv6 = true;
    result->connection_endpoints[0]->address = strdup("::1");
    result->connection_endpoints[0]->port = 33326;
    result->connection_endpoints[1]->address = strdup("::1");
    result->connection_endpoints[1]->port = 37831;
    return result;
}

void initialise_test(void) {
    fixture.ops = init_tcpbridge_options();
    fixture.evbase = setup_network(global_opts);
    init_test_clients(fixture.client);
}

void teardown_fixture(void) {
    teardown_test_clients(fixture.client);
    free_tcpbridge_options(fixture.ops);
    free_atexit();
}

void do_fork(void) {
    pid_t process_id = fork();
    if (process_id < 0) {
        err(errno, "Error while forking");
    }
    return process_id;
}

void run_clients(void);

void run_test(void) {
    pid_t process_id = do_fork();
    if (process_id == 0) {
        run_clients();
        exit(0);
    }
}

int get_subprocess_result(void);
void connect_client(test_client *client, int id);
void send_message(test_client *client, int msgid);
void receive_message(test_client *client, int msgid);

void run_clients() {
    pid_t process_id = do_fork();

    int source, target;
    bool need_to_wait = true;

    if (process_id == 0) {
        source = 0;
        target = 1
        need_to_wait = false;
    } else {
        source = 1;
        target = 0;
    }

    test_client *source_client = fixture.client[source];

    connect_client(source_client, source);

    send_message(source_client, source);
    receive_message(source_client, target);
    send_message(source_client, 10 + source);
    receive_message(source_client, 10 + target);

    if (need_to_wait) {
        int subproc_result = get_subprocess_result();
        if (subproc_result != 0) {
            exit(subproc_result);
        }
    } else {
        exit(0);
    }
}

void connect_client(test_client *client, int id) {
    int res = connect(client.sock,
            client.connection_address, sizeof(client.connection_address));
    if (res != 0) {
        err(errno, "connect client %d failed", source);
    }
}

void send_message(test_client *client, int msgid) {
    char message[10];
    snprintf(message, 10, "Message %d", msgid);

    ssize_t bytes_to_send = strlen(message), sent_bytes;
    sent_bytes = send(client->sock, message, bytes_to_send, 0);

    if (sent_bytes != bytes_to_send) {
        err(errno, "send(message %d) delivered %d instead of %d bytes",
                msgid, sent_bytes, bytes_to_send);
    }
}

void receive_message(test_client *client, int msgid) {
    char received_message[10];
    memset(received_message, 0, 10);
    ssize_t received_bytes = recv(client->sock, received_message, 10, 0);
    if (received_bytes <= 0) {
        err(errno, "Could not receive message %d", msgid);
    }

    char expected_message[10];
    snprintf(expected_message, 10, "Message %d", msgid);
    if (strcmp(received_message, expected_message) != 0) {
        err(errno, "received %s instead of %s",
                received_message, expected_message);
    }
}

int get_subprocess_result() {
    int subproc_exit = 1;
    wait(&subproc_exit);
    return subproc_exit;
}

int main(int argc, char *argv[]) {
    atexit(teardown_fixture);
    register_signal_handler();

    initialise_test();
    run_test();
    event_base_dispatch(evbase);

    int subproc_result = get_subprocess_result();
    if (subproc_result != 0) {
        exit(subproc_result);
    }

    return 0;
}


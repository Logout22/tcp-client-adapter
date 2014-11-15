#include "options.h"
#include "tca_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <err.h>
#include <errno.h>

#include <sys/wait.h>

#include <arpa/inet.h>

typedef struct integtest_fixture {
    tca_options *opts;
    struct event_base *evbase;
} integtest_fixture;

static integtest_fixture fixture;

typedef struct test_client {
    int sock;
#ifdef HAVE_IPV6
    struct sockaddr_in6 connection_address;
#else
    struct sockaddr_in connection_address;
#endif
} test_client;

tca_options *init_tca_options(void) {
    tca_options *result = allocate_tca_options();
#ifdef HAVE_IPV6
    result->use_ipv6 = true;
    result->connection_endpoints[0]->address_str = strdup("::1");
    result->connection_endpoints[1]->address_str = strdup("::1");
#else
    result->use_ipv6 = false;
    result->connection_endpoints[0]->address_str = strdup("127.0.0.1");
    result->connection_endpoints[1]->address_str = strdup("127.0.0.1");
#endif
    result->connection_endpoints[0]->port = 33326;
    result->connection_endpoints[1]->port = 37831;
    return result;
}

void initialise_test(void) {
    memset(&fixture, 0, sizeof(fixture));
    fixture.opts = init_tca_options();
    fixture.evbase = setup_network(fixture.opts);
}

void teardown_fixture(void) {
    if (fixture.opts) {
        free_tca_options(fixture.opts);
    }
    free_atexit();
}

pid_t do_fork(void) {
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

test_client *init_test_client(int clientid);
void connect_client(test_client *client, int id);
void send_message(test_client *client, int clientid, int msgid);
void receive_message(test_client *client, int clientid, int msgid);
void teardown_test_client(test_client *client);
int get_subprocess_result(void);

void run_clients() {
    pid_t process_id = do_fork();

    int source, target;
    bool need_to_wait = true;

    if (process_id == 0) {
        source = 0;
        target = 1;
        need_to_wait = false;
    } else {
        source = 1;
        target = 0;
    }

    test_client *source_client = init_test_client(source);

    connect_client(source_client, source);

    send_message(source_client, source, source);
    receive_message(source_client, source, target);
    send_message(source_client, source, 10 + source);
    receive_message(source_client, source, 10 + target);

    teardown_test_client(source_client);

    if (need_to_wait) {
        int subproc_result = get_subprocess_result();
        if (subproc_result != 0) {
            exit(subproc_result);
        }
    } else {
        exit(0);
    }
}

int create_test_socket(void) {
    int result;

#ifdef HAVE_IPV6
    result = socket(AF_INET6, SOCK_STREAM, 0);
#else
    result = socket(AF_INET, SOCK_STREAM, 0);
#endif

    if (result < 0) {
        err(errno, "Could not create socket");
    }
    return result;
}

char const *get_connection_address(int clientid);
uint16_t get_connection_port(int clientid);

#ifdef HAVE_IPV6
void fill_connection_address(
        int clientid, struct sockaddr_in6 *connection_address) {
    memset(connection_address, 0, sizeof(struct sockaddr_in6));

    connection_address->sin6_family = AF_INET6;
    connection_address->sin6_port = get_connection_port(clientid);
    inet_pton(AF_INET6,
            get_connection_address(clientid),
            &connection_address->sin6_addr);
}
#else
void fill_connection_address(
        int clientid, struct sockaddr_in *connection_address) {
    memset(connection_address, 0, sizeof(struct sockaddr_in));

    connection_address->sin_family = AF_INET;
    connection_address->sin_port = get_connection_port(clientid);
    inet_pton(AF_INET,
            get_connection_address(clientid),
            &connection_address->sin_addr);
}
#endif

test_client *init_test_client(int clientid) {
    test_client *result = calloc(1, sizeof(test_client));
    result->sock = create_test_socket();
    fill_connection_address(clientid, &result->connection_address);

    return result;
}

char const *get_connection_address(int clientid) {
    return fixture.opts->connection_endpoints[clientid]->address_str;
}

uint16_t get_connection_port(int clientid) {
    return htons(fixture.opts->connection_endpoints[clientid]->port);
}

void connect_client(test_client *client, int id) {
    int res = connect(
            client->sock,
            (struct sockaddr*) &client->connection_address,
            sizeof(client->connection_address));
    if (res != 0) {
        err(errno, "connect client %d failed", id);
    }
}

#define MAX_MESSAGE_LENGTH 15

void send_message(test_client *client, int clientid, int msgid) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, MAX_MESSAGE_LENGTH, "Message %d", msgid);
    printf("Client %d: send_message %s\n", clientid, message);

    size_t bytes_to_send = strlen(message);
    assert(bytes_to_send < MAX_MESSAGE_LENGTH);
    ssize_t sent_bytes = send(
            client->sock, message, bytes_to_send, 0);

    if (sent_bytes != ((ssize_t) bytes_to_send)) {
        err(errno, "send(message %d) delivered %"PRIdPTR
                " instead of %"PRIdPTR" bytes",
                msgid, sent_bytes, bytes_to_send);
    }
}

void receive_message(test_client *client, int clientid, int msgid) {
    char received_message[MAX_MESSAGE_LENGTH];
    memset(received_message, 0, MAX_MESSAGE_LENGTH);
    ssize_t received_bytes = recv(
            client->sock, received_message, MAX_MESSAGE_LENGTH, 0);
    if (received_bytes <= 0) {
        err(errno, "Could not receive message %d", msgid);
    }
    printf("Client %d received message %s\n", clientid, received_message);

    char expected_message[MAX_MESSAGE_LENGTH];
    snprintf(expected_message, MAX_MESSAGE_LENGTH, "Message %d", msgid);
    if (strcmp(received_message, expected_message) != 0) {
        errx(1, "received %s instead of %s; errno: %d",
                received_message, expected_message, errno);
    }
}

void teardown_test_client(test_client *client) {
    if (client->sock >= 0) {
        close(client->sock);
    }
    free(client);
}

int get_subprocess_result() {
    int subproc_exit = 1;
    wait(&subproc_exit);
    return subproc_exit;
}

int main(int argc, char *argv[]) {
    (void) argc, (void) argv;

    atexit(teardown_fixture);
    register_signal_handler();

    initialise_test();
    run_test();
    event_base_dispatch(fixture.evbase);

    int subproc_result = get_subprocess_result();
    if (subproc_result != 0) {
        exit(subproc_result);
    }

    return 0;
}


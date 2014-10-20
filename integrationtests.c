#include "tcpbridge_options.h"
#include "tcpbridge_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

typedef struct test_client {
} test_client;

typedef struct integtest_fixture {
    tcpbridge_options *opts;
    struct event_base *evbase;
    test_client *client[2];
} integtest_fixture;

integtest_fixture *initialise_tests() {
    integtest_fixture *result = calloc(1, sizeof(result));
    result->ops = allocate_tcpbridge_options();
    result->evbase = NULL;
    // TODO init test clients
    return result;
}

void teardown_fixture(integtest_fixture *fixture) {
    // TODO free clients
    free_tcpbridge_options(fixture->ops);
    free(fixture);
}

int run_tests(integtest_fixture *fixture) {
    fixture->evbase = setup_network(global_opts);
    event_base_dispatch(evbase);
}

int main(int argc, char *argv[]) {
    atexit(free_atexit);
    register_signal_handler();

    integtest_fixture *fixture = initialise_tests();
    int result = run_tests(fixture);
    teardown_fixture(fixture);

    return result;
}


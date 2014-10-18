#include <stdlib.h>

#include "tcpbridge_options.h"
#include "tcpbridge_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

int main(int argc, char *argv[]) {
    atexit(free_atexit);

    tcpbridge_options *global_opts = evaluate_options(argc, argv);

    register_signal_handler();

    struct event_base *evbase = setup_network(global_opts);
    event_base_dispatch(evbase);

    return 0;
}


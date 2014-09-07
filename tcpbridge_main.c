#include <stdlib.h>

#include "tcpbridge_options.h"
#include "tcpbridge_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

int main(int argc, char *argv[]) {
    atexit(free_atexit);

    /* first, check if we have everything we need to proceed,
     * and quit if we have not
     */
    tcpbridge_options *global_opts = evaluate_options(argc, argv);

    // all OK, now go on with the actual program
    register_signal_handler();

    struct event_base *evbase = setup_network(global_opts);
    // transfer data until interrupted by signal
    event_base_dispatch(evbase);

    return 0;
}


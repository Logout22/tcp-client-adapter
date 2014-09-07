#include "tcpbridge_signal.h"

#include <stdlib.h>
#include <signal.h>

void sig_handler(int signal) {
    // just exit regularly when interrupted
    exit(0);
}

void register_signal_handler() {
	struct sigaction sighdl = {
		.sa_handler = sig_handler
	};
	sigaction(SIGTERM, &sighdl, NULL);
	sigaction(SIGHUP, &sighdl, NULL);
	sigaction(SIGINT, &sighdl, NULL);
}


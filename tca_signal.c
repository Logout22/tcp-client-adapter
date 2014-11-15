#include "tca_signal.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>

void sig_handler(int signal) {
    (void) signal;
    exit(0);
}

void register_signal_handler() {
	struct sigaction sighdl;
    memset(&sighdl, 0, sizeof(sighdl));
	sighdl.sa_handler = sig_handler;

	sigaction(SIGTERM, &sighdl, NULL);
	sigaction(SIGHUP, &sighdl, NULL);
	sigaction(SIGINT, &sighdl, NULL);
}


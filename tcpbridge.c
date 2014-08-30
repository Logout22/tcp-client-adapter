/* This is TCPBRIDGE by Martin Unzner.
   You should have received a README.md file,
   see there for more information
*/

// System headers
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <err.h>
#include <errno.h>
#include <signal.h>

// Libevent
#include <event2/event.h>

// Local headers
#include "tcpbridge_macros.h"
#include "tcpbridge_types.h"
#include "nwfns.h"
#include "freeatexit.h"

// Global variables
static struct option const long_opts[] = {
    { "help", no_argument, 0, 'h', },
    { "first-address", required_argument, 0, 'a', },
    { "first-port", required_argument, 0, 'p', },
    { "second-address", required_argument, 0, 'b', },
    { "second-port", required_argument, 0, 'q', },
};

// Global functions
void usage(char const *progname) {
    printf("Creates two TCP server sockets, waits for an incoming connection\n"
      "on each of them and then sends data back and forth\n"
      "between the sockets until it is terminated.\n"
      "\n"
      "Useful if you want to connect two clients with each other,\n"
      "e.g. listening VNC viewer and VNC client.\n"
      "\n"
      "Usage: %s <options>\n"
      "\n"
      "-h, --help\t\tShow this help text\n"
      "-a, --first-address\tThe first address to listen on (host or IP)\n"
      "-p, --first-port\tThe first TCP port to listen on\n"
      "-b, --second-address\tThe second address to listen on (host or IP)\n"
      "-q, --second-port\tThe second TCP port to listen on\n"
      "-6\t\t\tUse IPv6\n"
      "\n"
      "-p and -q are required.\n"
      ,progname);
    exit(0);
}

void sig_handler(int signal);

bool get_port(char const *arg, uint16_t *out_port);
tcpbridge_options *evaluate_options(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    atexit(free_atexit);

    /* first, check if we have everything we need to proceed,
     * and quit if we have not
     */
    tcpbridge_options *global_opts;
    global_opts = evaluate_options(argc, argv);

    // all OK, now go on with the actual program
	struct sigaction sighdl = {
		.sa_handler = sig_handler
	};
	sigaction(SIGTERM, &sighdl, NULL);
	sigaction(SIGHUP, &sighdl, NULL);
	sigaction(SIGINT, &sighdl, NULL);

    struct event_base *evbase = event_base_new();
    free_object_at_exit(tcpbridge_free_eventbase, evbase);

    socket_pair *server_sockets = alloc_socket_pair();
    free_object_at_exit(free_socket_pair, server_sockets);

    server_sockets->sock1 = bind_socket(
            global_opts->first_address_str,
            global_opts->first_port,
            global_opts->use_ipv6);
    server_sockets->sock2 = bind_socket(
            global_opts->second_address_str,
            global_opts->second_port,
            global_opts->use_ipv6);

    socket_pair *client_sockets = connect_sockets(server_sockets);

    // transfer data until interrupted by signal
    event_base_dispatch(evbase);

    return 0;
}

tcpbridge_options *evaluate_options(int argc, char *argv[]) {
    tcpbridge_options *result;
    result = alloc_tcpbridge_options();
    free_object_at_exit(free_tcpbridge_options, result);

    int selected_option;
    while ((selected_option = getopt_long(
                    argc, argv, "6ha:b:p:q:", long_opts, NULL)) != -1) {
        if (selected_option == '6') {
            result->use_ipv6 = 1;
        } else if (selected_option == 'a') {
            result->first_address_str = strdup(optarg);
        } else if (selected_option == 'b') {
            result->second_address_str = strdup(optarg);
        } else if (selected_option == 'p') {
            if (!get_port(optarg, &result->first_port)) {
                errx(EINVAL, "First port invalid: %s", optarg);
            }
        } else if (selected_option == 'q') {
            if (!get_port(optarg, &result->second_port)) {
                errx(EINVAL, "Second port invalid: %s", optarg);
            }
        } else if (selected_option == 'h') {
            usage(argv[0]);
        } else if (selected_option == '?') {
            errx(EINVAL, "Error: Unrecognised option or missing arguments");
        } else {
            err(errno, "Error while parsing options");
        }
    }

    // check requirements and set defaults where applicable
    if (result->first_port == 0 || result->second_port == 0) {
        errx(ENOENT, "Ports are required for forwarding; exiting");
    }
    if (result->first_address_str == NULL) {
        result->first_address_str = DEFAULT_ADDRESS;
    }
    if (result->second_address_str == NULL) {
        result->second_address_str = DEFAULT_ADDRESS;
    }

    return result;
}

bool get_port(char const *arg, uint16_t *out_port) {
    char *check_ptr;
    // base autodetect -> also accepts oct / hex values
    long conv_result = strtol(arg, &check_ptr, 0);
    /* either it did not convert the string at all
       or the result is not within TCP port range */
    if (arg == check_ptr || conv_result <= 0 || conv_result > 65535) {
        return false;
    }
    // safe to cast now
    *out_port = (uint16_t) conv_result;
    return true;
}

void sig_handler(int signal) {
    // just exit regularly when interrupted
    exit(0);
}


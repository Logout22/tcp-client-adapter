#define _GNU_SOURCE
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <err.h>
#include <errno.h>

#include "options.h"
#include "freeatexit.h"

tca_address *allocate_tca_address() {
    ALLOCATE(tca_address, result);
    return result;
}

void free_tca_address(void *t) {
    tca_address *target = (tca_address*) t;
    free(target->address_str);
    free(target);
}

tca_options *allocate_tca_options() {
    ALLOCATE(tca_options, result);

    int i;
    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        result->connection_endpoints[i] = allocate_tca_address();
    }
    return result;
}

void free_tca_options(void *t) {
    tca_options *target = (tca_options *) t;
    int i;
    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        free_tca_address(target->connection_endpoints[i]);
    }
    free(target);
}

static struct option const long_opts[] = {
    { "help", no_argument, 0, 'h', },
    { "first-address", required_argument, 0, 'a', },
    { "first-port", required_argument, 0, 'p', },
    { "second-address", required_argument, 0, 'b', },
    { "second-port", required_argument, 0, 'q', },
    { NULL, 0, 0, 0, },
};

char *usage_text(char const *progname) {
    char *result = NULL;
    int chars_written = asprintf(&result,
    "Creates two TCP server sockets, waits for an incoming connection\n"
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
#ifdef HAVE_IPV6
    "-6\t\t\tUse IPv6\n"
#endif
    "\n"
    "-p and -q are required.\n",
    progname);
    assert(chars_written > 0);
    free_object_at_exit(free, result);
    return result;
}

bool get_port(char const *arg, uint16_t *out_port) {
    char *check_ptr;
    long conv_result = strtol(arg, &check_ptr, 0);
    if (arg == check_ptr || conv_result <= 0 || conv_result > 65535) {
        return false;
    }
    *out_port = (uint16_t) conv_result;
    return true;
}

tca_options *evaluate_options(int argc, char *argv[]) {
    tca_options *result;
    result = allocate_tca_options();
    free_object_at_exit(free_tca_options, result);

    optind = 1;
    int selected_option;
#ifdef HAVE_IPV6
    char const *short_option_string = "6ha:b:p:q:";
#else
    char const *short_option_string = "ha:b:p:q:";
#endif
    while ((selected_option = getopt_long(
                    argc, argv, short_option_string, long_opts, NULL)) != -1) {
        if (selected_option == '6') {
            result->use_ipv6 = true;
        } else if (selected_option == 'a') {
            result->connection_endpoints[0]->address_str = strdup(optarg);
        } else if (selected_option == 'b') {
            result->connection_endpoints[1]->address_str = strdup(optarg);
        } else if (selected_option == 'p') {
            if (!get_port(optarg, &result->connection_endpoints[0]->port)) {
                errx(EINVAL, "First port invalid: %s", optarg);
            }
        } else if (selected_option == 'q') {
            if (!get_port(optarg, &result->connection_endpoints[1]->port)) {
                errx(EINVAL, "Second port invalid: %s", optarg);
            }
        } else if (selected_option == 'h') {
            printf("%s", usage_text(argv[0]));
            exit(0);
        } else if (selected_option == '?') {
            errx(EINVAL, "Error: Unrecognised option or missing arguments");
        } else {
            err(errno, "Error while parsing options");
        }
    }

    /* check requirements and set defaults where applicable */
    int i;
    for (i = 0; i < NUMBER_OF_ENDPOINTS; i++) {
        tca_address *current_ep = result->connection_endpoints[i];

        if (current_ep->port == 0) {
            errx(ENOENT, "Both ports are required for forwarding.\n\n"
                    "Usage: %s", usage_text(argv[0]));
        }
        if (current_ep->address_str == NULL) {
            current_ep->address_str =
                result->use_ipv6 ? strdup("::0") : strdup("0.0.0.0");
        }
    }

    return result;
}


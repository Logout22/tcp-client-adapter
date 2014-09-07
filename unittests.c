#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <err.h>

#include "tcpbridge_options.h"
#include "tcpbridge_signal.h"
#include "freeatexit.h"
#include "nwfns.h"

char *host1 = "nonexistinghost.nodomain";
char *host2 = "otherhost.nodomain";

void test_no_options() {
    if (g_test_subprocess()) {
        char *test_argv[] = { "" };
        evaluate_options(1, test_argv);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*Ports are required for forwarding.\n\nUsage*");
}

void test_wrong_option() {
    if (g_test_subprocess()) {
        char *test_argv[] = { "", "--nonex-option" };
        evaluate_options(2, test_argv);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*Unrecognised option*");
}

void test_help() {
    if (g_test_subprocess()) {
        char *test_argv[] = { "", "-h" };
        evaluate_options(2, test_argv);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_passed();
    g_test_trap_assert_stdout(usage_text(""));
}

void test_default_values() {
    char *test_argv[] = {
        "",
        "-p", "6",
        "-q", "8",
    };
    int test_argc = 5;
    tcpbridge_options *result = evaluate_options(test_argc, test_argv);
    g_assert(result->use_ipv6 == false);
    g_assert_cmpstr(result->first_address_str, ==, DEFAULT_ADDRESS);
    g_assert_cmpstr(result->second_address_str, ==, DEFAULT_ADDRESS);
    g_assert(result->first_port == 6);
    g_assert(result->second_port == 8);
}

void test_wrong_port1() {
    if (g_test_subprocess()) {
        char *test_argv[] = {
            "",
            "-p", "ABC",
            "-q", "8",
        };
        int test_argc = 5;
        evaluate_options(test_argc, test_argv);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*First port invalid*");
}

void test_wrong_port2() {
    if (g_test_subprocess()) {
        char *test_argv[] = {
            "",
            "-p", "6",
            "-q", "DEF",
        };
        int test_argc = 5;
        evaluate_options(test_argc, test_argv);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*Second port invalid*");
}

void test_all_values() {
    char *test_argv[] = {
        "",
        "-6",
        "-a", host1,
        "-b", host2,
        "-p", "6",
        "-q", "8",
    };
    int test_argc = 10;
    tcpbridge_options *result = evaluate_options(test_argc, test_argv);
    g_assert(result->use_ipv6 == true);
    g_assert_cmpstr(result->first_address_str, ==, host1);
    g_assert_cmpstr(result->second_address_str, ==, host2);
    g_assert(result->first_port == 6);
    g_assert(result->second_port == 8);
}

void test_signal(int signal, bool success) {
    if (g_test_subprocess()) {
        raise(signal);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    if (success) {
        g_test_trap_assert_passed();
    } else {
        g_test_trap_assert_failed();
    }
}

void test_sighup() { test_signal(SIGHUP, true); }
void test_sigterm() { test_signal(SIGTERM, true); }
void test_sigint() { test_signal(SIGINT, true); }
void test_sigusr1() { test_signal(SIGUSR1, false); }

void test_bind(char const *address, uint16_t const port, bool use_ipv6) {
    int socket = bind_socket(address, port, use_ipv6);
    g_assert(socket >= 0);
    close(socket);
}

void test_bind_local(bool use_ipv6) {
    uint16_t const test_port = 29407;
    warnx("If this test fails check that TCP port %d is unoccupied\n",
            test_port);

    if (g_test_subprocess()) {
        test_bind("localhost", test_port, use_ipv6);
        exit(0);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_passed();
}

void test_bind_local_v4() { test_bind_local(false); }
void test_bind_local_v6() { test_bind_local(true); }

void test_bind_local_root(bool use_ipv6) {
    warnx("If this test fails check that you are not root\n");

    if (g_test_subprocess()) {
        test_bind("localhost", 6, use_ipv6);
        exit(0);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
}

void test_bind_local_root_v4() { test_bind_local_root(false); }
void test_bind_local_root_v6() { test_bind_local_root(true); }

void test_bind_nonexisting(bool use_ipv6) {
    warnx("If this test fails check if host %s accidentally exists\n", host1);

    if (g_test_subprocess()) {
        test_bind(host1, 6, use_ipv6);
        exit(0);
    }

    g_test_trap_subprocess(NULL, 0, 0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*getaddrinfo failed*");
}

void test_bind_nonexisting_v4() { test_bind_nonexisting(false); }
void test_bind_nonexisting_v6() { test_bind_nonexisting(true); }

int main(int argc, char *argv[]) {
    atexit(free_atexit);
    register_signal_handler();

    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/options/no_options", test_no_options);
    g_test_add_func("/options/wrong_option", test_wrong_option);
    g_test_add_func("/options/help", test_help);
    g_test_add_func("/options/default_values", test_default_values);
    g_test_add_func("/options/wrong_port1", test_wrong_port1);
    g_test_add_func("/options/wrong_port2", test_wrong_port2);
    g_test_add_func("/options/all_values", test_all_values);
    g_test_add_func("/signals/sighup", test_sighup);
    g_test_add_func("/signals/sigterm", test_sigterm);
    g_test_add_func("/signals/sigint", test_sigint);
    // ...and an example for a signal that is not implemented
    g_test_add_func("/signals/sigusr1", test_sigusr1);
    g_test_add_func("/bind/test_bind_local_v4", test_bind_local_v4);
    g_test_add_func("/bind/test_bind_local_v6", test_bind_local_v6);
    g_test_add_func("/bind/test_bind_local_root_v4", test_bind_local_root_v4);
    g_test_add_func("/bind/test_bind_local_root_v6", test_bind_local_root_v6);
    g_test_add_func(
            "/bind/test_bind_nonexisting_v4", test_bind_nonexisting_v4);
    g_test_add_func(
            "/bind/test_bind_nonexisting_v6", test_bind_nonexisting_v6);
    return g_test_run();
}

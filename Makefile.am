## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tcpbridge
SOURCES = freeatexit.c nwfns.c tcpbridge_options.c tcpbridge_signal.c
tcpbridge_SOURCES = tcpbridge_main.c $(SOURCES)
tcpbridge_CFLAGS = $(LIBEVENT_CFLAGS)
tcpbridge_LDADD = $(LIBEVENT_LIBS)

if TESTS
TESTS = $(check_PROGRAMS)
check_PROGRAMS = unittests

unittests_SOURCES = unittests.c $(SOURCES)
unittests_CFLAGS = $(LIBEVENT_CFLAGS) $(GLIB_CFLAGS)
unittests_LDADD = $(LIBEVENT_LIBS) $(GLIB_LIBS)

if COVERAGE
unittests_CFLAGS += -fprofile-arcs -ftest-coverage

coverage: check
	./run_gcov.sh unittests $(unittests_SOURCES)
	##./run_gcov.sh integrationtests $(integrationtests_SOURCES)
endif

endif


## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tca
SOURCES = freeatexit.c nwfns.c options.c tca_signal.c
tca_SOURCES = main.c $(SOURCES)
tca_CFLAGS = $(LIBEVENT_CFLAGS)
tca_LDADD = $(LIBEVENT_LIBS)

if TESTS
TESTS = $(check_PROGRAMS)
check_PROGRAMS = unittests integrationtest

unittests_SOURCES = unittests.c $(SOURCES)
unittests_CFLAGS = $(LIBEVENT_CFLAGS) $(GLIB_CFLAGS)
unittests_LDADD = $(LIBEVENT_LIBS) $(GLIB_LIBS)

integrationtest_SOURCES = integrationtest.c $(SOURCES)
integrationtest_CFLAGS = $(LIBEVENT_CFLAGS)
integrationtest_LDADD = $(LIBEVENT_LIBS)

if COVERAGE
unittests_CFLAGS += -fprofile-arcs -ftest-coverage

coverage: check
	./run_gcov.sh unittests $(unittests_SOURCES)
endif

if VALGRIND
VALGRIND_CMD = valgrind --leak-check=full
valgrind: check
	$(VALGRIND_CMD) ./unittests && $(VALGRIND_CMD) ./integrationtest
endif

endif


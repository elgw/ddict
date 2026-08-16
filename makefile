CFLAGS=-Wall -Wextra

DEBUG?=0
ANA?=0
SAN?=0

ifeq ($(SAN),1)
CFLAGS+=-fsanitize=address
endif

ifeq ($(ANA),1)
CFLAGS+=-fanalyzer
endif

ifeq ($(DEBUG), 1)
CFLAGS+=-g3 -O1
else
CFLAGS+=-O3 -ffast-math -DNDEBUG -march=native -mtune=native
LDFLAGS+=-flto
endif

# Of course, please check with scan-build and valgrind as well.

CFLAGS_DDICT = $(CFLAGS) -I./include

ddict_test: test/ddict_test.c ddict.o test/common.h
	$(CC) $(CFLAGS_DDICT) test/ddict_test.c ddict.o $(LDFLAGS)  -o ddict_test

ddict.o: src/ddict.c include/ddict.h
	$(CC) -c $(CFLAGS_DDICT) -pedantic src/ddict.c -std=c99

CFLAGS_GLIB = $(CFLAGS) `pkg-config --cflags glib-2.0`
LDFLAGS_GLIB = $(LDFLAGS) `pkg-config --libs glib-2.0`

glib_test: test/glib_test.c test/common.h
	$(CC) $(CFLAGS_GLIB) test/glib_test.c $(LDFLAGS_GLIB) -o glib_test

# Just a .h file so nothing extra to do here
cdict_test: test/cdict_test.c test/common.h
	$(CC) $(CFLAGS) test/cdict_test.c $(LDFLAGS) -o cdict_test

# Assumes that libcdict it present, build it with
# $ cd test
# $ build_libcdict.sh
LDFLAGS_LIBCDICT=$(LDFLAGS) -Ltest/libcdict/builddir -lcdict
LDFLAGS_LIBCDICT+=-Wl,-rpath=test/libcdict/builddir/
libcdict_test: test/libcdict_test.c test/common.h
	cd test;
	$(CC) $(CFLAGS) test/libcdict_test.c $(LDFLAGS_LIBCDICT) -o libcdict_test

go_test:
	cd test/go; go build
	cp test/go/dict go_test

all: ddict_test cdict_test glib_test go_test libcdict_test

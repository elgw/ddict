CFLAGS=-Wall -Wextra -pedantic

CFLAGS+=-I./include

DEBUG?=0
ANA?=0
SAN?=0

ifeq ($(DEBUG), 1)
CFLAGS+=-g3 -O1
else
ifeq ($(SAN),1)
CFLAGS+=-g3 -fsanitize=address -O1
endif
# Optimization is default
CFLAGS+=-O3 -ffast-math -DNDEBUG -march=native -mtune=native
LDFLAGS+=-flto
endif

ifeq ($(ANA),1)
CFLAGS+=-fanalyzer
endif

# Of course, please check with scan-build and valgrind as well.

ddict_test: test/ddict_test.c ddict.o
	$(CC) $(CFLAGS) test/ddict_test.c ddict.o $(LDFLAGS)  -o ddict_test

ddict.o: src/ddict.c include/ddict.h
	$(CC) -c $(CFLAGS) src/ddict.c -std=c99

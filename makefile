CFLAGS=-Wall -Wextra -pedantic

CFLAGS+=-I./include

DEBUG?=0

ifeq ($(DEBUG),1)
CFLAGS+=-g3 -fsanitize=address
else
CFLAGS+=-O3 -ffast-math
LDFLAGS+=-flto
endif

ifeq ($(ANA),1)
CFLAGS+=-fanalyzer
endif

# Of course, please check with scan-build and valgrind as well.

ddict_test: ddict_test.c ddict.o
	$(CC) $(CFLAGS) ddict_test.c ddict.o $(LDFLAGS) -o ddict_test

ddict.o: src/ddict.c include/ddict.h
	$(CC) -c $(CFLAGS) src/ddict.c

CFLAGS=-Wall -Wextra -pedantic

DEBUG?=0

ifeq ($(DEBUG),1)
CFLAGS+=-g3 -fsanitize=address
else
CFLAGS+=-O3 -ffast-math -I./include
LDFLAGS+=-flto
endif

# consider also -fanalyzer and
# scan-build

ddict_test: ddict_test.c src/ddict.c
	$(CC) $(CFLAGS) ddict_test.c src/ddict.c $(LDFLAGS) -o ddict_test

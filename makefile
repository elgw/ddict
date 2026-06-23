CFLAGS=-Wall -Wextra -pedantic

DEBUG?=0

ifeq ($(DEBUG),1)
CFLAGS+=-g3 -fsanitize=address
else
CFLAGS+=-O3 -ffast-math
LDFLAGS+=-flto
endif

# consider also -fanalyzer and
# scan-build

test: test_ddict.c ddict.c
	$(CC) $(CFLAGS) test_ddict.c ddict.c $(LDFLAGS) -o test_ddict

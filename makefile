CFLAGS=-Wall

DEBUG?=0

ifeq ($(DEBUG),1)
CFLAGS+=-g3 -fsanitize=address
else
CFLAGS+=-O3
LDFLAGS+=-flto
endif

test: test.c ddict.c
	$(CC) $(CFLAGS) test.c ddict.c $(LDFLAGS) -o test

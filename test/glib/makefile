CFLAGS+=-Wall

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

CFLAGS+=`pkg-config --cflags glib-2.0`
LDFLAGS+=`pkg-config --libs glib-2.0`

glib_test: glib_test.c
	$(CC) $(CFLAGS) glib_test.c $(LDFLAGS) -o glib_test

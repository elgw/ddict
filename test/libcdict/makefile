CFLAGS=-Wall

# Obviously you need to change this for your machine
CFLAGS+=-I/home/erikw/include/ -DNDEBUG -march=native -mtune=native
LDFLAGS=-flto -L/home/erikw/lib -lcdict -Wl,-rpath=/home/erikw/lib

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


libcdict_test: libcdict_test.c
	$(CC) $(CFLAGS) libcdict_test.c $(LDFLAGS) -o libcdict_test

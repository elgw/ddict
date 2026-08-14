CFLAGS+=-Wall

DEBUG?=0

ifeq ($(DEBUG),1)
CFLAGS+=-g3 -fsanitize=address
else
CFLAGS+=-O3 -ffast-math -DNDEBUG -march=native -mtune=native
LDFLAGS+=-flto
endif


CFLAGS+=`pkg-config --cflags glib-2.0`
LDFLAGS+=`pkg-config --libs glib-2.0`

test_glib: test.c
	$(CC) $(CFLAGS) test.c $(LDFLAGS) -o test_glib

CFLAGS=-Wall -O3 -I/home/erikw/include/ -DNDEBUG -march=native -mtune=native
LDFLAGS=-flto -L/home/erikw/lib -lcdict -Wl,-rpath=/home/erikw/lib -flto

test: cdicts_test.c
	$(CC) $(CFLAGS) cdicts_test.c $(LDFLAGS) -o cdicts_test

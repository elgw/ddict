CFLAGS=-Wall -O3 -I/home/erikw/include/
LDFLAGS=-flto -L/home/erikw/lib -lcdict -Wl,-rpath=/home/erikw/lib -flto

test: cdicts_test.c
	$(CC) $(CFLAGS) cdicts_test.c $(LDFLAGS) -o cdicts_test

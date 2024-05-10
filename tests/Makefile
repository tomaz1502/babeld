.PHONY=clean

ROOT_DIR=..

CFLAGS = -Os -g -Wall -DTESTING

OBJS = babeld.o net.o kernel.o util.o interface.o source.o neighbour.o \
	   route.o xroute.o message.o resend.o configuration.o local.o \
	   hmac.o rfc6234/sha224-256.o BLAKE2/ref/blake2s-ref.o \
	   tests/hmac_test.o tests/util_test.o tests/test_utilities.o \
	   tests/route_test.o

main: main.c route_test.o hmac_test.o util_test.o test_utilities.o
	cd $(ROOT_DIR) && $(CC) $(CFLAGS) -o tests/main tests/main.c $(OBJS)

route_test.o: route_test.c route_test.h
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -c -o tests/route_test.o tests/route_test.c

hmac_test.o: hmac_test.c hmac_test.h
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -c -o tests/hmac_test.o tests/hmac_test.c

util_test.o: util_test.c util_test.h
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -c -o tests/util_test.o tests/util_test.c

test_utilities.o: test_utilities.h test_utilities.c
	$(CC) $(CFLAGS) -c test_utilities.c

clean:
	rm -f main *.o && cd $(ROOT_DIR) && $(MAKE) clean

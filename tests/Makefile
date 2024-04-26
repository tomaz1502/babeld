.PHONY=clean

ROOT_DIR=..

CFLAGS = -Os -g -Wall -DTESTING

OBJS = babeld.o net.o kernel.o util.o interface.o source.o neighbour.o \
       route.o xroute.o message.o resend.o configuration.o local.o \
       hmac.o rfc6234/sha224-256.o BLAKE2/ref/blake2s-ref.o

main: main.c hmac_test.o util_test.o
	cd $(ROOT_DIR) && $(CC) $(CFLAGS) -o tests/main tests/main.c tests/hmac_test.o tests/util_test.o $(OBJS)

hmac_test.o: hmac_test.c hmac_test.h
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -c -o tests/hmac_test.o tests/hmac_test.c $(OBJS)

util_test.o: util_test.c util_test.h
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -c -o tests/util_test.o tests/util_test.c $(OBJS)

clean:
	rm -f main *.o && cd $(ROOT_DIR) && $(MAKE) clean

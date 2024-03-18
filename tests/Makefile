.PHONY=clean

ROOT_DIR=..

CFLAGS = -Os -g -Wall

OBJS = babeld.o net.o kernel.o util.o interface.o source.o neighbour.o \
       route.o xroute.o message.o resend.o configuration.o local.o \
       hmac.o rfc6234/sha224-256.o BLAKE2/ref/blake2s-ref.o

all: test

test: test.c
	cd $(ROOT_DIR) && $(MAKE) CFLAGS+=-DTESTING LDLIBS='' o_files && $(CC) $(CFLAGS) -o tests/test tests/test.c $(OBJS)

clean:
	rm -f *.o test && cd $(ROOT_DIR) && $(MAKE) clean

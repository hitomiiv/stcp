CC = gcc
CCFLAGS = -O0 -g3 -Wall -Wextra -Werror

libstcp.a: error.o socket.o stcp.o
	ar rcs libstcp.a error.o socket.o stcp.o

error.o:
	${CC} -c src/error.c ${CCFLAGS}

socket.o:
	${CC} -c src/socket.c ${CCFLAGS}
	
stcp.o:
	${CC} -c src/stcp.c ${CCFLAGS}

test.o:
	${CC} -c src/test.c ${CCFLAGS}

static: libstcp.a

shared: libstcp.a
	${CC} -o libstcp.dylib -shared -L. -lstcp

test: libstcp.a test.o
	${CC} -o test test.o -L. -lstcp
	
all: test

clean:
	rm -f test *.o *.a *.dylib *.dll
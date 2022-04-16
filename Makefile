CC = gcc
CCFLAGS = -O0 -g3 -Wall -Wextra -Werror -fPIC
#CCFLAGS = -02 -Wall -Wextra -Werror -fPIC

libstcp.so:
	${CC} -c src/*.c ${CCFLAGS}
	${CC} -shared -o libstcp.so *.o -lssl -lcrypto 

run_tests: libstcp.so tests/*.c
	${CC} -c tests/*.c ${CCFLAGS}
	${CC} -o run_tests driver.o -L. -lstcp
	
all: libstcp.so run_tests

clean:
	rm -f run_tests *.o *.a *.so
CC = gcc -Wall

main: main.o stcp.o socket.o

main.o: main.c stcp.o

stcp.o: stcp.c socket.o

socket.o: socket.c

clean:
	rm -f *.o main

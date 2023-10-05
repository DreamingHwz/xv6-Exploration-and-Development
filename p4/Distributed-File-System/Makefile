all: clean libmfs.so server client

main:
	gcc main.c udp.h udp.c fs.c fs.h ufs.h mfs.h -g -o main

server:
	gcc server.c udp.h udp.c fs.c fs.h ufs.h mfs.h -g -o server

client: 
	gcc client.c udp.h udp.c -o client

libmfs.so: libmfs.o
	gcc -shared -Wl,-soname,libmfs.so -o libmfs.so libmfs.o udp.o -lc

libmfs.o: libmfs.c
	gcc -fPIC -c -Wall libmfs.c -o libmfs.o
	gcc -fPIC -c -Wall udp.c -o udp.o

mkfs:
	gcc mkfs.c ufs.h -o mkfs

clean:
	rm -f *.o *.so server client main
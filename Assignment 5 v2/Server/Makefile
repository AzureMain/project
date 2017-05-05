CFLAGS = -Wall -I. -I./md5 -O2 -g -lssl -lcrypto
DEBUG =
CC = gcc

server: server.c md5sum.c
	$(CC) $(CFLAGS) server.c md5sum.c -lcrypto -lssl -o $@

clean:
	rm -f *.o \
	server \
	client \
	catalog.csv

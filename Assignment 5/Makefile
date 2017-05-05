CFLAGS = -Wall -I. -I./md5 -O2 -g -lssl -lcrypto -std=c11
DEBUG =
CC = gcc

both: client server

server: Server/server.c md5/md5sum.c
	$(CC) $(CFLAGS) Server/server.c md5/md5sum.c -lcrypto -lssl -o $@

client: Client/client.c md5/md5sum.c
	$(CC) $(CFLAGS) Client/client.c md5/md5sum.c -lcrypto -lssl -o $@

clean:
	rm -f *.o \
	server \
	client

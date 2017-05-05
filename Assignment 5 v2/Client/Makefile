CFLAGS = -Wall -I. -O2 -g -lssl -lcrypto
DEBUG =
CC = gcc

client: client.c md5sum.c
	$(CC) $(CFLAGS) client.c md5sum.c -lcrypto -lssl -o $@

clean:
	rm -f *.o \
	client \
	images/* \
	catalog.csv \
	download.html

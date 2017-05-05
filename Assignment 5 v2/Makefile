CFLAGS = -Wall -I. -I./md5 -O2 -g -lssl -lcrypto
DEBUG =
CC = gcc

both: client server

server:
	make -C Server

client:
	make -C Client

clean:
	make -C Server clean
	make -C Client clean

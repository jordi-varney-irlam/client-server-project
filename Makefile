.PHONY: all
all: server client

server: server.c
	gcc -Wall -Werror -o server server.c

client: client.c
	gcc -Wall -Werror -o client client.c

clean: 
	rm -f server client

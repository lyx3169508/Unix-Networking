all:server client
	echo "pafone make "
server: socket_server.o
	gcc -o server socket_server.o
client: socket_client.o
	gcc -o client socket_client.o
%.o:%.c
	gcc -c $< -o $@
clean:
	rm -f server client *.o
client: client.c
	gcc -o client client.c

server: FTPserver.c
	gcc -o server FTPserver.c

clean:
	rm *.o server client

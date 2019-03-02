client: client.c
	gcc -o client client.c && ./client localhost 9999

server: FTPserver.c
	gcc -o server FTPserver.c && ./server 

clean:
	rm *.o server client

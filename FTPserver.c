#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>
#include <string.h>

char response[] = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: 9\r\n\r\n\
It works!\r\n";

int main(){
    int sock_fd, client_fd;
    int port = 8080;
    struct sockaddr_in server_addr, client_addr;

    // initializing the sockaddr to 0
    memset(&client_addr, 0, sizeof(client_addr));
    memset(&server_addr, 0, sizeof(server_addr));

    // setting the server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // opening the socket
    // your code goes here
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // set SO_REUSEADDR to avoid binding issues
    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    // binding the socket to server_addr
    // your code goes here
    int status = bind(sock_fd,(struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));

    // listening for connections
    // your code goes here
    status = listen(sock_fd, 5);

    // accepting a connection
    // your code goes here
    fd_set read_fd_set;
    while(1) {
        // clear the socket set
        FD_ZERO(&read_fd_set);

        // add STDIN to file descriptor set
        FD_SET(sock_fd, &read_fd_set);

        // wait for activity on file descriptor set
        select(sock_fd+1, &read_fd_set, NULL, NULL, NULL);

        // check for activity on file descriptor
        if (FD_ISSET(sock_fd, &read_fd_set)) {
          char buf[1024];
          memset(buf, 0, sizeof(buf));
          read(sock_fd, buf, sizeof(buf));
          printf("A key was pressed! %s\n", buf);
        }
    }
    socklen_t sock_len = sizeof(client_addr);
    int client_sockfd = accept(sock_fd, (struct sockaddr *) &client_addr, &sock_len);
    printf("Sockfd: %d\n", sock_fd);
    printf("Error code: %d\n", client_sockfd);
    // sending response back to client
    // your code goes here
    int count = send(client_sockfd, response, sizeof(response), 0);

    // terminating the connection
    // your code goes here
    status = close(sock_fd);

    // Fgets  a FTP command line from client via stream socket.
    //
    // If command  == "USER username"
    //     1. Check if username against authorized users.
    //     2. If username exists:
    //         send "Username OK, password required" message to client.
    //     3. If username does not exists
    //         send "Username does not exist" to client
    //
    // if the command is "PASS password":
    //     if the username is set
    //         check of the password matches.
    //         If password matches, send "Authentication complete"
    //         if password doesnt match, send "wrong password"
    //     if the username is not set
    //         send "set USER first"
    //
    // if user has authenticated:
    //     if the command is "PUT a file"
    //         1. Create a file
    //         2. Read the file from socket
    //         3. Write to the file
    //         4. Close the file.
    //
    //     if the command is "GET a file"
    //         1. Open the file
    //         2. If existed
    //             2.1 Send "existed" to client
    //             2.2 Read the file
    //             2.3 Write the file to client
    //             2.4 close the file
    //         3. if noneixsted:
    //             send "nonexisted" to client
    //
    //     if the command is "ls ..." or "pwd"
    //         1. fp = popen(command, "r")
    //         2. read the result from f
    //         3. if no result from fp, reply "wrong command usage!" else reply
    //         "Sucessfully executed!"
    //         4. send the result to client
    //
    //     if the command is "cd directory..."
    //         1. call chdirr(directory).
    //         2. reply to the client if the command is Sucessfully executed.
    //
    // if the command is "QUIT"
    //     1. close socket
    //     2. exit
    //
    // if user has not authenticated yet:
    //     Regardless of what you recieve
    //         send "Authenticate first" message


}

#include <stdio.h>

int main(){
    while(1){
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

}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
  //Server address
  char serverAddress[64];
  //Server port which we get from flags
  int serverPort;
  if (argc != 3){
    printf("Please enter the host name and server port number\n");
    return(1);
  } else {
    strcpy(serverAddress, argv[1]);
    serverPort = atoi(argv[2]);
  }
  printf("Host address: %s\n", serverAddress);
  printf("Server port: %d\n", serverPort);
  //One socket for communication, one socket for file transfer.
  int sockfd, sockfd2, i;
  struct sockaddr_in addr;
  char buf[1024]; //Buffer which we use to send and read.
  //If socket could not be created with the provided server address.
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(1);
  }
  //Set the socket details.
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort);
  inet_aton(serverAddress, &(addr.sin_addr));
  //Socket could not be connected
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(1);
  }

  while (1) {
    //Use inputLine to verify commands
    char inputLine[1024];
    char * command;
    char * serverResponse;
    //Make sure input is not empty and has command
    while(1){
      printf("ftp >> ");
      fgets(buf, 1024, stdin);
      strcpy(inputLine, buf);
      command = strtok(inputLine, "\n");
      if(command == NULL){
        continue;
      }
      command = strtok(command, " ");
      if(command == NULL){
        continue;
      }
      break;
    }
    //WE are sending username to server
    if(strcmp(command, "USER") == 0){
      //Write this, and clear buffer for response
      write(sockfd, buf, strlen(buf+1));
      memset(buf, 0, strlen(buf));
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      if (strcmp(buf, "Username OK, password required") == 0){
        printf("Username OK, password required\n");
      } else if (strcmp(buf, "Username does not exist") == 0){
        printf("Username does not exist\n");
      } else {
        printf("Incorrect response from server\n");
      }
    }

    else if(strcmp(command, "PASS") == 0){
      write(sockfd, buf, strlen(buf+1));
      memset(buf, 0, strlen(buf));
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      if (strcmp(buf, "Authentication complete") == 0){
        printf("Authentication complete\n");
      } else if (strcmp(buf, "Wrong password") == 0) {
        printf("Wrong password\n");
      } else if (strcmp(buf, "set USER first") == 0){
        printf("set USER first\n");
      } else {
        printf("%s",buf);
        printf("Incorrect response from server\n");
      }
    }

    //We are uploading file to server.
    else if(strcmp(command, "PUT") == 0){
      //Get the filename
      char* file = strtok(NULL, " ");
      if(file == NULL){
        printf("Please enter file name after PUT\n");
        continue;
      }
      //See if file can be opened.
      int fp = open(file,O_RDONLY);
      if(fp == -1){
        printf("File %s could not be opened \n", file);
        continue;
      }
      char buf[1000];
      memset(&buf, 0, sizeof(buf)); // zero out the buffer
      sprintf(buf, "PUT %s", file); //Fill the buffer with our filename
      write(sockfd, buf, strlen(buf) + 1); //Send this to server
      memset(buf, 0, strlen(buf)); //Reset buf in preparation to recieve
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }

      serverResponse = strtok(buf, "\n");
      if(serverResponse == NULL){
        printf("Malformed response from server \n");
        continue;
      }
      serverResponse = strtok(serverResponse, " ");
      if(strcmp(serverResponse, "AUTHENTICATION") == 0){
        printf("Server requires you to be authenticated \n");
        continue;
      }
      if(strcmp(serverResponse, "PUTREADY") != 0){
        printf("Server did not respond with PUTREADY \n");
        continue;
      }
      serverResponse = strtok(NULL, " ");
      if(serverResponse == NULL){
        printf("Server did not provide port \n");
        continue;
      }
      /*  Create new socket for file transfer*/
      if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Can't open socket\n");
        exit(1);
      }
      //Change the port of addr.
      addr.sin_port = htons(atoi(serverResponse));
      //Connect and be ready to transfer
      if (connect(sockfd2, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Can't connect to port given by server\n");
        continue;
      }
      //We create a line, and while we read files from the FP, we will keep writing.
      char* line = (char*)malloc(1024);
      int bytes_read = 0;
      do{
        bytes_read = read(fp, line, 1024);
        write(sockfd2, line, bytes_read);
      } while(bytes_read != 0);

      //Close the respective sockets
      free(line);
      close(fp);
      close(sockfd2);
    }

    //Download a file from the server
    else if(strcmp(command, "GET") == 0){
      //Get the filename
      char* file = strtok(NULL, " ");
      if(file == NULL){
        printf("Please enter file name after GET\n");
        continue;
      }
      //WRite the command and clear buffer for reading
      write(sockfd, buf, strlen(buf+1));
      memset(buf, 0, strlen(buf));
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0); //Server closed the connection!!! How dare they.
      }

      serverResponse = strtok(buf, "\n");
      if(serverResponse == NULL){
        printf("Malformed response from server \n");
        continue;
      }
      serverResponse = strtok(serverResponse, " ");
      if(strcmp(serverResponse, "AUTHENTICATION") == 0){
        printf("Server requires you to be authenticated \n");
        continue;
      }
      if(strcmp(serverResponse, "GETREADY") != 0){
        printf("Server did not respond with GETREADY \n");
        continue;
      }
      serverResponse = strtok(NULL, " ");
      if(serverResponse == NULL){
        printf("Server did not provide port \n");
        continue;
      }
      //We create a new socket for recieving files.
      if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Can't open socket\n");
        exit(1);
      }
      //Change the port of addr.
      addr.sin_port = htons(atoi(serverResponse));
      //Connect and be ready to transfer
      if (connect(sockfd2, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Can't connect to port given by server\n");
        continue;
      }
      //See if file can be created
      int fp = open(file,O_CREAT, 0666);
      if(fp == -1){
        printf("File %s could not be created \n", file);
        continue;
      }
      close(fp);
      //See if file can be opened
      fp = open(file,O_WRONLY, 0666);
      if(fp == -1){
        printf("File %s could not be opened for writing \n", file);
        continue;
      }
      //While the server is writing to us, we will read and write to file.
      //Wait until server sends a close, which will be 0 bytes read.
      char* line = (char*)malloc(1024);
      int bytes_read = 0;
      do{
        bytes_read = read(sockfd2, line, 1024);
        write(fp, line, bytes_read);
      } while(bytes_read != 0);
      close(fp); //Close the socket and the file pointer
      close(sockfd2);
    }

    //Simply call getcwd to print the working directory.
    else if(strcmp(command, "!PWD") == 0){
      char wd[1024];
      getcwd(wd, 1024);
      printf("%s\n", wd);
    }

    else if(strcmp(command, "!LS") == 0){
      //Get the arguments
      char* args = strtok(NULL, " ");
      //Fork and call LS with our arguments
      int LSstatus;
      if(fork() == 0){
        char* argv[2];
        argv[0] = "/bin/ls";
        argv[1] = args;
        execv(argv[0], argv);
      } else {
        //Wait for LS to finish printing
        wait(&LSstatus);
      }
    }
    else if(strcmp(command, "!CD") == 0){
      //Get the arguments
      char* args = strtok(NULL, " ");
      if(args == NULL){
        continue;
      }
      //Change the directory
      if(chdir(args) == -1){
        printf("qksh: %s: error changing directory\n", args);
      }
    }

    else if(strcmp(command, "PWD") == 0){
      //printf("-2%s\n", buf);
      write(sockfd, buf, strlen(buf+1));
      //printf("-1%s\n", buf);
      char *buff = (char *)malloc(1024);
      memset(buff, 0, strlen(buff)+1);
      //printf("0%s\n", buff);
      if (read(sockfd, buff, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      serverResponse = strtok(buff, "\n");
      if(serverResponse == NULL){
        printf("Malformed response from server \n");
        continue;
      }
      serverResponse = strtok(serverResponse, " ");
      if(strcmp(serverResponse, "AUTHENTICATION") == 0){
        printf("Server requires you to be authenticated \n");
        continue;
      }
      if(strcmp(serverResponse, "SUCCESS") != 0){
        printf("Error executing PWD on server \n");
        continue;
      }
      serverResponse = strtok(NULL, " ");
      if(serverResponse == NULL){
        printf("Command executed \n");
        continue;
      }
      printf("%s\n", serverResponse);
      free(buff);
    }

    else if(strcmp(command, "CD") == 0){
      //Write the command to server and reset buffer in prepration for response
      write(sockfd, buf, strlen(buf+1));
      char *buff = (char *)malloc(1024);
      memset(buff, 0, strlen(buff)+1);
      if (read(sockfd, buff, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      serverResponse = strtok(buff, "\n");
      if(serverResponse == NULL){
        printf("Malformed response from server \n");
        continue;
      }
      if(strcmp(serverResponse, "AUTHENTICATION") == 0){
        printf("Server requires you to be authenticated \n");
        continue;
      }
      serverResponse = strtok(serverResponse, " ");
      if(strcmp(serverResponse, "SUCCESS") != 0){
        printf("Error executing CD on server \n");
        continue;
      }
      serverResponse = strtok(NULL, " ");
      if(serverResponse == NULL){
        printf("Command executed \n");
        continue;
      }
      printf("%s\n", serverResponse);
    }

    //Because LS can be long, we will tokenize the LS and recieve it one by one.
    //Until a terminating sign.
    else if(strcmp(command, "LS") == 0){
      //Send LS command and reset buffer in preparation for reading.
      write(sockfd, buf, strlen(buf+1));
      char *buff = (char *)malloc(1024);
      memset(buff, 0, strlen(buff)+1);
      if (read(sockfd, buff, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      serverResponse = strtok(buff, "\n");
      if(serverResponse == NULL){
        printf("Malformed response from server \n");
        continue;
      }
      if(strcmp(serverResponse, "AUTHENTICATION") == 0){
        printf("Server requires you to be authenticated \n");
        continue;
      }
      if(strcmp(serverResponse, "SUCCESS") != 0){
        printf("Error executing LS on server \n");
        continue;
      }
      //Begin recieving LS from server.
      char* line = (char*)malloc(1024);
      int bytes_read = 0;
      do{
        memset(line, 0 , 1024); //Reset line in preparation for new read.
        bytes_read = read(sockfd, line, 1024);
        printf("%s",line); //We wait util we recieve terminating
      } while(bytes_read != 0 && strcmp(line,"\r\n") != 0);

    }

    //Shuts down, and close our socket with the server.
    else if(strcmp(command, "QUIT") == 0){
      printf("Shutting down...\n");

      if(close(sockfd) == 0){
        printf("Closed socket\n");
      } else {
        printf("Closing socket error\n");
      }
      return(0);
    }

    //If no commands detected at this point, user probably entered a wrong command
    else {
      printf("Invalid FTP command entered\n");
    }
  }
}

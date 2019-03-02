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
#include <unistd.h>

int main(int argc, char * argv[])
{
  char serverAddress[64];
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

  int sockfd, i;
  struct sockaddr_in addr;
  char buf[1024];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(serverPort);
  inet_aton(serverAddress, &(addr.sin_addr));

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(1);
  }

  while (1) {
    //Use inputLine to verify commands
    char inputLine[1024];
    char * command;

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

    if(strcmp(command, "USER") == 0){
      write(sockfd, buf, strlen(buf+1));
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      if (strcmp(buf, "USER placeholderSucess\n") == 0){
        printf("Username OK, password required\n");
      } else if (strcmp(buf, "USER placeholderSucess2\n") == 0){
        printf("Username does not exist\n");
      } else {
        printf("Incorrect response from server\n");
      }
    }

    else if(strcmp(command, "PASS") == 0){
      write(sockfd, buf, strlen(buf+1));
      if (read(sockfd, buf, 1024) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      if (strcmp(buf, "PASS placeholderSucess\n") == 0){
        printf("Authentication complete\n");
      } else if (strcmp(buf, "PASS placeholderSucess2\n") == 0) {
        printf("Wrong password\n");
      } else if (strcmp(buf, "PASS placeholderSucess3\n") == 0){
        printf("set USER first\n");
      } else {
        printf("Incorrect response from server\n");
      }
    }

    else if(strcmp(command, "QUIT") == 0){
      printf("Shutting down...\n");

      if(close(sockfd) == 0){
        printf("Closed socket\n");
      } else {
        printf("Closing socket error\n");
      }
      return(0);
    }

    else {
      printf("Incorrect input entered\n");
    }
  }
}

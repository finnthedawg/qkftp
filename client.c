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
  int sockfd, i;
  struct sockaddr_in addr;
  char buf[1024];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9999);
  inet_aton("127.0.0.1", &(addr.sin_addr));

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(1);
  }
  
  printf("Enter a message\n");
  
  while (fgets(buf, 1024, stdin)) {
    write(sockfd, buf, strlen(buf+1));
    if (read(sockfd, buf, 1024) == 0) {
      printf("Server closed connection\n");
      exit(0);
    }
    printf("Server response: %s\n", buf);
  }
}


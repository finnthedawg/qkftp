#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>


#define MAXCLIENTS 30
#define MAXUSERS 100

struct user{
    char name[30];
    char password[30];
};

struct client
{
  int fd;
  int user_id;
  int is_authenticated;
  char directory[100];
};

int main(int argc, char * argv[])
{
   FILE *fp;
   char * line = NULL;
    size_t len = 0;
    ssize_t read_bytes;


   fp = fopen("users.txt", "r"); // read mode
   if (fp == NULL)
   {
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
   }
   read_bytes = getline(&line, &len, fp);
   int nuser = atoi(line);
   struct user* users = (struct user*)malloc(sizeof(struct user)*nuser);
   int ind = 0;
    while ((read_bytes = getline(&line, &len, fp)) != -1) {
          char * pch;
          pch = strtok(line," ");
          int cnt = 0;
          while (pch != NULL){
            if (cnt == 0) strcpy(users[ind].name, pch);
            else strcpy(users[ind].password, pch);
            pch = strtok (NULL, " ");
            cnt++;
          }
          ind++;
    }
   fclose(fp);
  int master_socket, accepted_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  char buf[1024];
  fd_set read_fd_set;
  int maxfd, i;
  int port = 9999;
  int start_port = 9930;

  // initialize array of clients
  struct client* clients = (struct client*)malloc(sizeof(struct client)*100);
  for (i = 0; i < MAXCLIENTS; i++) {
    clients[i].fd = -1;
    clients[i].user_id = -1;
    clients[i].is_authenticated=0;
    getcwd(clients[i].directory, 100);
  }

  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket < 0) {
    printf("Can't open socket\n");
    exit(1);
  }

  int reuse = 1;
  setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(master_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
    close(master_socket);
    printf("Can't bind socket\n");
    exit(1);
  }

  if (listen(master_socket, 5) < 0) {
    close(master_socket);
    printf("Can't listen on socket\n");
    exit(1);
  }

  while (1) {

    // clear the socket set
    FD_ZERO(&read_fd_set);
 
    // add master socket to file descriptor set
    FD_SET(master_socket, &read_fd_set);
    maxfd = master_socket;

    // add child sockets to set
    for (i = 0; i < MAXCLIENTS; i++) {

        // get socket descriptor
        client_socket = clients[i].fd;

        // if socket descriptor is valid, then add it to read list
        if(client_socket > 0) {
          FD_SET(client_socket, &read_fd_set);
        }

        // add highest file descriptor number to maxfd
        if(client_socket > maxfd)
            maxfd = client_socket;
    }

    // wait for activity on one of the sockets
    select(maxfd+1, &read_fd_set, NULL, NULL, NULL);

    // check for activity on the master_socket (if so, then it must be an incoming request)
    if (FD_ISSET(master_socket, &read_fd_set)) {

      // a client tries to connect
      socklen_t len = sizeof(client_addr);
      if ((accepted_socket = accept(master_socket, (struct sockaddr *)(&client_addr), &len)) < 0) {
          printf("Can't accept connection\n");
          exit(1);
      }

      printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

      // add new socket to array of clients
      for (i = 0; i < MAXCLIENTS; i++) {

          // if position is empty, add it
          if (clients[i].fd < 0) {
              clients[i].fd = accepted_socket;
              printf("[%d]Adding client to list of sockets with socket\n", i);
              break;
          }
      }

      // new client can not be added, close socket
      if (i == MAXCLIENTS) {
        printf("Too many connections\n");
        close(accepted_socket);
      }
    }

    // loop through all clients and check for activity on all client sockets
    for (i = 0; i < MAXCLIENTS; i++) {

      // skip it array position is not used
      if (clients[i].fd < 0) {
        continue;
      }

      // check for activity
      if (FD_ISSET(clients[i].fd, &read_fd_set)) {

        memset(buf, 0, sizeof(buf)); //reset the buffer
        int num = recv(clients[i].fd, buf, 1024, 0); // read from socket

        // client closed the connection
        if (num == 0) {
          printf("[%d]Closing connection for a client\n", i);
          close(clients[i].fd);
          FD_CLR(clients[i].fd, &read_fd_set); // clear the file descriptor set for client[i]
          clients[i].fd = -1;
        } else {
          printf("[%d]Received: %s\n", i, buf);
          char * pch;
          pch = strtok(buf," ");
          int cnt = 0;
          char cmd[10];
          char argument[100];
          while (pch != NULL){
            if (cnt == 0) strcpy(cmd, pch);
            else strcpy(argument, pch);
            pch = strtok (NULL, " ");
            cnt++;
          }
          if (strcmp (cmd,"USER") == 0){
            int ii=0;
            int ind = -1;
            for (ii=0;ii<nuser;ii++){
                if (strcmp(users[ii].name,argument)==0){
                    ind = ii;
                    break;
                }
            }
            if (ind == -1){
                char message[] = "Username does not exist";
                write(clients[i].fd,message,strlen(message)+1);
            }
            else {
                char message[] = "Username OK, password required";
                clients[i].user_id=ind;
                write(clients[i].fd,message,strlen(message)+1);
            }
          }
          else if (strcmp (cmd,"PASS") == 0){
          
            if(clients[i].user_id!=-1){
          
              if (strncmp(users[clients[i].user_id].password,argument,strlen(argument))==0){
                char message1[] = "Authentication complete";
                clients[i].is_authenticated=1;
                write(clients[i].fd,message1,strlen(message1)+1);
              }
              else {
                char message1[] = "Wrong password";
                clients[i].is_authenticated=1;
                write(clients[i].fd,message1,strlen(message1)+1);
              }
            }
            else {
              char message1[] = "set USER first";
              write(clients[i].fd,message1,strlen(message1)+1);
            }
          }
          if (clients[i].is_authenticated==1){
              if (strcmp (cmd,"PUT") == 0){
                int port = start_port + i;
                printf("HERE\n");
                if (fork() == 0){
                  char dir[1000];
                  memset(&dir, 0, sizeof(dir)); // zero out the buffer    
                  sprintf(dir, "%s/%s", clients[i].directory, argument);
                  printf("DIR:%s\n", dir);
                  int fp = open(dir,O_CREAT|O_WRONLY, 0666);
                  int data_socket = socket(AF_INET, SOCK_STREAM, 0);
                  if (data_socket < 0) {
                    printf("Can't open socket\n");
                    exit(1);
                  }
                  printf("FILE CREATED\n");
                  int reuse = 1;
                  setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
                  struct sockaddr_in client_addr, data_server_addr;
                  data_server_addr.sin_family = AF_INET;
                  data_server_addr.sin_addr.s_addr = INADDR_ANY;
                  data_server_addr.sin_port = htons(port);
                  if (bind(data_socket, (struct sockaddr *) &data_server_addr, sizeof(data_server_addr))) {
                    close(data_socket);
                    printf("Can't bind socket\n");
                    exit(1);
                  }

                  if (listen(data_socket, 5) < 0) {
                    close(data_socket);
                    printf("Can't listen on socket\n");
                    exit(1);
                  }
                  char mess[30];
                  memset(&mess, 0, sizeof(mess)); // zero out the buffer    
                  sprintf(mess, "PUTREADY %d", port);
                  write(clients[i].fd, mess, strlen(mess)+1);
                  socklen_t len = sizeof(client_addr);
                  int client_data_socket;
                  if ((client_data_socket = accept(data_socket, (struct sockaddr *)(&client_addr), &len)) < 0) {
                      printf("Can't accept connection\n");
                      exit(1);
                  }
                  int bytes_read = 0;
                  char* line = (char*)malloc(1024);
                  printf("%d\n", client_data_socket);
                  printf("START READING\n");
                  do{
                    bytes_read = read(client_data_socket, line, 1024);
                    printf("READ %d bytes: %s\n", bytes_read, line);
                    write(fp, line, bytes_read);
                  } while(bytes_read != 0);
                  printf("WHILE LOOP DONE\n");
                  free(line);
                  close(client_data_socket);
                  close(fp);
                  return;
                }
                //printf("I AM PARENT\n");
              }
              else if (strcmp (cmd,"GET") == 0){

              }
              else if (strcmp (cmd,"ls") == 0){

              }
              else if (strcmp (cmd,"pwd") == 0){

              }
              else if (strcmp (cmd,"cd directory") == 0){

              }

          }
          else if (strcmp (cmd,"QUIT") == 0){
              printf("[%d]Closing connection for a client\n", i);
              close(clients[i].fd);
              FD_CLR(clients[i].fd, &read_fd_set); // clear the file descriptor set for client[i]
              clients[i].fd = -1;
          }
        }
      }
    }
  }
}

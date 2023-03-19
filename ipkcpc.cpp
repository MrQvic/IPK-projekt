#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>

#define BUFSIZE 257
#define PAYLOADSIZE 255
int client_socket;

void arg_check(int argc, char * argv[]);
void do_udp(const char *server_name, int port);
void do_tcp(const char *server_name, int port);
void leave(int s);

int main(int argc, char * argv[]) {

  /*  Odchyceni vyjimky preruseni (ctrl-c) */
  struct sigaction interrupthandler;
  memset(&interrupthandler, 0, sizeof(struct sigaction));
  interrupthandler.sa_flags = 0;
  interrupthandler.sa_handler = leave;
  sigaction(SIGINT, &interrupthandler, NULL);

  /*  test vstupnich parametru: */
  arg_check(argc, argv);
}

/* Funkce pro kontrolu zadaných argumentů */
void arg_check(int argc, char * argv[]){
  if (argc != 7) {
      fprintf(stderr,"usage: ipkcpc -h <host> -p <port> -m <mode>\n");
      exit(EXIT_FAILURE);
  }
  const char *server_name;
  const char *mode;
  int port;

  for(int i = 1; i < argc; i+=2){
    if(strcmp(argv[i], "-h") == 0)
      server_name = argv[i+1];

    else if(strcmp(argv[i], "-p") == 0)
      port = atoi(argv[i+1]);

    else if(strcmp(argv[i], "-m") == 0)
      mode = argv[i+1];
    
    else{
      fprintf(stderr, "Wrong input arguments! %s\n", argv[i]);
      fprintf(stderr, "usage: ipkcpc -h <host> -p <port> -m <mode>\n");
      exit(EXIT_FAILURE);
    }
  }

  if(strcmp(mode, "udp") == 0){
    do_udp(server_name, port);
  }
  else if(strcmp(mode, "tcp") == 0){
    do_tcp(server_name, port);
  }
  else{
    fprintf(stderr, "Wrong input argument <mode>!\n");
    fprintf(stderr, "usage: ipkcpc -h <host> -p <port> -m <mode>\n");
    exit(EXIT_FAILURE);
  }
}

void do_udp(const char *server_name, int port){

  /* Definice proměnných */
	int bytes_sent, bytes_recv;
  socklen_t serverlen;
  struct hostent *server;
  struct sockaddr_in server_address;
  
  /*  Ziskani adresy serveru pomoci DNS */
  server = gethostbyname(server_name);
  if (server == NULL) {
      fprintf(stderr,"ERROR: no such host as %s\n", server_name);
      exit(EXIT_FAILURE);
  }
  
  /*  Nalezeni IP adresy serveru a inicializace struktury server_address */
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(port);
  
  /*  Pomocne promenne pro komunikaci se serverem */
  char input[PAYLOADSIZE];
  char recv[BUFSIZE+1];
  char strip[PAYLOADSIZE];

  /* Vytvoreni soketu */
  client_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_socket < 1)
  {
    perror("ERROR: Chyba pri vytvareni socketu");
    exit(EXIT_FAILURE);
  }

  while(true){ 

    /* Vynulovani pomocnych promennych */
    bzero(input, PAYLOADSIZE);
    bzero(recv, BUFSIZE+1);
    bzero(strip, PAYLOADSIZE);
  
    /* Nacteni inputu ze STDIN */
    fgets(input, BUFSIZE, stdin);

    /* Slozeni zpravy pro server podle protokolu "Request Message Format"*/
    char send[257] = {0, (char)strlen(input)};
    memcpy(send + 2, input, 255);



    /* Odeslani zpravy na server */
    serverlen = sizeof(server_address);
    bytes_sent = sendto(client_socket, send, strlen(&send[2]) + 2, 0, (struct sockaddr *) &server_address, serverlen);
    if (bytes_sent < 0) 
      perror("ERROR: Chyba pri odesilani na server");
    
    /* Prijeti odpovedi*/
    bytes_recv = recvfrom(client_socket, recv, 257, 0, (struct sockaddr *) &server_address, &serverlen);
    if (bytes_recv < 0) 
      perror("ERROR: Chyba pri prijimani odpovedi ze serveru");

    /* Vypsani odpovedi*/
    memcpy(strip, &recv[3], recv[2]); //lze pouzit strlen(&send[3]) pro delku zpravy - predpokladam spravnou odpoved serveru
    printf("%s", strip);
    printf("\n");
  }
}

void leave(int s){
  close(client_socket);
  exit(0);
}
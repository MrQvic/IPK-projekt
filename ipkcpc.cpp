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
#define TCPSIZE 255
int client_socket;
int flag;

void arg_check(int argc, char * argv[]);
void prepare_connect(const char *server_name, int port, const char *mode);
void do_udp(struct sockaddr_in* server_address);
void do_tcp(struct sockaddr_in* server_address);
void leave(int s);

int main(int argc, char * argv[]) {

  /*  Odchyceni vyjimky preruseni (ctrl-c) */
  /*  inspirace z https://stackoverflow.com/questions/1641182/how-can-i-catch-a-ctrl-c-event*/
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
      fprintf(stderr,"Pouziti: ./ipkcpc -h <host> -p <port> -m <mode>\n");
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
      fprintf(stderr, "Spatne zadany vstupni argument %s\n", argv[i]);
      fprintf(stderr, "Pouziti: ./ipkcpc -h <host> -p <port> -m <mode>\n");
      exit(EXIT_FAILURE);
    }
  }

  if(strcmp(mode, "udp") != 0 && strcmp(mode, "tcp") != 0){
    fprintf(stderr, "Spatne zadany argument <mode>!\n");
    fprintf(stderr, "Pouziti: ./ipkcpc -h <host> -p <port> -m <mode>\n");
    exit(EXIT_FAILURE);
  }
  prepare_connect(server_name, port, mode);
}

void prepare_connect(const char *server_name, int port, const char *mode){

  /* Definice proměnných */
  struct hostent *server;
  struct sockaddr_in server_address;
  
  /*  Ziskani adresy serveru pomoci DNS */
  server = gethostbyname(server_name);
  if (server == NULL) {
      fprintf(stderr,"ERR: Neexistujici host %s\n", server_name);
      exit(EXIT_FAILURE);
  }

  /*  Nalezeni IP adresy serveru a inicializace struktury server_address */
  /*  využito z https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/client.c */
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(port);
  
  /* Vytvoreni soketu UDP*/
  if(strcmp(mode, "udp") == 0){
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket < 1){
      perror("ERR: Chyba pri vytvareni socketu");
      exit(EXIT_FAILURE);
    }
    flag = 1;
    do_udp(&server_address);  //zavolani funkce pro UDP protokol
  }

  /* Vytvoreni soketu TCP*/
  else{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 1){
      perror("ERR: Chyba pri vytvareni socketu");
      exit(EXIT_FAILURE);
    }
    flag = 2;
    do_tcp(&server_address);  //zavolani funkce pro TCP protokol
  }
}

/* Komunikace se serverem pomoci protokolu UDP*/
void do_udp(struct sockaddr_in* server_address){

  /*  Pomocne promenne pro komunikaci se serverem */
  socklen_t serverlen;
  char input[PAYLOADSIZE];
  char recv[BUFSIZE];
  char strip[PAYLOADSIZE];
	int bytes_sent, bytes_recv;

  while(true){ 
    /* Vynulovani pomocnych promennych */
    bzero(input, PAYLOADSIZE);
    bzero(recv, BUFSIZE);
    bzero(strip, PAYLOADSIZE);
  
    /* Nacteni inputu ze STDIN */
    fgets(input, BUFSIZE, stdin);

    /* Slozeni zpravy pro server podle protokolu "Request Message Format" */
    char send[257] = {0, (char)strlen(input)};
    memcpy(send + 2, input, 255);

    /* Odeslani zpravy na server */
    bytes_sent = sendto(client_socket, send, strlen(&send[2]) + 2, 0, (struct sockaddr *)server_address, sizeof(*server_address));
    if (bytes_sent < 0) 
      perror("ERR: Chyba pri odesilani na server");
    
    /* Prijeti odpovedi*/
    bytes_recv = recvfrom(client_socket, recv, 257, 0, (struct sockaddr *)server_address, &serverlen);
    if (bytes_recv < 0) 
      perror("ERR: Chyba pri prijimani odpovedi ze serveru");

    /* Vypsani odpovedi*/
    memcpy(strip, &recv[3], recv[2]); //lze pouzit strlen(&send[3]) pro delku zpravy - predpokladam spravnou odpoved serveru
    if(recv[1] == 0)
      printf("OK:%s\n", strip);
    else
      printf("ERR:%s\n", strip);
  }
}

void do_tcp(struct sockaddr_in* server_address){
  int connection = connect(client_socket, (const struct sockaddr *)server_address, sizeof(*server_address));
  if (connection != 0){
    perror("ERR: Chyba pri pripojovani k serveru (TCP)");
    exit(EXIT_FAILURE);        
  }

  char input[TCPSIZE];
  //char recv[BUFSIZE];
	int bytes_sent, bytes_recv;

  while(true){
    /* nacteni zpravy od uzivatele */
    bzero(input, TCPSIZE);
    fgets(input, TCPSIZE, stdin);

    /* odeslani zpravy na server */
    bytes_sent = send(client_socket, input, strlen(input), 0);
    if (bytes_sent < 0) 
      perror("ERR: Chyba pri odesilani dat na server");
    
    bzero(input, TCPSIZE);

    /* prijeti odpovedi a jeji vypsani */
    bytes_recv = recv(client_socket, input, TCPSIZE, 0);
    if (bytes_recv < 0) 
      perror("ERR: Chyba pri prijimani dat ze serveru");
      
    printf("%s", input);

    if(strcmp(input, "BYE\n") == 0){
      break;
    }
  }
  shutdown(client_socket, SHUT_RDWR);
  close(client_socket);
  exit(0);
}

void leave(int s){
  (void)s;
  if(flag == 2){
    char input[PAYLOADSIZE] = "BYE\n";
    int bytes;

    bytes = send(client_socket, input, strlen(input), 0);
    if (bytes < 0) 
      perror("ERR: Chyba pri odesilani dat na server");

    printf("%s", input);

    bzero(input, PAYLOADSIZE);
    bytes = 0;

    bytes = recv(client_socket, input, PAYLOADSIZE, 0);
    if (bytes < 0) 
      perror("ERR: Chyba pri prijimani dat ze serveru");

    printf("%s", input);
    shutdown(client_socket, SHUT_RDWR);
  }
  close(client_socket);
  exit(0);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void arg_check(int argc, char * argv[]);
void do_udp();
void do_tcp();


int main(int argc, char * argv[]) {
  /*  test vstupnich parametru: */
  arg_check(argc, argv);
}

/* Funkce pro kontrolu zadaných argumentů */
void arg_check(int argc, char * argv[]){
  if (argc != 7) {
      fprintf(stderr,"Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
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
      fprintf(stderr, "Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
      exit(EXIT_FAILURE);
    }
  }

  if(strcmp(mode, "udp") == 0){
    do_udp();
  }
  else if(strcmp(mode, "tcp") == 0){
    do_tcp();
  }
  else{
    fprintf(stderr, "Wrong input argument <mode>!\n");
    fprintf(stderr, "Usage: ipkcpc -h <host> -p <port> -m <mode>\n");
    exit(EXIT_FAILURE);
  }
}

void do_udp(){
    return 0;
}

void do_tcp(){
    return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <pthread.h>
#include <netdb.h>


// Globals
int tracker_port;
char tracker_ip[20];
int connections[1];
pthread_t childListen;
pthread_t childConnect;
unsigned int roomnum;
int sock;
struct sockaddr_in addr;

// Function Prototypes
void parse_args(int argc, char **argv);

int main(int argc, char **argv){
    
  parse_args(argc, argv);
  
  sock = socket(PF_INET, SOCK_STREAM, 0);
  
  if(sock < 0) {
	stderr("Issue Creating Socket");
	return 0;
	}
	
	parse_args(argc, argv);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
		stderr("Issue binding");
		return 0;
	}
	
	pthread_create(&childListen, 0, listener,0);
	pthread_create(&childConnect, 0, connector, 0);
	while(1) {
	
	}
	
	return 0;
}

void parse_args(int argc, char **argv){
	if (argc != 3) {
		fprintf(stderr, "%s\n", "Argument number not correct");
	}
	tracker_port = atoi(argv[2]);
	memcpy(tracker_ip, argv[1], (strlen(argv[1]) + 1 > sizeof(tracker_ip)) ? sizeof(tracker_ip) : strlen(argv[1]));
}

void* listener() {

}

void* connector() {

}

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

// Globals
int tracker_port;
char tracker_ip[20];
int connections[1];
//Threads
unsigned int roomnum;


// Function Prototypes
void parse_args(int argc, char **argv);

int main(int argc, char **argv){
  struct sockaddr_in addr;
  int sock, port;
  
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
	
	if(listen(sock, 5) != 0) {
		stderr("Issue in listening");
		return 0;
	}
	else {
		int k;
		pthread_t child;
		
		while(1) {
			k = accept(sock, 0, 0)
			
		}
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

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


// Function Prototypes
void parse_args(int argc, char **argv);

int main(int argc, char **argv){
parse_args(argc, argv);
	
	return 0;
}

void parse_args(int argc, char **argv){
	if (argc != 3) {
		fprintf(stderr, "%s\n", "Argument number not correct");
	}
	tracker_port = atoi(argv[2]);
	memcpy(tracker_ip, argv[1], (strlen(argv[1]) + 1 > sizeof(tracker_ip)) ? sizeof(tracker_ip) : strlen(argv[1]));
}

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
int peer_port;
int tracker_port;
char tracker_ip[20];
int connections[100];
unsigned int roomnum;
int sock;
struct sockaddr_in addr;
int totalConnections;
pthread_mutex_t lock;
struct hostent* server;
struct sockaddr_in tracker_addr;
void* ks;

// Function Prototypes
void parse_args(int argc, char **argv);
void* connector();
void * listener(void * ptr);
void * listenForMessage(void * ptr);
void * sendMessage(void * ptr);
void * postText(void * ptr);


int main(int argc, char **argv){
	totalConnections = 0;
  if(pthread_mutex_init(&lock, NULL) != 0) {
	fprintf(stderr,"Mutex init failed");
	return 1;
  }
  
	pthread_t childListen;
  parse_args(argc, argv);
  
  sock = socket(PF_INET, SOCK_STREAM, 0);
  
  if(sock < 0) {
	fprintf(stderr,"Issue Creating Socket");
	return 0;
  }
	
	parse_args(argc, argv);
	
	// server = (struct hostent *) gethostbyname(tracker_ip);

	// addr of tracker
	tracker_addr.sin_family = AF_INET; 
	tracker_addr.sin_port = htons(tracker_port);
	if (inet_aton(tracker_ip, &tracker_addr.sin_addr) == 0) {
		fprintf(stderr,"tracker ip not valid");
		return 0;
	}
	
	memset(&addr, 0, sizeof(addr));
	
	// addr that other people in the chat room should send message to
	addr.sin_family = AF_INET;
	addr.sin_port = htons(peer_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
		fprintf(stderr,"Issue binding");
		return 0;
	}
	
	pthread_create(&childListen, NULL, listener, NULL);
	pthread_detach(childListen);
	connector();
	return 0;
}

void parse_args(int argc, char **argv){
	if (argc != 4) {
		fprintf(stderr, "%s\n", "Argument number not correct");
	}
	tracker_port = atoi(argv[2]);
	peer_port = atoi(argv[3]);
	memcpy(tracker_ip, argv[1], (strlen(argv[1]) + 1 > sizeof(tracker_ip)) ? sizeof(tracker_ip) : strlen(argv[1]));
}

void * listener(void * ptr) {
	pthread_t child;
	int connection;
	int* curNum;
	curNum = malloc(sizeof(int));
	if(listen(sock, 10) != 0) {
		fprintf(stderr,"Issue Listening");
		abort();
	}
	else {
		while(1) {
			connection = accept(sock, 0, 0);
			pthread_mutex_lock(&lock);
			connections[totalConnections] = connection;
			totalConnections++;			
			if(connection <= 0)  {
				totalConnections--;
			}	
			else {
				curNum = totalConnections;
				pthread_create(&child, NULL, listenForMessage State, (void*) curNum);
				pthread_detach(child);
			}
			pthread_mutex_unlock(&lock);
		}
	}
	return NULL;
	
}

void* connector() {
	int k;
	pthread_t child;
	k = connect(sock, (struct sockaddr*)  &addr, sizeof(addr));
	if( k == -1) {
		fprintf(stderr,"Issue connecting to server.")
		abort();
	}
	pthread_mutex_lock(&lock);
	connections[totalConnections] = k;
	
	totalConnections++;
	pthread_mutex_unlock(&lock);
	
	
	pthread_create(&child, NULL, ListenForMessage, (void*) curNum);
	pthread_detach(child);
	
	//Goto Send Messages	
	return NULL;
}

//Insert Listening for a message
void * listenForMessage(void * ptr) {

}

//Insert send message function
void * sendMessage(void * ptr) {

}

//Insert Post Message to Screen
void * postText(void * ptr) {

}
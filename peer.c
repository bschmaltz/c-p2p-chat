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
int connections[100]; //List of all current IP connections
unsigned int roomnum;
int sock;
struct sockaddr_in addr;
int totalConnections;
pthread_mutex_t lock; //Lock Mutex for connections
pthread_mutex_t lockOut; //Lock Mutex for Output
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

typedef struct message_ts {
	char* payload;
	int ip;
} message_t; //For Outputting Text


int main(int argc, char **argv){
	totalConnections = 0;
	pthread_t childListen;
	//Create Mutexes
	
  if(pthread_mutex_init(&lock, NULL) != 0) {
	fprintf(stderr,"Mutex lock init failed");
	return 1;
  }
  
  if(pthread_mutex_init(&lockOut, NULL) != 0) {
	fprintf(stderr,"Mutex out init failed");
	return 1;
  }
  
  parse_args(argc, argv);
  
  //Create Socket
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
	
	//Create a thread to listen for incoming connections
	pthread_create(&childListen, NULL, listener, NULL);
	pthread_detach(childListen);

	connector(); //Connect to given IP (usually for tracker)

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
	pthread_t child; //For creating threads for waiting for incoming messages to output
	int connection; //The IP of whoever connects
	int* curNum; //For Ptr

	curNum = malloc(sizeof(int));

	if(listen(sock, 10) != 0) {
		fprintf(stderr,"Issue Listening");
		abort();
	}

	else {
		while(1) {
			connection = accept(sock, 0, 0);
			pthread_mutex_lock(&lock); //Lock up for list of Connections
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
	
	sendMessage(NULL);
	return NULL;
}

//Insert Listening for a message
void * listenForMessage(void * ptr) {
	char buff[999];
	message_header* head;
	int connected = connections[(int) ptr];
	int connectNum = (int) ptr;
	int i;
	while(1) {
		head = malloc(sizeof(message_header));
		bzero(buff, 998);
		if(recv(sock, head, sizeof(message_header), 0) < 0) {
			pthread_mutex_lock(&lock);
			for(i = connectNum; connectNum < totalConnections - 1; connectNum++) {
				connections[connectNum] = connections[connectNum + 1];
			}
			totalConnections--;
			pthread_mutex_unlock(&lock);
			pthread_exit();
		}
		//Insert reading of header here.
		//Insert reading real message here, if needed
	}
	return NULL;
}

//Insert send message function
void * sendMessage(void * ptr) {
	while(1) {
		//Read text input
		//Output using postText
		//iterate and send to
	}
	return NULL;
}

//Insert Post Message to Screen
void * postText(void * ptr) { //ptr will be the message to print
	pthread_mutex_lock(&lockOut);
	
	fprintf(stdout, "\n %d : %s", (message_t*) ptr->ip, (message_t*) ptr->payload);
	
	pthread_mutex_unlock(&lockOut);
	
	return NULL;
}
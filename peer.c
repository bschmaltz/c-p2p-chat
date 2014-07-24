#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
#include "message.h"

// Globals
int peer_port;
int tracker_port;
char tracker_ip[20];
char connectionValid[100];
int connections[100]; //List of all current IP connections
int connectionToTracker;
unsigned int roomnum;
int sock;
struct sockaddr_in addr;
int totalConnections;
int totalConnectionsMade;
pthread_mutex_t lock; //Lock Mutex for connections
pthread_mutex_t lockOut; //Lock Mutex for Output
pthread_mutex_t lockSend; //Lock Mutex for Send
struct hostent* server;
struct sockaddr_in tracker_addr;
message_header* toSend;
char* buffToSend;

int sendMessageFlag = 0;
int numSent = 0;

// Function Prototypes
void parse_args(int argc, char **argv);
void* connector();
void * listener(void * ptr);
void * listenForMessage(void * ptr);
void * sendMessage(void * ptr);
void * postText(void * ptr);
void * threadSend(void * ptr);

typedef struct message_ts {
	char* payload;
	int ip;
} message_t; //For Outputting Text


int main(int argc, char **argv){
	totalConnections = 0;
	memset(connectionValid, 0, sizeof(connectionValid));
	// totalConnectionsMade = 0;
	
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

	if(pthread_mutex_init(&lockSend, NULL) != 0) {
		fprintf(stderr,"Mutex send init failed");
		return 1;
	}
  
  	parse_args(argc, argv);
  
  	//Create Socket
  	sock = socket(AF_INET, SOCK_STREAM, 0);
  
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

	curNum = (int *)malloc(sizeof(int));

	if(listen(sock, 10) != 0) {
		fprintf(stderr,"Issue Listening");
		abort();
	}

	else {
		while(1) {
			connection = accept(sock, 0, 0);
			if (connection >= 0) {
				pthread_mutex_lock(&lock); //Lock up for list of Connections
				int i;
				for (i = 0; i < 100; i++) {
					if (!connectionValid[i]) {
						connections[i] = connection;
						connectionValid[i] = 1;
						totalConnections++;
						break;
					}
				}
				// connections[totalConnectionsMade] = connection;
				// totalConnectionsMade++;
				// totalConnections++;
				if (i < 100) {
					*curNum = i;
					pthread_create(&child, NULL, listenForMessage, curNum);
					pthread_detach(child);
					pthread_create(&child, NULL, threadSend, curNum);
					pthread_detach(child);
				}
				else {
					// error handling
				}
				pthread_mutex_unlock(&lock);
			}
		}
	}
	return NULL;
	
}

void* connector() {
	int k;
	pthread_t child;
	int* curNum;
	curNum = (int *)malloc(sizeof(int));
	k = connect(sock, (struct sockaddr*)  &addr, sizeof(addr));
	if( k == -1) {
		fprintf(stderr,"Issue connecting to server.");
		abort();
	}
	pthread_mutex_lock(&lock);
	int i;
	for (i = 0; i < 100; i++) {
		if (!connectionValid[i]) {
			connections[i] = k;
			connectionValid[i] = 1;
			totalConnections++;
			break;
		}
	}
	// totalConnectionsMade++;
	// totalConnections++;
	// curNum = (int) totalConnectionsMade;

	if (i < 100) {
		*curNum = i;
		pthread_create(&child, NULL, listenForMessage, curNum);
		pthread_detach(child);
		pthread_create(&child, NULL, threadSend, curNum);
		pthread_detach(child);
	}

	pthread_mutex_unlock(&lock);
	
	sendMessage(NULL);
	return NULL;
}

//Insert Listening for a message
void * listenForMessage(void * ptr) {
	char buff[999];
	message_header* head;
	int connected = connections[*(int *)ptr];
	int connectNum = *(int *)ptr;
	int i;
	while(1) {
		head = (message_header *)malloc(sizeof(message_header));
		bzero(buff, 998);
		buff[998] = '\0';
		if(recv(sock, head, sizeof(message_header), 0) < 0) {
			pthread_mutex_lock(&lock);
			totalConnections--;
			connectionValid[*(int *)ptr] = 0;
			pthread_mutex_unlock(&lock);
			pthread_exit(NULL);
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
		//Set sendMessageFlag to high
	}
	return NULL;
}

void * threadSend(void * ptr) {
	int c = 0; //Sent message yet.
	int connected = connections[*(int *)ptr];
	int connectNum = *(int *)ptr;
	

	while(1) {
		while(sendMessageFlag == 0) {
			c = 0;
		}
		pthread_mutex_lock(&lockSend);
		//send the header
		//send the buffer
		//if send fails, end the pthread
		numSent++;
		if(numSent == totalConnections) {
			numSent = 0;
			sendMessageFlag = 0;
			c = 0;
		}
		else {
			c = 1;
		}
		pthread_mutex_unlock(&lockSend);
		while(c == 1) {
			if(sendMessageFlag == 0)
				c = 0;
		}
	}
}

//Insert Post Message to Screen
void * postText(void * ptr) { //ptr will be the message to print
	pthread_mutex_lock(&lockOut);
	
	fprintf(stdout, "\n %d : %s", ((message_t *)ptr)->ip, ((message_t *)ptr)->payload);
	
	pthread_mutex_unlock(&lockOut);
	
	return NULL;
}
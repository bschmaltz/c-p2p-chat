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
#include <pthread.h>
#include <netdb.h>
#include "message.h"

int sock;
// int peer_port;
// int tracker_port;
// char tracker_ip[20];
struct sockaddr_in tracker_addr;
struct sockaddr_in self_addr;
struct sockaddr_in peer_list[100];
int room_num = -1;
int peer_num = 0;

pthread_mutex_t stdout_lock;

// Function Prototypes
void parse_args(int argc, char **argv);
void read_input();
void create_room_request();
void join_room_request(int new_room_num);
void leave_room_request();
void send_message(char *msg);
void request_available_rooms();

int main(int argc, char **argv) {

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, "%s\n", "error : error creating socket.");
		abort();
	}

	parse_args(argc, argv);

	if (bind(sock, (struct sockaddr *)&self_addr, sizeof(self_addr))) {
		fprintf(stderr, "%s\n", "error : error binding.");
		abort();
	}

	read_input();

}

void parse_args(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "%s\n", "error : Argument number not correct");
	}

	short tracker_port = atoi(argv[2]);
	short self_port = atoi(argv[3]);
	char tracker_ip[20];
	memcpy(tracker_ip, argv[1], (strlen(argv[1]) + 1 > sizeof(tracker_ip)) ? sizeof(tracker_ip) : strlen(argv[1]));

	self_addr.sin_family = AF_INET; 
	self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	self_addr.sin_port = htons(self_port);

	tracker_addr.sin_family = AF_INET; 
	if (inet_aton(tracker_ip, &tracker_addr.sin_addr) == 0) {
		fprintf(stderr, "%s\n", "error : error parsing tracker ip.");
		abort();
	}
	tracker_addr.sin_port = htons(tracker_port);
}

void read_input() {

	char line[1000];
	char *p;
	while (1) {
		// read input
		memset(line, 0, sizeof(line));
		p = fgets(line, sizeof(line), stdin);
		// flush input stream to clear out long message
		// while ((ch = getchar()) != '\n' && ch != EOF);
		if (p == NULL) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s\n", "error : cannot read input");
			pthread_mutex_unlock(&stdout_lock);
			continue;
		}
		if (line[strlen(line) - 1] != '\n') {
			// flush input stream to clear out long message
			scanf ("%*[^\n]"); 
			(void) getchar ();
		}
		line[strlen(line) - 1] = '\0';

		// parse input
		if (line[0] != '-') {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s\n", "error : input format is not correct.");
			pthread_mutex_unlock(&stdout_lock);
			continue;
		}
		int new_room_num;
		switch (line[1]) {
			case 'c': 
				create_room_request();
				break;
			case 'j':
				new_room_num = atoi(line + 3);
				if (new_room_num < 0) {
					pthread_mutex_lock(&stdout_lock);
					fprintf(stderr, "%s\n", "error : room number invalid.");
					pthread_mutex_unlock(&stdout_lock);
				}
				else {
					join_room_request(new_room_num);
				}
				break;
			case 'l':
				leave_room_request();
				break;
			case 'm':
				send_message(line + 3);
				break;
			case 'r':
				request_available_rooms();
				break;
			default:
				pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "%s\n", "error : request type unknown.");
				pthread_mutex_unlock(&stdout_lock);
				break;
		}
	}
}

void create_room_request() {
	// format packet
	packet pkt;
	pkt.header.type = 'l';
	pkt.header.error = '\0';
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error : error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}
void join_room_request(int new_room_num) {
	// format packet
	packet pkt;
	pkt.header.type = 'j';
	pkt.header.error = '\0';
	pkt.header.room = new_room_num;
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error : error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}
void leave_room_request() {
	// format packet
	packet pkt;
	pkt.header.type = 'l';
	pkt.header.error = '\0';
	pkt.header.room = room_num;
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error : error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void send_message(char *msg) {
	if (msg[0] == '\0') {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error : no message content.");
		pthread_mutex_unlock(&stdout_lock);
	}

	// format packet
	packet pkt;
	pkt.header.type = 'm';
	pkt.header.error = '\0';
	pkt.header.room = room_num;
	pkt.header.payload_length = strlen(msg) + 1;
	memcpy(pkt.payload, msg, pkt.header.payload_length);
	printf("%s\n", pkt.payload);

	// send packet to every peer
	int i;
	int status;
	for (i = 0; i < peer_num; i++) {
		status = sendto(sock, &pkt, sizeof(pkt.header) + pkt.header.payload_length, 0, (struct sockaddr *)&(peer_list[i]), sizeof(peer_list[i]));
		if (status == -1) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s %d\n", "error : error sending packet to peer", i);
			pthread_mutex_unlock(&stdout_lock);
		}
	}
}

void request_available_rooms() {
	// format packet
	packet pkt;
	pkt.header.type = 'r';
	pkt.header.error = '\0';
	pkt.header.room = room_num;
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error : error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}


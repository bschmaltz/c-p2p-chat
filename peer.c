#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <netdb.h>
#include "message.h"

int sock;
struct sockaddr_in tracker_addr;
struct sockaddr_in self_addr;
struct sockaddr_in peer_list[100];
unsigned int room_num = 0;
int peer_num = 0;

pthread_mutex_t stdout_lock;
pthread_mutex_t peer_list_lock;

// Function Prototypes
void parse_args(int argc, char **argv);

void * read_input(void *ptr);
void create_room_request();
void join_room_request(int new_room_num);
void leave_room_request();
void send_message(char *msg);
void request_available_rooms();
void get_room_info();

void receive_packet();
void create_room_reply(packet *pkt);
void join_room_reply(packet *pkt);
void leave_room_reply(packet *pkt);
void user_connection_updates(packet *pkt);
void receive_available_rooms(packet *pkt);
void receive_message(struct sockaddr_in *sender_addr, packet *pkt);
void reply_to_ping(struct sockaddr_in *sender_addr);

void setup_test_peers();


int main(int argc, char **argv) {

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, "%s\n", "error - error creating socket.");
		abort();
	}

	parse_args(argc, argv);

	if (bind(sock, (struct sockaddr *)&self_addr, sizeof(self_addr))) {
		fprintf(stderr, "%s\n", "error - error binding.");
		abort();
	}

	// setup_test_peers();

	// create a thread to read user input
	pthread_t input_thread;
	pthread_create(&input_thread, NULL, read_input, NULL);
	pthread_detach(input_thread);

	receive_packet();
}

void parse_args(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "%s\n", "error - Argument number not correct");
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
		fprintf(stderr, "%s\n", "error - error parsing tracker ip.");
		abort();
	}
	tracker_addr.sin_port = htons(tracker_port);
}

void * read_input(void *ptr) {

	char line[1000];
	char *p;
	while (1) {
		// read input
		memset(line, 0, sizeof(line));
		p = fgets(line, sizeof(line), stdin);
		// flush input stream to clear out long message
		if (p == NULL) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s\n", "error - cannot read input");
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
			fprintf(stderr, "%s\n", "error - input format is not correct.");
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
					fprintf(stderr, "%s\n", "error - room number invalid.");
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
			case 'i':
				get_room_info();
				break;
			default:
				pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "%s\n", "error - request type unknown.");
				pthread_mutex_unlock(&stdout_lock);
				break;
		}
	}
  return NULL;
}

void receive_packet() {

	struct sockaddr_in sender_addr;
	socklen_t addrlen = 10;
	packet pkt;
	int status;

	while (1) {
		status = recvfrom(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sender_addr, &addrlen);
		if (status == -1) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s\n", "error - error receiving a packet, ignoring.");
			pthread_mutex_unlock(&stdout_lock);
			continue;
		}

		switch (pkt.header.type) {
			case 'c': 
				create_room_reply(&pkt);
				break;
			case 'j':
				join_room_reply(&pkt);
				break;
			case 'l':
				leave_room_reply(&pkt);
				break;
			case 'u': 
				user_connection_updates(&pkt);
				break;
			case 'r':
				receive_available_rooms(&pkt);
				break;
			case 'm':
				receive_message(&sender_addr, &pkt);
				break;
			case 'p':
				reply_to_ping(&sender_addr);
				break;
			default:
				pthread_mutex_lock(&stdout_lock);
				fprintf(stderr, "%s\n", "error - received packet type unknown.");
				pthread_mutex_unlock(&stdout_lock);
				break;
		}
	}
}

void create_room_request() {
	// format packet
	packet pkt;
	pkt.header.type = 'c';
	pkt.header.error = '\0';
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - error sending packet to tracker");
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
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - error sending packet to tracker");
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
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void send_message(char *msg) {
	if (msg[0] == '\0') {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - no message content.");
		pthread_mutex_unlock(&stdout_lock);
	}

	// format packet
	packet pkt;
	pkt.header.type = 'm';
	pkt.header.error = '\0';
	pkt.header.room = room_num;
	pkt.header.payload_length = strlen(msg) + 1;
	memcpy(pkt.payload, msg, pkt.header.payload_length);

	// send packet to every peer
	int i;
	int status;
	pthread_mutex_lock(&peer_list_lock);
	for (i = 0; i < peer_num; i++) {
		status = sendto(sock, &pkt, sizeof(pkt.header) + pkt.header.payload_length, 0, (struct sockaddr *)&(peer_list[i]), sizeof(struct sockaddr_in));
		if (status == -1) {
			pthread_mutex_lock(&stdout_lock);
			fprintf(stderr, "%s %d\n", "error - error sending packet to peer", i);
			pthread_mutex_unlock(&stdout_lock);
		}
	}
	pthread_mutex_unlock(&peer_list_lock);
}

void request_available_rooms() {
	// format packet
	packet pkt;
	pkt.header.type = 'r';
	pkt.header.error = '\0';
	pkt.header.room = room_num;
	pkt.header.payload_length = 0;

	// send packet to tracker
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)&tracker_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - error sending packet to tracker");
		pthread_mutex_unlock(&stdout_lock);
	}
}

// local method that print out a list of all peer in the chatroom
void get_room_info() {
	pthread_mutex_lock(&stdout_lock);
	if (peer_num == 0) {
		fprintf(stderr, "%s\n", "error - you are not in any room!");
	}
	else {
		printf("%s %d\n", "you are in chatroom number", room_num);
		printf("%s\n", "member: ");
		int i;
		char *peer_ip;
		short peer_port;
		for (i = 0; i < peer_num; i++) {
			peer_ip = inet_ntoa(peer_list[i].sin_addr);
			peer_port = htons(peer_list[i].sin_port);
			printf("%s:%d\n", peer_ip, peer_port);
		}
	}
	pthread_mutex_unlock(&stdout_lock);
}

void create_room_reply(packet *pkt) {
	// error checking
	char error = pkt->header.error;
	if (error != '\0') {
		pthread_mutex_lock(&stdout_lock);
		if (error == 'o') {
			fprintf(stderr, "%s\n", "error - the application is out of chatroom!");
		}
		else if (error == 'e') {
			fprintf(stderr, "%s\n", "error - you already exist in a chatroom!");
		}
		else {
			fprintf(stderr, "%s\n", "error - unspecified error.");
		}
		pthread_mutex_unlock(&stdout_lock);
		return;
	}

	pthread_mutex_lock(&peer_list_lock);
	room_num = pkt->header.room;
	peer_num = 1;
	memcpy(peer_list, &self_addr, sizeof(struct sockaddr_in));
	pthread_mutex_unlock(&peer_list_lock);
	pthread_mutex_lock(&stdout_lock);
	printf("%s %d\n", "You've created and joined chatroom", room_num);
	pthread_mutex_unlock(&stdout_lock);
}

void join_room_reply(packet *pkt) {
	// error checking
	char error = pkt->header.error;
	if (error != '\0') {
		pthread_mutex_lock(&stdout_lock);
		if (error == 'f') {
			fprintf(stderr, "%s\n", "error - the chatroom is full!");
		}
		else if (error == 'e') {
			fprintf(stderr, "%s\n", "error - the chatroom does not exist!");
		}
		else if (error == 'a') {
			fprintf(stderr, "%s\n", "error - you are already in that chatroom!");
		}
		else {
			fprintf(stderr, "%s\n", "error - unspecified error.");
		}
		pthread_mutex_unlock(&stdout_lock);
		return;
	}

	pthread_mutex_lock(&peer_list_lock);
	room_num = pkt->header.room;
	// store sockaddr list in payload
	peer_num = pkt->header.payload_length / sizeof(struct sockaddr_in);
	if (peer_num <= 0) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - peer list missing, can't join chatroom, leaving old chatroom if switching.");
		pthread_mutex_unlock(&stdout_lock);
		room_num = 0;
		peer_num = 0;
	}
	else {
		memcpy(peer_list, pkt->payload, peer_num * sizeof(struct sockaddr_in));
		pthread_mutex_lock(&stdout_lock);
		printf("%s %d\n", "you have joined chatroom", room_num);
		pthread_mutex_unlock(&stdout_lock);
	}
	pthread_mutex_unlock(&peer_list_lock);
}

void leave_room_reply(packet *pkt) {
	// error checking
	char error = pkt->header.error;
	if (error != '\0') {
		pthread_mutex_lock(&stdout_lock);
		if (error == 'e') {
			fprintf(stderr, "%s\n", "error - you are not in any chatroom!");
		}
		else {
			fprintf(stderr, "%s\n", "error - unspecified error.");
		}
		pthread_mutex_unlock(&stdout_lock);
		return;
	}

	pthread_mutex_lock(&peer_list_lock);
	room_num = 0;
	peer_num = 0;
	pthread_mutex_unlock(&peer_list_lock);
	pthread_mutex_lock(&stdout_lock);
	printf("%s\n", "you have left the chatroom.");
	pthread_mutex_unlock(&stdout_lock);
}

void user_connection_updates(packet *pkt) {

	pthread_mutex_lock(&peer_list_lock);
	// store sockaddr list in payload
	int new_peer_num = pkt->header.payload_length / sizeof(struct sockaddr_in);
	if (new_peer_num <= 0) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - peer list missing.");
		pthread_mutex_unlock(&stdout_lock);
	}
	else {
		pthread_mutex_lock(&stdout_lock);
		printf("%s\n", "room update recieved.");
		pthread_mutex_unlock(&stdout_lock);
		peer_num = new_peer_num;
		memcpy(peer_list, pkt->payload, peer_num * sizeof(struct sockaddr_in));
	}
	pthread_mutex_unlock(&peer_list_lock);
}

void receive_available_rooms(packet *pkt) {

	/*
	unsigned int *room_info = (unsigned int *)(pkt->payload);
	// check if length of payload is a power of 8 (2 unsigned int)
	int num_of_rooms = pkt->header.payload_length / 8;
	if (pkt->header.payload_length / 8) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "warning - packet not in correct format, available room list may be corrupted.");
		pthread_mutex_unlock(&stdout_lock);
	}

	int i;
	*/
	pthread_mutex_lock(&stdout_lock);
	/*
	printf("%s\n", "available room list: ");
	printf("%s\n", "chatroom number | number of chatters");
	for (i = 0; i < num_of_rooms; i++) {
		printf("%d | %d\n", room_info[2 * i], room_info[2 * i + 1]);
	}
	*/
	printf("Room List: \n%s", pkt->payload);
	pthread_mutex_unlock(&stdout_lock);
}

void receive_message(struct sockaddr_in *sender_addr, packet *pkt) {
	// fetch sender information
	char *sender_ip = inet_ntoa(sender_addr->sin_addr);
	short sender_port = htons(sender_addr->sin_port);

	// display message in stdout if the message is from the chatroom that peer is in
	if (pkt->header.room == room_num) {
		pthread_mutex_lock(&stdout_lock);
		printf("%s:%d - %s\n", sender_ip, sender_port, pkt->payload);
		pthread_mutex_unlock(&stdout_lock);
	}
}

void reply_to_ping(struct sockaddr_in *sender_addr) {
	// format packet
	packet pkt;
	pkt.header.type = 'p';
	pkt.header.error = '\0';
	pkt.header.payload_length = 0;

	// send ping reply
	int status = sendto(sock, &pkt, sizeof(pkt.header), 0, (struct sockaddr *)sender_addr, sizeof(struct sockaddr_in));
	if (status == -1) {
		pthread_mutex_lock(&stdout_lock);
		fprintf(stderr, "%s\n", "error - error replying to ping message, possibility of being opt-out.");
		pthread_mutex_unlock(&stdout_lock);
	}
}

void setup_test_peers() {
	char test_ip[] = "127.0.0.1";
	peer_num = 3;

	peer_list[0].sin_family = AF_INET; 
	inet_aton(test_ip, &peer_list[0].sin_addr);
	peer_list[0].sin_port = htons(8081);
	peer_list[1].sin_family = AF_INET; 
	inet_aton(test_ip, &peer_list[1].sin_addr);
	peer_list[1].sin_port = htons(8082);
	peer_list[2].sin_family = AF_INET; 
	inet_aton(test_ip, &peer_list[2].sin_addr);
	peer_list[2].sin_port = htons(8083);
}


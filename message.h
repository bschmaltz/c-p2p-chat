typedef struct message_header_t {
	char type;
	char error;
	unsigned int room;
	unsigned int payload_length;
} message_header;

typedef struct packet_t {
	struct message_header_t header;
	char payload[1000];
} packet;
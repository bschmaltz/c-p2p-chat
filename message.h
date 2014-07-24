typedef struct message_header_t {
	char type;
	char error;
	unsigned int room;
	unsigned int payload_length;
} message_header;
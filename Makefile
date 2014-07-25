CC = g++
DEBUG_FLAGS = -g -O0 -DDEBUG -pthread
CFLAGS = $(DEBUG_FLAGS) -Wall
RM = rm -f

all: tracker peer

tracker: tracker.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

peer: peer.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# peer_tcp: peer_udp.o
# 	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	
clean:
	$(RM) *.o tracker peer

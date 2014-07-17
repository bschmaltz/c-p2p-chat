CC = g++
DEBUG_FLAGS = -g -O0 -DDEBUG
CFLAGS = $(DEBUG_FLAGS) -Wall
RM = rm -f

all: tracker peer

tracker: tracker.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

peer: peer.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(RM) *.o tracker peer

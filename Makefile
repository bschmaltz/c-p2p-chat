CC = g++
DEBUG_FLAGS = -g -O0 -DDEBUG -pthread
CFLAGS = $(DEBUG_FLAGS) -Wall
RM = rm -f

all: tracker

tracker: tracker.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	
clean:
	$(RM) *.o tracker peer

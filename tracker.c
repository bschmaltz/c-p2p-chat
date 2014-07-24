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
#include "uthash.h"

struct peer {
  char ip_and_port[20];
  unsigned int room;
  UT_hash_handle hh;
};

// Globals
unsigned short g_usPort;
struct sockaddr_in svr_addr;
int svr_socket;
struct peer *peers;

// Function Prototypes
void parse_args(int argc, char **argv);
char add_peer(int ip, short port);


int main(int argc, char **argv){
  //setup hashtable
  peers = NULL;
  fprintf(stderr, "Hashtable created\n");

  parse_args(argc, argv);
  fprintf(stderr, "Starting server on port: %hu\n", g_usPort);
  //setup tracker accept port
  svr_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(svr_socket < 0)
  {
    fprintf(stderr, "Server socket failed to open\n");
    exit(1);
  }
  //set server socket address
  memset(&svr_addr, 0, sizeof(svr_addr));
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  svr_addr.sin_port = htons(g_usPort);
  //bind address to socket
  if(bind(svr_socket, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) < 0)
  {
    fprintf(stderr, "Server bind failed\n");
    exit(1);
  }
  //turn socket listening on
  if(listen(svr_socket, 10) < 0)
  {
    perror("Server listen failed\n");
    exit(1);
  }
  fprintf(stderr, "Server listening...\n");

  //wait to accept connections from peers

  return 0;
}

char add_peer(unsigned int ip, short peer, unsigned int room){
  struct peer *new_peer;
  memset(&new_peer, 0, sizeof(new_peer));
  //set key
  char *ip_and_port_format = (char *)"%d:%d";
  sprintf(new_peer->ip_and_port, ip_and_port_format, ip, peer);
  //set value
  new_peer->room = room;
  //add to table
  HASH_ADD_STR( peers, ip_and_port, new_peer );
  return 'a';
}

void parse_args(int argc, char **argv){
  if (argc < 2)
  {
    g_usPort = 8080;
  }
  else
  {
    errno = 0;
    char *endptr = NULL;
    unsigned long ulPort = strtoul(argv[1], &endptr, 10);

    if (0 == errno)
    {
      // If no other error, check for invalid input and range
      if ('\0' != endptr[0])
        errno = EINVAL;
      else if (ulPort > USHRT_MAX)
        errno = ERANGE;
    }
    if (0 != errno)
    {
      // Report any errors and abort
      fprintf(stderr, "Failed to parse port number \"%s\": %s\n",
              argv[1], strerror(errno));
      abort();
    }
    g_usPort = ulPort;
  }
}

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

// Globals
unsigned short g_usPort;

// Function Prototypes
void parse_args(int argc, char **argv);

int main(int argc, char **argv){
  struct sockaddr_in svr_addr;
  int svr_socket;

  parse_args(argc, argv);
  fprintf(stderr, "Starting server on port: %hu\n", g_usPort);

  //create server socket
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
  return 0;
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

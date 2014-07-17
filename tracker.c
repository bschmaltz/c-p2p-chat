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

// Function Prototypes
void parse_args(int argc, char **argv);

int main(int argc, char **argv){
  parse_args(argc, argv);
  return 0;
}

void parse_args(int argc, char **argv){

}

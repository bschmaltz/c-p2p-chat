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
#include <math.h>
#include "uthash.h"
#define MAX_NUM_ROOMS 10
#define MAX_ROOM_SIZE 5

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
void peer_join(unsigned int ip, short port, unsigned int room);
void peer_leave(unsigned int ip, short peer);
char* room_list();
char* peer_list(unsigned int room);
void test_hash_table();


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


  test_hash_table();

  return 0;
}

void peer_join(unsigned int ip, short peer, unsigned int room){
  struct peer *s;
  if(room<0 || room>=MAX_NUM_ROOMS){
    printf("Peer join failed - room number out of possible range.\n");
    return;
  }
  int r=0;
  for(s=peers; s != NULL; s=(struct peer *)s->hh.next){
    if(s->room == room){
      r = r+1;
      if(r>=MAX_ROOM_SIZE){
        printf("Peer join failed - room full.\n");
        return;
      }
    }
  }
  
  //setup entry
  struct peer *new_peer;
  new_peer = (struct peer *)malloc(sizeof(struct peer));
  char* ip_and_port_format = (char *)"%d:%d";
  sprintf(new_peer->ip_and_port, ip_and_port_format, ip, peer);
  new_peer->room = room;
  

  HASH_FIND_STR(peers, (new_peer->ip_and_port), s);
  if(s!=NULL && s->room==room){
    printf("Peer join failed - already in room.\n");
    return;
  }
  if(s==NULL){ 
    //peer not found - join
    HASH_ADD_STR( peers, ip_and_port, new_peer );
  }else{
    //peer found - switch
    //TODO: alert s->room that peer left by sending peer list
    HASH_REPLACE_STR( peers, ip_and_port, new_peer, s );
  }
}

void peer_leave(unsigned int ip, short peer){
  char ip_port[20];
  memset(ip_port, 0, sizeof(ip_port));
  char* ip_and_port_format = (char *)"%d:%d";
  sprintf(ip_port, ip_and_port_format, ip, peer);
  struct peer *s;
  HASH_FIND_STR(peers, ip_port, s);
  if(s!=NULL){
    HASH_DEL( peers, s);
    free(s);
  }
}

char* room_list(){
  int room_stats[MAX_NUM_ROOMS];
  memset(room_stats, 0, MAX_NUM_ROOMS * sizeof(room_stats[0]));
  struct peer *s;

  for(s=peers; s != NULL; s=(peer *)s->hh.next){
    room_stats[s->room] = room_stats[s->room]+1;
  }

  int max_room_size_len = (int)floor(log10((float)MAX_ROOM_SIZE)) + 1;
  int max_num_room_len = (int)floor(log10((float)MAX_NUM_ROOMS)) + 1;
  char *list_entry_format = (char *)"room: %d - %d/%d\n";
  char *list_entry = (char *)malloc(max_num_room_len +2*max_room_size_len+strlen(list_entry_format));
  char *list = (char *)malloc(max_num_room_len*(max_num_room_len+2*max_room_size_len+strlen(list_entry_format)));
  unsigned int i;
  char *list_i = list;
  for(i=0; i<sizeof(room_stats)/sizeof(room_stats[0]); i++){
    sprintf(list_entry, list_entry_format, i, room_stats[i], MAX_NUM_ROOMS);
    strcpy(list_i, list_entry);
    list_i += strlen(list_entry);
  }

  return list;
}

char* peer_list(unsigned int room){
  struct peer *s;
  int num_in_room = 0;
  for(s=peers; s != NULL; s=(peer *)s->hh.next){
    if(s->room==room){
      num_in_room = num_in_room+1;

    }
  }

  char *list = (char *)malloc(num_in_room*20);
  char *list_i = list;
  for(s=peers; s != NULL; s=(peer *)s->hh.next){
    if(s->room==room){
      strcpy(list_i, s->ip_and_port);
      list_i += strlen(s->ip_and_port);
    }
  }
  return list;
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

void test_hash_table(){
  fprintf(stderr, "test hash table\n");
  peer_join(1234, 1, 0);
  peer_join(1234, 1, 5);
  peer_join(1111, 1, 0);
  peer_join(6969, 2, 0);
  peer_join(88696988, 2, 0);
  peer_join(420, 2, MAX_NUM_ROOMS); //fail - room number out of possible range
  peer_join(420, 2, 1);
  peer_join(420, 2, 1); //fail - user already in room
  peer_join(420, 3, 1);
  peer_join(420, 4, 1);
  peer_join(420, 5, 1);
  peer_join(420, 6, 1);
  peer_join(420, 7, 1); //fail - room full
  peer_leave(6969, 2);
  peer_leave(6969444, 2);//unknown address, should just ignore
  fprintf(stderr, "room list\n%s\n", room_list());
  fprintf(stderr, "peer list for room 0\n%s\n", peer_list(0));
}

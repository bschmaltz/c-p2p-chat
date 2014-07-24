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
#define MAX_NUM_ROOMS 5
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
void peer_create_room(unsigned int ip, short peer, unsigned int room);
void peer_join(unsigned int ip, short port, unsigned int room);
void peer_leave(unsigned int ip, short peer);
char* room_list();
char* peer_list(unsigned int room);
int get_total_num_rooms();
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

void peer_create_room(unsigned int ip, short peer, unsigned int room){
  //check if room limit reached
  int number_of_rooms = get_total_num_rooms();
  if(number_of_rooms>=MAX_NUM_ROOMS){
    perror("Peer create room failed - max number of rooms reached.\n");
    return;
  }

  //check if room already exists
  struct peer *s;
  for(s=peers; s != NULL; s=(struct peer *)s->hh.next){
    if(s->room == room){
      perror("Peer create room failed - room already exist.\n");
      return;
    }
  }

  //create entry
  struct peer *new_peer;
  new_peer = (struct peer *)malloc(sizeof(struct peer));
  char* ip_and_port_format = (char *)"%d:%d";
  sprintf(new_peer->ip_and_port, ip_and_port_format, ip, peer);
  new_peer->room = room;

  //check if peer in a room
  HASH_FIND_STR(peers, (new_peer->ip_and_port), s);
  if(s!=NULL){
    perror("Peer create room failed - already in a room.\n");
  }else{
    HASH_ADD_STR( peers, ip_and_port, new_peer );  //create room - add peer
    fprintf(stderr, "%s created %d\n", new_peer->ip_and_port, room);
  }
}

void peer_join(unsigned int ip, short peer, unsigned int room){
  struct peer *s;
  int r=0;
  int room_exists = 0;
  for(s=peers; s != NULL; s=(struct peer *)s->hh.next){
    if(s->room == room){
      r = r+1;
      if(r>=MAX_ROOM_SIZE){
        perror("Peer join failed - room full.\n");
        return;
      }
      room_exists = 1;
    }
  }
  if (room_exists==0){
    perror("Peer join failed - room does not exist.\n");
    return;
  }
  
  //setup entry
  struct peer *new_peer;
  new_peer = (struct peer *)malloc(sizeof(struct peer));
  char* ip_and_port_format = (char *)"%d:%d";
  sprintf(new_peer->ip_and_port, ip_and_port_format, ip, peer);
  new_peer->room = room;
  

  HASH_FIND_STR(peers, (new_peer->ip_and_port), s);
  if(s!=NULL && s->room==room){
    perror("Peer join failed - already in room.\n");
    return;
  }
  int old_room_update = -1;
  if(s==NULL){ 
    //peer not found - join

    HASH_ADD_STR( peers, ip_and_port, new_peer );
  }else{
    //peer found - switch
    old_room_update = s->room;
    HASH_REPLACE_STR( peers, ip_and_port, new_peer, s );
  }
  if(old_room_update!=-1){
    fprintf(stderr, "%s peer switched from %d to %d.\n", new_peer->ip_and_port, old_room_update, room);
  }else{
    fprintf(stderr, "%s joined %d\n", new_peer->ip_and_port, room);
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
    unsigned int left_room = s->room;
    HASH_DEL( peers, s);
    free(s);
    fprintf(stderr, "%s left %d\n", ip_port, left_room);
  }
}

char* room_list(){
  int number_of_rooms = get_total_num_rooms();
  unsigned int room_indexed[number_of_rooms];
  memset(room_indexed, 0, number_of_rooms * sizeof(room_indexed[0]));
  unsigned int room_nums[number_of_rooms];
  memset(room_nums, 0, number_of_rooms * sizeof(room_nums[0]));
  int room_stats[number_of_rooms];
  memset(room_stats, 0, number_of_rooms * sizeof(room_stats[0]));
  struct peer *s;
  int max_occupants=0;
  unsigned int max_room_number =0;
  int num_rooms_indexed = 0;
  for(s=peers; s != NULL; s=(peer *)s->hh.next){
    int room_index=-1;
    unsigned int a;
    for(a=0; a<sizeof(room_nums)/sizeof(room_nums[0]); a++){
      if(room_nums[a]==s->room){
        room_index=a;
        break;
      }
    }
    if(room_indexed[room_index]==0){
      room_index = num_rooms_indexed;
      room_nums[room_index] = s->room;
      room_stats[room_index] = 1;
      room_indexed[room_index]= 1;
      num_rooms_indexed = num_rooms_indexed+1;
    }else{
      room_stats[room_index] = room_stats[room_index]+1;
    }
    if(room_stats[room_index]>max_occupants){
      max_occupants=room_stats[room_index];
    }
    if(room_nums[room_index]>max_room_number){
      max_room_number=room_nums[room_index];
    }
  }

  int max_room_num_len;
  if(max_room_number==0){
    max_room_num_len=1;
  }else{
    max_room_num_len=(int)floor(log10((float)max_room_number)) + 1;
  }
  int max_room_size_len = (int)floor(log10((float)MAX_ROOM_SIZE)) + 1;
  int max_num_room_len = (int)floor(log10((float)max_occupants)) + 1;
  char *list_entry_format = (char *)"room: %d - %d/%d\n";
  char *list_entry = (char *)malloc(max_room_num_len+max_num_room_len+max_room_size_len+strlen(list_entry_format));
  char *list = (char *)malloc(max_num_room_len*(max_room_num_len+max_num_room_len+max_room_size_len+strlen(list_entry_format)));
  unsigned int i;
  char *list_i = list;
  for(i=0; i<sizeof(room_stats)/sizeof(room_stats[0]); i++){
    sprintf(list_entry, list_entry_format, room_nums[i], room_stats[i], MAX_ROOM_SIZE);
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

int get_total_num_rooms(){
  int total = 0;
  unsigned int room_found[MAX_NUM_ROOMS];
  memset(room_found, 0, MAX_NUM_ROOMS * sizeof(room_found[0]));
  unsigned int rooms[MAX_NUM_ROOMS];
  memset(rooms, 0, MAX_NUM_ROOMS * sizeof(rooms[0]));

  struct peer *s;
  unsigned int a;
  for(s=peers; s != NULL; s=(peer *)s->hh.next){
    int found = 0;
    for(a=0; a<sizeof(rooms)/sizeof(rooms[0]); a++){
      if(rooms[a]==s->room && room_found[a]==1){
        found=1;
        break;
      }
    }
    if(found==0){
      room_found[total]=1;
      rooms[total]=s->room;
      total=total+1;
    }
  }

  return total;
}

void test_hash_table(){
  fprintf(stderr, "test hash table\n");
  peer_create_room(1234, 1, 0);
  peer_leave(1234, 1);
  peer_create_room(1234, 1, 5);
  peer_create_room(1111, 1, 0);
  peer_join(6969, 2, 0);
  peer_join(88696988, 2, 0);
  peer_create_room(420, 2, 1);
  peer_join(420, 2, 1); //fail - user already in room
  peer_join(420, 3, 1);
  peer_join(420, 4, 1);
  peer_join(420, 5, 1);
  peer_join(420, 6, 1);
  peer_join(420, 7, 1); //fail - room full
  peer_leave(6969, 2);
  peer_leave(444, 2);//unknown address, should just ignore
  peer_join(1234, 1, 0);
  fprintf(stderr, "room list\n%s\n", room_list());
  fprintf(stderr, "peer list for room 0\n%s\n", peer_list(0));
  fprintf(stderr, "peer list for room 1\n%s\n", peer_list(1));
  fprintf(stderr, "peer list for room 5\n%s\n", peer_list(5));
  peer_create_room(419, 1, 10);
  peer_create_room(419, 2, 11);
  peer_create_room(419, 3, 12);
  peer_create_room(419, 4, 13);
}

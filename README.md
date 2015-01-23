c-p2p-chat   +README
==========   
   
Peer to peer chat application written in C for CS 3251

Group: Robin Egg Blue Team
Members: Charles (Ben) Schmaltz, Alexander Leavitt, Yiqi Chen


run make to compile the code

to run tracker program, it takes in one optional parameter: 

  ./tracker %tracker_port%

  This will create two UDP sockets at ports %tracker_port% and %tracker_port+1%.
  If %tracker_port% is not specified, it uses 8080. So, the ports used are 8080 and 8081

to run peer program, it takes in 3 parameters: 

  ./peer %tracker_ip% %tracker_port% %peer_port%

peer program takes user inputs, and below is the input format:

to request a list of all available rooms and number of peers in each room: 
  
  -r

to request to create a new room: 

  -c

to request to join or switch to a new room: 

  -j %new_chatroom_number%

to request to leave a room

  -l

to send a message to peers in the chatroom: 

  -m %message_that_you_want_to_send%

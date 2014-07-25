README

run make to compile the code

to run tracker program, it takes in one optional parameter: 
	./trakcer %tracker_port%
	if tracker_port is not specified, it is set to 8080 by default

to run peer program, it takes in 3 parameters: 
	./peer_udp %tracker_ip% %tracker_port% %peer_port%

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


- CSCE 3530 Lab 4: Demonstrates TCP 3-way handshake and closing a TCP connection using a client-server architecture.
				   Furthermore, the client will send a 1KB (1024 bytes) file to the server and the
				   server will send acknowledgements of each 128-byte segment received.

PLEASE NOTE: when running the program, use "make all" to compile and to run the program,
			 do: ./server <port> for the server and ./client <port> for the client,
			 and use "make clean" to clean up the output files. The default port that I used for testing purposes
			 was port 24853, but you can use any port.
			 
IMPORTANT: A known bug/issue is that sometimes, the server will display "address in use" if it
was terminated via CTRL+Z or CTRL+C during the previous runtime. Reopening the terminal solves this issue.


--------------------------------------------------------------------------------------
								Comments and code usage:								
--------------------------------------------------------------------------------------
I wrote comments throughout my code that were in order, so they would be easy to read
and easy to follow. The program can be viewed beginning at server.c, where the user first
reads in the number of arguments and ensures that the port number can be specified during
runtime. After this, the socket is then initialized, and the server socket is bound to the 
address. After we establish our connection, the server is ready to accept incoming connections 
from the client.

Moving over to the client ->
	The client checks for the number of arguments as well, making sure that the user enters
	in a specified port number. After this, a socket connection to the server is established
	and we are ready to send over our TCP segments to the server. There is much redundancy
	when sending and receiving TCP segments, so I created two methods which greatly simplified
	the process: 'sendSegment' and 'printAndWrite.' the sendSegment method creates a new
	TCP header struct from the tcpStruct.h header file. The parameters of this method are passed
	as the values in the struct, and the TCP segment is written to the server, who then receives
	it, prints it out and writes it to the server.out file. When the server is ready to send a TCP
	segment, the 'printAndWrite' method makes it easy to output the contents of the segment to the 
	console and to the client.out file.
	Going back to the server:

The server functions the same way as the client in the sense that the 'sendSegment' and 'printAndWrite'
methods are also present, because both the client and server are sending and receiving TCP segments.
The server closes its socket connection after the final close acknowledgement from the client.

Addition for lab 4: 
-   The only addition for lab 4 was the transfer of a 1 KB text file. I added this after the first 
	few acknowledgements; the client starts off by sending a 128-byte payload to the server,
	and the server responds with a TCP acknowledgement segment. This process is repeated another
	7 times until the whole 1024 bytes is transferred over.



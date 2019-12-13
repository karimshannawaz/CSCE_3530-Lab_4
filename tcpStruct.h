
// Header file that holds the structure for a TCP header.
// Received from the professor.

struct tcpHeader {
	unsigned short int sourcePort;
	unsigned short int destPort;
	unsigned int seq;
	unsigned int ack;
	unsigned short int flags;
	unsigned short int rec;
	unsigned short int cksum;
	unsigned short int ptr;
	unsigned int opt;
};

/**
	Lab 4 CSCE 3530
	Client that connects to the server
	Demonstrates TCP handshake using TCP segments.

 */

#include <time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "tcpStruct.h"

// Represents the bit positions of the flags.
const int urgBitFlag = 5, ackBitFlag = 4, pshBitFlag = 3, rstBitFlag = 2, synBitFlag = 1, finBitFlag = 0;

// Connects to the TCP server using the specified hostname (cse01) and the port (Default: 24853)
int connectToServer(int portNumber);

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * errorMessage);

// Returns true if the specified file exists.
bool fileExists(const char* file);

// Converts a number to its binary representation
char* toBinary(int number);

// Computes the checksum for the given TCP header
// (This code is from the professor)
unsigned int computeChecksum(struct tcpHeader tcp_seg);

// Prints out the contents of the TCP segment and writes them to the file.
void printAndWrite(FILE* fp, struct tcpHeader segment, int dataPayload);

// Sends a TCP segment with the following parameters to the server.
void sendSegment(int sockfd, FILE* fp, char* message, int port, int seqNum, 
	int ackNum, int ackBit, int synBit, int finBit, int dataPayload);


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Invalid command lind arguments; Usage: ./client <port>\n");
        return 1;
    }
	
	// Create the output file if it doesn't exist.
	if(!fileExists("./client.out")) {
		FILE *fp = fopen("client.out", "ab+");
		fclose(fp);
	}
	
    // The server port that we will connect to
    const int tcpServerPort = atoi(argv[1]);
   
    // Establishes the connection to the proxy server through cse01
    int sockfd = connectToServer(tcpServerPort);
	
	// Closes our connection once we're done with it.
    close(sockfd);
    return 0;
}

int connectToServer(int portNumber) {
	// Declare our current socket
    int sockfd;
	
	// The address information for the connector
    struct sockaddr_in serverAddress;
    struct hostent* server;

	// Creates the socket here.
    printErrorMessage((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1, "Failed to connect through the socket.");
    printErrorMessage((server = gethostbyname("cse01")) == NULL, "Failed to connect to cse01.");

    // Here we create the server address.
	// Zeros out the structure to create the address.
    bzero(&(serverAddress.sin_zero), 8);

	// Byte order of the host.
    serverAddress.sin_family = AF_INET;
	// Sets the port to connect through.
    serverAddress.sin_port = htons(portNumber);
	
	// Finally establishes a connection to the server.
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
	printErrorMessage(connect(sockfd, (struct sockaddr *)&serverAddress, 
		sizeof(struct sockaddr)) == -1, "Failed to connect to the server.");
	
	// Opens up our file for writing.
	FILE * fp;
	fp = fopen("./client.out", "w");
	
	// Creates a connection request TCP segment
	sendSegment(sockfd, fp, "Sending TCP segment to the server:\n\n", 
		portNumber, 64028, 0, 0, 1, 0, 0);
	
	// The server responds to the request by creating a connection granted TCP segment
	struct tcpHeader fromServer1;
	read(sockfd, ((void*) &fromServer1), sizeof(fromServer1));
	printf("Received connection granted TCP segment from the server:\n\n");
	fprintf(fp, "Received connection granted TCP segment from the server:\n\n");
	printAndWrite(fp, fromServer1, 0);
	
	// The client responds back with an acknowledgement TCP segment.
	sendSegment(sockfd, fp, "Sending acknowledgement TCP segment to the server:\n\n", 
		portNumber, 64029, fromServer1.seq + 1, 1, 0, 0, 0);

		
	// After this line, the 8 text file segments will be transferred to the server.
	printf("-----------------------------------------------------------\n\n");
	fprintf(fp, "-----------------------------------------------------------\n\n");
	
	// The client sends 128 bytes to the server.
	sendSegment(sockfd, fp, "Sending 128-byte text file payload to the server... (1)\n\n", 
		portNumber, 64030 + 128, fromServer1.seq + 1, 1, 0, 0, 128);
			
	// The server responds to the payload with an ACK
	struct tcpHeader fromServerF;
	read(sockfd, ((void*) &fromServerF), sizeof(fromServerF));
	printf("Received payload ACK TCP segment from the server (1):\n\n");
	fprintf(fp, "Received payload ACK TCP segment from the server (1):\n\n");
	printAndWrite(fp, fromServerF, 0);
	
	// The remaining 7 file-transfer segments are sent to the server.
	for(int count = 1; count < 8; count++) {
		// The client sends 128 bytes to the server.
		printf("Sending 128-byte text file payload to the server... (%d)\n\n", count + 1);
		fprintf(fp, "Sending 128-byte text file payload to the server... (%d)\n\n", count + 1);
		sendSegment(sockfd, fp, NULL, 
			portNumber, 64030 + count + 128, fromServerF.seq + 1, 1, 0, 0, 128);
				
		// The server responds to the payload with an ACK
		memset(&fromServerF, 0, sizeof(fromServerF));
		read(sockfd, ((void*) &fromServerF), sizeof(fromServerF));
		printf("Received payload ACK TCP segment from the server (%d):\n\n", count + 1);
		fprintf(fp, "Received payload ACK TCP segment from the server (%d):\n\n", count + 1);
		printAndWrite(fp, fromServerF, 0);
	}
	
		
	// After this line, the close request TCP segments will be sent
	// and received.
	printf("-----------------------------------------------------------\n\n");
	fprintf(fp, "-----------------------------------------------------------\n\n");
	
	// Creates a close request TCP segment.
	sendSegment(sockfd, fp, "Sending close request TCP segment to the server:\n\n", 
		portNumber, 57682, 0, 0, 0, 1, 0);
		
	// The server responds back with an acknowledgment TCP segment.
	struct tcpHeader fromServer2;
	read(sockfd, ((void*) &fromServer2), sizeof(fromServer2));
	printf("Received close request acknowledgement TCP segment from the server:\n\n");
	fprintf(fp, "Received close request acknowledgement TCP segment from the server:\n\n");
	printAndWrite(fp, fromServer2, 0);
	
	// The server again sends another close acknowledgement TCP segment. 
	struct tcpHeader fromServer3;
	read(sockfd, ((void*) &fromServer3), sizeof(fromServer3));
	printf("Received SECOND close acknowledgement TCP segment from the server:\n\n");
	fprintf(fp, "Received SECOND close acknowledgement TCP segment from the server:\n\n");
	printAndWrite(fp, fromServer3, 0);
	
	// The client responds back with a final acknowledgement TCP segment.
	sendSegment(sockfd, fp, "Sending final close acknowledgement TCP segment to the server:\n\n", 
		portNumber, 57683, fromServer3.seq + 1, 1, 0, 0, 0);
	
	
	printf("\n");
	// Closes our file writer.
	fclose(fp);
	return sockfd;
}

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * errorMessage) {
	// Only prints if the condition is met.
	if (!condition) 
		return;
	perror(errorMessage);
	exit(1);
}


// Returns true if the specified file exists.
bool fileExists(const char* file) {
    struct stat buffer;
    int exists = stat(file, &buffer);
    if(exists == 0)
        return true;
    else
        return false;
}

// Converts a number to its binary representation
char* toBinary(int num) { 
	char asString[100];
	asString[0] = '\0';
	strcat(asString, "0x");
    // 6 flags total
    for (int index = 5; index >= 0; index--) { 
        int k = num >> index; 
        if (k & 1) {
            strcat(asString, "1");
        } else {
           strcat(asString, "0");
		}
    } 
	return strdup(asString);
} 

// Computes the checksum for the given TCP header
// (This code is from the professor)
unsigned int computeChecksum(struct tcpHeader tcp_seg) {
	unsigned short int cksum_arr[12];
	memcpy(cksum_arr, &tcp_seg, 24);
	unsigned int i, sum = 0, cksum;
	for (i = 0; i < 12; i++) {
		sum = sum + cksum_arr[i];
	}

	cksum = sum >> 16; // Fold once
	sum = sum & 0x0000FFFF; 
	sum = cksum + sum;

	cksum = sum >> 16; // Fold once more
	sum = sum & 0x0000FFFF;
	cksum = cksum + sum;
	return cksum;
}

// Sends a TCP segment with the following parameters to the server.
void sendSegment(int sockfd, FILE* fp, char* message, int port, int seqNum, 
	int ackNum, int ackBit, int synBit, int finBit, int dataPayload) {
	// Prints and writes the specified message.
	if(message != NULL) {
		printf("%s", message);
		fprintf(fp, "%s", message);
	}
	
	// Creates a new TCP header and fills the fields with the values
	// that were passed to the parameters.
	struct tcpHeader segment;
	segment.sourcePort = port;
	segment.destPort = port;
	segment.seq = seqNum;
	segment.ack = ackNum;
	segment.flags = 0x000000;
	if(ackBit == 1) {
		// Sets the ACK bit to 1
		segment.flags |= (1 << ackBitFlag);
	}
	if(synBit == 1) {
		// Sets the SYN bit to 1
		segment.flags |= (1 << synBitFlag);
	}
	if(finBit == 1) {
		// Sets the FIN bit to 1
		segment.flags |= (1 << finBitFlag);
	}
	segment.rec = 0;
	segment.cksum = 0;
	segment.ptr = 0;
	segment.opt = 0;
	segment.cksum = computeChecksum(segment);
	
	// Prints and writes the segment out.
	printAndWrite(fp, segment, dataPayload);

	// TCP segment is sent to the server through the socket
	send(sockfd, ((void*) &segment), sizeof(segment), 0);
}

// Prints out the contents of the TCP segment and writes them to the file.
void printAndWrite(FILE* fp, struct tcpHeader segment, int dataPayload) {
	printf("Source port: %d\n", segment.sourcePort);
	fprintf(fp, "Source port: %d\n", segment.sourcePort);
	printf("Destination port: %d\n", segment.destPort);
	fprintf(fp, "Destination port: %d\n", segment.destPort);
	printf("Sequence num: %d\n", segment.seq);
	fprintf(fp, "Sequence num: %d\n", segment.seq);
	printf("Ack num: %d\n", segment.ack);
	fprintf(fp, "Ack num: %d\n", segment.ack);
	printf("Offset: 6, Header Length: 24\n");
	fprintf(fp, "Offset: 6, Header Length: 24\n");
	printf("Flags: %04u and flags' binary: %s\n", segment.flags, toBinary((int) segment.flags));
	fprintf(fp, "Flags: %04u and flags' binary: %s\n", segment.flags, toBinary((int) segment.flags));
	printf("Rec: 0x%04X\n", segment.rec);
	fprintf(fp, "Rec: 0x%04X\n", segment.rec);
	printf("Checksum value: 0x%04X\n", (0xFFFF^segment.cksum));
	fprintf(fp, "Checksum value: 0x%04X\n", (0xFFFF^segment.cksum));
	printf("Ptr: 0x%04X\n", segment.ptr);
	fprintf(fp, "Ptr: 0x%04X\n", segment.ptr);
	if(dataPayload > 0) {
		printf("Optional: 0x%08X\n", segment.opt);
		fprintf(fp, "Optional: 0x%08X\n", segment.opt);
		printf("Data payload: %d\n\n\n", dataPayload);
		fprintf(fp, "Data payload: %d\n\n\n", dataPayload);
	}
	else {
		printf("Optional: 0x%08X\n\n\n", segment.opt);
		fprintf(fp, "Optional: 0x%08X\n\n\n", segment.opt);
	}
}







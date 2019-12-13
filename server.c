
/**
	Lab 4 CSCE 3530
	Server that accepts single client’s request using sockets.
	Demonstrates TCP handshake using TCP segments.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "tcpStruct.h"

// Represents the bit positions of the flags.
const int urgBitFlag = 5, ackBitFlag = 4, pshBitFlag = 3, rstBitFlag = 2, synBitFlag = 1, finBitFlag = 0;

// Initializes the server and listens to requests on the network.
int initializeServer(int port);

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * error_mgs);

// Returns true if the specified word starts with the second parameter.
bool startsWith(const char* word, const char* startsWith);

// Returns true if the specified word in the first parameter contains the word in the second param.
bool contains(char* word, char* containsWord);

// Returns true if the specified file exists.
bool fileExists(const char* file);

// Computes the checksum for the given TCP header
// (This code is from the professor)
unsigned int computeChecksum(struct tcpHeader tcp_seg);

// Converts a number to its binary representation
char* toBinary(int number);

// Prints out the contents of the TCP segment and writes them to the file.
void printAndWrite(FILE* fp, struct tcpHeader segment, int dataPayload);

// Sends a TCP segment with the following parameters to the client.
void sendSegment(int connectionFD, FILE* fp, char* message, int port, int seqNum, 
	int ackNum, int ackBit, int synBit, int finBit, int dataPayload);

// Main function which runs the program.
int main(int argc, char** argv) {
		
	if (argc != 2) {
        printf("Invalid command lind arguments; Usage: ./server <port>\n");
        return 1;
    }  
	
	// Create the output file if it doesn't exist.
	if(!fileExists("./server.out")) {
		FILE *fp = fopen("server.out", "ab+");
		fclose(fp);
	}

	// Opens up our file for writing.
	FILE * fp;
	fp = fopen("./server.out", "w");
	
	// Grabs the port number from the user.
	int portNumber = atoi(argv[1]);
	
    // Starts to accept requests from the network.
    int sockfd = initializeServer(portNumber);
    printf("Socket established; waiting on client to connect...\n");
	
	// Incoming connections are handled here; any newly connected client will appear
	// and have its own unique connection ID.
	int connectionFD = accept(sockfd, (struct sockaddr*) NULL, NULL);
	printf("Client connected.\n\n");
	
	// Gets the connection request TCP segment from the client.
	struct tcpHeader fromClient1;
	read(connectionFD, ((void*) &fromClient1), sizeof(fromClient1));
	printf("TCP Connection granted.\nReceived TCP segment from client:\n\n");
	fprintf(fp, "TCP Connection granted.\nReceived TCP segment from client:\n\n");
	printAndWrite(fp, fromClient1, 0);
	
	// The server responds to the request by creating a connection granted TCP segment.
	sendSegment(connectionFD, fp, "Sending connection granted TCP segment to the client:\n\n",
		portNumber, 60298, fromClient1.seq + 1, 1, 1, 0, 0);
	
	// The client responds back with an acknowledgement TCP segment.
	struct tcpHeader fromClient2;
	read(connectionFD, ((void*) &fromClient2), sizeof(fromClient2));
	printf("Received acknowledgement TCP segment from client:\n\n");
	fprintf(fp, "Received acknowledgement TCP segment from client:\n\n");
	printAndWrite(fp, fromClient2, 0);
	
	
	// After this line, the 8 text file segments will be received and ack
	// segments will be sent to the client.
	printf("-----------------------------------------------------------\n\n");
	fprintf(fp, "-----------------------------------------------------------\n\n");
	
	// The client sends 128-byte payload of a text file.
	struct tcpHeader fromClientF;
	read(connectionFD, ((void*) &fromClientF), sizeof(fromClientF));
	printf("Received 128-byte text file payload from client (1):\n\n");
	fprintf(fp, "Received 128-byte text file payload from client (1):\n\n");
	printAndWrite(fp, fromClientF, 128);
	
	// The server responds to the client with an ack of the text file.
	sendSegment(connectionFD, fp, "Sending text file payload ACK to the client (1):\n\n",
		portNumber, 60299, fromClientF.seq + 1, 1, 1, 0, 0);
		
	// The remaining 7 file-transfer segments are received and acknowledged.
	for(int count = 1; count < 8; count++) {
		// The client sends 128-byte payload of a text file.
		memset(&fromClientF, 0, sizeof(fromClientF));
		read(connectionFD, ((void*) &fromClientF), sizeof(fromClientF));
		printf("Received 128-byte text file payload from client (%d):\n\n", count + 1);
		fprintf(fp, "Received 128-byte text file payload from client (%d):\n\n", count + 1);
		printAndWrite(fp, fromClientF, 128);
		
		// The server responds to the client with an ack of the text file.
		printf("Sending text file payload ACK to the client (%d):\n\n", count + 1);
		fprintf(fp, "Sending text file payload ACK to the client (%d):\n\n", count + 1);
		sendSegment(connectionFD, fp, NULL,
			portNumber, 60299 + count, fromClientF.seq + 1, 1, 1, 0, 0);
	}
	
	
	// After this line, the close request TCP segments will be sent
	// and received.
	printf("-----------------------------------------------------------\n\n");
	fprintf(fp, "-----------------------------------------------------------\n\n");
	
	// Gets the close request TCP segment from the client.
	struct tcpHeader fromClient3;
	read(connectionFD, ((void*) &fromClient3), sizeof(fromClient3));
	printf("Received close request TCP segment from client:\n\n");
	fprintf(fp, "Received close request TCP segment from client:\n\n");
	printAndWrite(fp, fromClient3, 0);
	
	// The server responds back with an acknowledgment TCP segment.
	sendSegment(connectionFD, fp, "Sending close request acknowledgement TCP segment to the client:\n\n",
		portNumber, 48118, fromClient3.seq + 1, 1, 0, 0, 0);
		
	// The server again sends another close acknowledgement TCP segment. 
	sendSegment(connectionFD, fp, "Sending SECOND close acknowledgement TCP segment to the client:\n\n",
		portNumber, 42804, fromClient3.seq + 1, 0, 0, 1, 0);
	
	// The client responds back with a final acknowledgement TCP segment.
	struct tcpHeader fromClient4;
	read(connectionFD, ((void*) &fromClient4), sizeof(fromClient4));
	printf("Received final close acknowledgement TCP segment from client:\n\n");
	fprintf(fp, "Received final close acknowledgement TCP segment from client:\n\n");
	printAndWrite(fp, fromClient4, 0);
		
	// Close our connection once we are done.
	close(connectionFD); 
	
	// Closes our socket once we are done using it.
	close(sockfd);
	
	// Closes our file writer.
	fclose(fp);
	return 0;
}

// Initializes the server and listens to requests on the network.
int initializeServer(int port) {
	
	// Creates our server socket and opens it for connections.
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;

    printErrorMessage(sockfd == -1, "Failed to initialize the server socket.");

	// Sets our server's address here.
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
	// Sets the port to be the same as the client.
    serverAddress.sin_port = htons(port);

    // Binds the socket to the address.
    printErrorMessage(bind(sockfd, (struct sockaddr *) &serverAddress, 
		sizeof(serverAddress)) == -1, "Failed to bind the server.");
    
	int option = 1;
    
	// This enables us to reuse our socket.
    printErrorMessage(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
		&option, sizeof(option)) == -1, "Failed on setsocketoption.");
    
	// We can listen to a maximum of 10 connections here.
    printErrorMessage(listen(sockfd, 10) != 0, "Failed to listen to the server.");

    return sockfd;
}

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * error_mgs) {
	// Only prints if the condition is met.
	if (!condition) 
		return;
	perror(error_mgs);
	exit(1);
}

// Returns true if the specified word starts with the second parameter.
bool startsWith(const char* word, const char* startsWith) {
	// Compares the two strings to see if there's a match.
	if(strncmp(word, startsWith, strlen(startsWith)) == 0) 
		return 1;
	return 0;
}

// Returns true if the specified word in the first parameter contains the word in the second param.
bool contains(char* word, char* containsWord) {
	if (strstr(word, containsWord) != NULL)
		return true;
	return false;
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

// Sends a TCP segment with the following parameters to the client.
void sendSegment(int connectionFD, FILE* fp, char* message, int port, 
	int seqNum, int ackNum, int ackBit, int synBit, int finBit, int dataPayload) {
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

	// TCP segment is sent to the client through the socket
	write(connectionFD, ((void*) &segment), sizeof(segment));
}



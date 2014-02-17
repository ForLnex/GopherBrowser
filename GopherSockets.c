#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
extern int errno;
const int MAXECHOLEN=16536;

int main (int argCount, char *argValues[]) {
	int socketDescriptor;
	struct sockaddr_in serverAddress;
	struct addrinfo hints, *res=NULL;
	char *testString="\r\n";
	char echoResponse[MAXECHOLEN];
	int status, i;
	char server[256], port[]="70";

	if (argCount != 2) {
		fprintf(stderr, "Usage: %s <server name>\n", argValues[0]);
		exit (-1);
	}
	strncpy(server, argValues[1],255);
	server[255] = '\0';
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_V4MAPPED | AI_ALL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int retCode = 0;
	retCode = inet_pton(AF_INET, server, &serverAddress);
	if (retCode == 1) { // valid ipv4 address
		hints.ai_family = AF_INET;
	} else {
		retCode = inet_pton(AF_INET6, server, &serverAddress);
		if (retCode == 1) { // valid ipv6 address 
			hints.ai_family = AF_INET6;
		}
	}

	retCode = getaddrinfo(server, port, &hints, &res);
	if (retCode != 0) {
		printf("Host not found ---> %s\n", gai_strerror(retCode));
		if (retCode == EAI_SYSTEM) {
			perror("getaddrinfo() failed");
		}
	}

	socketDescriptor = socket(	res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socketDescriptor < 0) {
		perror("Socket Creation Failed");
		exit(-1);
	}

	status = connect (socketDescriptor, res->ai_addr, res->ai_addrlen);
	if (status != 0) {
		perror("Connect Failed");
		exit(-1);
	}

	int bytesRead;
	status = write(socketDescriptor, testString, strlen(testString));
	do {
		bytesRead = read(socketDescriptor, echoResponse, MAXECHOLEN);
		echoResponse[bytesRead] = 0;
		printf("%s", echoResponse);
	} while (bytesRead > 0);
	close(socketDescriptor);
	exit(0);
}

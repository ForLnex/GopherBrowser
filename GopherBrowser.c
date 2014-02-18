#include <stdlib.h>
#include <stdbool.h>
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

#define NUM_TOKENS 5

char** tokenize(char* str, char delim);
int findChar(char* str, char delim, int startPos);
char* subString(char* str, int start, int end);

/*
 * The following opens a Gopher formatted file and prints its tokens.
 */
int main(int argc, char* argv[]){
	FILE* fp;
	char** tokens;
	char* line;
	size_t len = 0;
	ssize_t read;

	if(argc != 2)
		printf("Error! Usage: %s filename\n", argv[0]);
	else{
		fp = fopen(argv[1], "r");

		if(fp == 0){
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		//Read the whole file line-by-line, tokenize each line and print the tokens
		while ((read = getline(&line, &len, fp)) != -1){
			tokens = tokenize(line, '\t');
			for(int i = 0; i < NUM_TOKENS; i++)
				printf("%s\n", tokens[i]);
			
			//Free up memory that was allocated
			for(int i=0;i<NUM_TOKENS;i++)
				free(tokens[i]);
			free(tokens);
			free(line);
			
			//Setting line = NULL tells getline to handle memory allocation itself
			line = NULL;
		}
		free(line);
		fclose(fp);
		return 0;
	}
}

// This method reads the Gopher data via sockets rather than using Files
/*
 * This is commented out because I'm dumb.  echoResponse doesn't
 * just contain individual lines, it contains 0-MAXECHOLEN characters
 * from the server, newlines and all.  To get sockets to *really* work
 * I need to pull out the individual lines, and also set up a way to 
 * stitch together lines that are split between two reads.
 *
 * In other words:
 * TODO: tokenize the lines in the input.
int main (int argCount, char *argValues[]) {
	int socketDescriptor;
	char* testString="\r\n";
	struct sockaddr_in serverAddress;
	struct addrinfo hints, *res=NULL;
	char echoResponse[MAXECHOLEN];
	int status, i;
	char server[256], port[]="70";
	char** tokens;

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

	socketDescriptor = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
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
		printf("\n%s",echoResponse);	
		tokens = tokenize(echoResponse, '\t');
			for(int i = 0; i < NUM_TOKENS; i++)
				printf("%s\n", tokens[i]);
			
			//Free up memory that was allocated
			for(int i=0;i<NUM_TOKENS;i++)
				free(tokens[i]);
			free(tokens);
	} while (bytesRead > 0);
	close(socketDescriptor);
	exit(0);
}*/

//Because strtok is awful and I refuse to learn how to use it
char** tokenize(char* str, char delim){
	char** tokens;
	int lastEnd = 1;
	int newEnd = 0;

	if (str == NULL || delim == '\0') {
		return NULL;
	} else {
		tokens = malloc(NUM_TOKENS*(sizeof(char *)));
		tokens[0] = malloc(sizeof(char)*2);
		strncpy(tokens[0], str, 1);
		tokens[0][1] = '\0';

		for(int i = 1; i < NUM_TOKENS; ++i){
			if(i < NUM_TOKENS)
				newEnd = findChar(str, '\t', lastEnd);
			else
				newEnd = findChar(str, '\r', lastEnd);
			tokens[i] = subString(str, lastEnd, newEnd);
			lastEnd = newEnd+1;
		}
	}
	return tokens;
}

//finds the next instance of delim in str starting at startPos
int findChar(char* str, char delim, int startPos){
	for(int i = startPos; str[i] != '\0'; i++)
		if(str[i] == delim)
			return i;

	return -1;
}

//Because I'm already reimplimenting C's string functions to not be awful
char* subString(char* str, int start, int end){
	if(end == -1) {
		// Copy till end
		end = strlen(str);
	}
	int len = end - start;
	char *ret = malloc(len+1);

	for(int i = 0; i < len && str[start+i] != '\0'; i++)
		ret[i] = str[start+i];
	ret[len] = '\0';
	return ret;
}

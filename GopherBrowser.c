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
const size_t MAXECHOLEN=1024;

#define NUM_TOKENS 5

char** tokenize(char* str, char delim);
int findChar(char* str, char delim, int startPos);
char* subString(char* str, int start, int end);
ssize_t readLine(int fd, void* buffer, size_t n);
int getSocket(char* server, char* port);
/*
 * The following opens a Gopher formatted file and prints its tokens.
 *
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
}*/

typedef struct Link {
	char type[2];
	char url[100];
	char port[6];
} Link;

// This method reads the Gopher data via sockets rather than using Files
int main (int argCount, char *argValues[]) {
	int socketDescriptor, bytesRead, totalOptions;
	char echoResponse[MAXECHOLEN];
	char server[256];
	char** tokens;
	char port[]="70";
	Link links[100];

	if (argCount != 2) {
		fprintf(stderr, "Usage: %s <server name>\n", argValues[0]);
		exit (-1);
	}
	strncpy(server, argValues[1],255);
	server[255] = '\0';

	socketDescriptor = getSocket(server, port);
	
	totalOptions = 0;
	do {
		bytesRead = readLine(socketDescriptor, echoResponse, MAXECHOLEN);
		if (bytesRead == -1){
			printf("Improperly formatted server");	
			exit(EXIT_FAILURE);
		}
		printf("\n%s\n",echoResponse);	
		tokens = tokenize(echoResponse, '\t');
	
		if (tokens[0] != "i"){
			//type
			strcpy( links[totalOptions].type, tokens[0]);
			//url
			strcpy( links[totalOptions].url, tokens[3]);
		   strcat( links[totalOptions].url, 	"/?");
		   strcat( links[totalOptions].url,	tokens[0]);
			strcat( links[totalOptions].url, tokens[2]);
			//port
			strcpy( links[totalOptions].port, tokens[4]);
			totalOptions++;
			printf("%s) ", totalOptions);
			printf("%s\n", tokens[1]);
		}
		else
			printf("%s\n",tokens[1]);

		//Free up memory that was allocated
		for(int i=0;i<NUM_TOKENS;i++)
			free(tokens[i]);
		free(tokens);
	} while (bytesRead > 0);

	//getUserInput(links);
	
	//free up memory
	for(int i=0; i < totalOptions; i++){
		free(links[i].url);
		free(links[i].port);
		free(links[i]);
	}
	free(links);

	close(socketDescriptor);
	
	return 0;
}

void printDirectory(char* server, char* port){
	
}

int getSocket(char* server, char* port){
	int socketDescriptor;
	char* testString="\r\n";
	struct sockaddr_in serverAddress;
	struct addrinfo hints, *res=NULL;
	int status;

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

	status = write(socketDescriptor, testString, strlen(testString));

	return socketDescriptor;	
}

//Based heavily upon the function found at man7.org/tlpi/code/online/dist/sockets/read_line.c.html
//
//Changed to read until it finds a \r character, at which point it reads one more character,
//verifies that the character is \n, and then returns the line minus \r\n.  If \r is not followed
//by \n, returns an error (as Gopher lines must end with CRLF
ssize_t readLine(int fd, void* buffer, size_t n){
	ssize_t numRead;
	size_t totRead;
	char* buf;
	char ch;

	if (n <= 0 || buffer == NULL){
		errno = EINVAL;
		return -1;
	}
	buf = buffer;

	totRead = 0;
	for (;;){
		numRead = read(fd, &ch, 1);

		if (numRead == -1){
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		else if (numRead == 0) {
			if (totRead == 0)
				return 0;
			else
				break;
		}
		else{
			if (ch == '\r'){
				numRead = read(fd, &ch, 1);
				if (ch == '\n')	
					break;
				errno = EINVAL;
				return -1;
			}
			if (totRead < n - 1){
				totRead++;
				*buf++ = ch;
			}
		}
	}
	*buf = '\0';
	return totRead;
}


//Because strtok is awful and I refuse to learn how to use it.
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

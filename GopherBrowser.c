#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char** tokenize(char * str, char delim);
int findChar(char *str, char delim, int startPos);
char *subString(char * str, int start, int end);

int main(int argc, char *argv[]){
	FILE *fp;
	char ** tokens;
	char *line;
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
		while ((read = getline(&line, &len, fp)) != -1){
			tokens = tokenize(line, '\t');
			for(int i = 0; i < 5; i++)
				printf("%s\n", tokens[i]);
		}
		
		/*
		while(fgets(str, 200, fp)!=NULL){
		token = strtok(str, "\t");
		while (token !=NULL){
			printf("%s\n",token);
			token = strtok(NULL, "\t");
		}
		fputs(str, stdout);	
	}
	*/
		fclose(fp);
		return 0;
	}
}

char ** tokenize(char * str, char delim){
	char **tokens = malloc(5 * sizeof(char*));
	int lastEnd = 1;
	int newEnd = 0;

	if (str == NULL || delim == '\0')
		tokens = NULL;
	else{
		tokens = malloc(5*sizeof(char*));
		
		tokens[0] = malloc(sizeof(char));	
		strncpy(tokens[0], str, 1);
		tokens[0][1] = '\0';

		for(int i = 1; i < 5; ++i){
			tokens[i] = malloc(sizeof(char));
			if(i != 4)
				newEnd = findChar(str, '\t', lastEnd);
			else
				newEnd = findChar(str, '\r', lastEnd);
			tokens[i]=subString(str, lastEnd, newEnd);
			lastEnd = newEnd+1;
		}	
	}
	return tokens;	
}

int findChar(char *str, char delim, int startPos){
	for(int i = startPos; str[i] != '\0'; i++)
		if(str[i] == delim)
			return i;
	
	return -1;
}

char *subString(char * str, int start, int end){
	int len = end - start;
	char *ret = malloc(len+1);

	for(int i = 0; i < len && str[start+i] != '\0'; i++)
		ret[i] = str[start+i];
	ret[len] = '\0';
	return ret;
}

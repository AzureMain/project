#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// #define FILENAME "server.config"
#define MAXBUF 1024
#define DELIM "="
#define BELIM "-"

struct config
{
	char server_ip[MAXBUF];
	char port[MAXBUF];
	char chunk_size[MAXBUF];
	char image_type[MAXBUF];

};
/*
* get_config() will read the server.config file
*/
struct config get_config(char *filename)
{
	struct config configstruct;
	FILE *file = fopen (filename, "r");
	int errnum;


	if (file != NULL){
		char line[MAXBUF];
		int i = 0;

		while(fgets(line, sizeof(line), file) != NULL)
		{
			char *cfline;
			cfline = strstr((char *)line,DELIM);
			cfline = cfline + strlen(DELIM);
			char* pos;
			if((pos = strchr(cfline, '\n')) != NULL){
				*pos = '\0';
			}
			if(cfline[0] == ' '){
				cfline++;
			}
			if (i == 0){
				memcpy(configstruct.server_ip,cfline,strlen(cfline));
				// printf("%s\n",configstruct.server_ip);
			} else if (i == 1){
				memcpy(configstruct.port,cfline,strlen(cfline));
				// printf("%s\n",configstruct.port);
			} else if (i == 2){
				memcpy(configstruct.chunk_size,cfline,strlen(cfline));
				// printf("%s\n",configstruct.chunk_size);
			} else if (i == 3){
				memcpy(configstruct.image_type,cfline,strlen(cfline));
				// printf("%s\n",configstruct.image_type);
			}

			i++;
		} // End while
		fclose(file);
	} else {
		errnum = errno;
		/*          fprintf(stderr, "Value of errno: %d\n", errno);
		perror("Error printed by perror");*/
		fprintf(stderr, "Error opening config file: %s\n", strerror( errnum ));
		exit(1);
	}
	// End if file
	return configstruct;
}


struct catalog
{
	char* filelogs[MAXBUF];
	int index;
};

struct catalog get_catalog_name(char *filename)
{
	struct catalog catalogstruct;
	FILE *file = fopen (filename, "r");
	int errnum;

	if (file != NULL){
		char line[MAXBUF];
		catalogstruct.index = 0;
		while(fgets(line, sizeof(line), file) != NULL)
		{
			char *cfline;
			// printf("did I get here1 %s\n", line);
			cfline = strstr((char *)line,BELIM);
			// printf("did I get here1 %s\n", cfline);

			if(cfline==NULL) continue;
			cfline = cfline + strlen(BELIM);
			// printf("did I get here\n");
			char* pos;
			// printf("did I get here1 %s\n", cfline);


			if((pos = strchr(cfline, '\n')) != NULL){
				*pos = '\0';
			}
			//get rid of md5 code and only displace image file
			for (int j=0; j<39; j++){
				pos-=1;
				*pos='\0';
			}
			// printf("did I get here221 %s\n", cfline);
			if(cfline[0] == ' '){
				cfline++;
			}
			// printf("did I get here111 %s\n", cfline);
			// catalogstruct.filelogs[catalogstruct.index]=cfline;
			catalogstruct.filelogs[catalogstruct.index] = (char *) malloc(strlen(cfline));
			memcpy(catalogstruct.filelogs[catalogstruct.index],cfline,strlen(cfline));
			// printf("[%d]%s\n",catalogstruct.index,catalogstruct.filelogs[catalogstruct.index]);

			catalogstruct.index++;
		} // End while
		// for(int i=0; i<catalogstruct.index; i++)
		//  printf("[%d]%s\n", i, catalogstruct.filelogs[i]);
		fclose(file);
	} else {
		errnum = errno;
		/*          fprintf(stderr, "Value of errno: %d\n", errno);
		perror("Error printed by perror");*/
		fprintf(stderr, "Error opening config file: %s\n", strerror( errnum ));
		exit(1);
	}


	// End if file



	return catalogstruct;

}

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct config configstruct;

	configstruct = get_config(argv[1]);

	struct catalog catalogstruct;

	catalogstruct = get_catalog_name("catalog.csv");


	if (strlen(configstruct.image_type) == 0){
		/*
		* Code for Interactive mode
		*/
		printf("===============================\n\
		Connecting server at %s, port %s\n\
		Chunk size is %s bytes. No image type found.\n\
		===============================\n\
		Dumping contents of catalog.csv\n", configstruct.server_ip,\
		configstruct.port, configstruct.chunk_size);
		for(int i=0; i<catalogstruct.index; i++)
		printf("[%d]%s\n", i+1, catalogstruct.filelogs[i]);
		printf("===============================\n");


		int sockfd, portno, n;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		char buffer[256];
		if (argc < 2) {
			fprintf(stderr,"usage %s <filename.config>\n", argv[0]);
			exit(0);
		}
		portno = atoi(configstruct.port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		error("ERROR opening socket");
		server = gethostbyname(configstruct.server_ip);
		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(portno);
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
		while(1) {
			printf("Enter ID to download (0 to quit): ");
			bzero(buffer,256);
			//fgets(buffer,255,stdin);
			// if (buffer[0] == '0') printf("%c\n", buffer[0]);
			// if (buffer[0] == '0') exit(0);
			if (buffer[0] == '0') return 0;
			n = write(sockfd,buffer,strlen(buffer));
			if (n < 0)
			error("ERROR writing to socket");
			if (strstr(buffer, "disconnect")) {
				printf("Recieved disconnect command, closing socket\n");
				close(sockfd);
				return 0;
			}
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0)
			error("ERROR reading from socket");
			printf("%s\n",buffer);
		}
		return 0;



	} else {
		/*
		* Code for Passive mode
		*/

		printf("===============================\n\
		Connecting server at %s, port %s\n\
		Chunk size is %s bytes. Image type is %s\n\
		===============================\n\
		Dumping contents of catalog.csv\n", configstruct.server_ip,\
		configstruct.port, configstruct.chunk_size, configstruct.image_type);
		for(int i=0; i<catalogstruct.index; i++)
		printf("[%d]%s\n", i+1, catalogstruct.filelogs[i]);
		printf("===============================\n");
		//  struct catalog catalogstruct;
		//
		// 	catalogstruct = get_catalog_name("catalog.csv");


		int sockfd, portno, n;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		char buffer[256];
		if (argc < 2) {
			fprintf(stderr,"usage %s <filename.config>\n", argv[0]);
			exit(0);
		}
		portno = atoi(configstruct.port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		error("ERROR opening socket");
		server = gethostbyname(configstruct.server_ip);
		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(portno);
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");
		while(1) {
			printf("Enter ID to download (0 to quit): ");
			bzero(buffer,256);
			//fgets(buffer,255,stdin);
			// if (buffer[0] == '0') printf("%c\n", buffer[0]);
			// if (buffer[0] == '0') exit(0);
			if (buffer[0] == '0') return 0;
			n = write(sockfd,buffer,strlen(buffer));
			if (n < 0)
			error("ERROR writing to socket");
			if (strstr(buffer, "disconnect")) {
				printf("Recieved disconnect command, closing socket\n");
				close(sockfd);
				return 0;
			}
			bzero(buffer,256);
			n = read(sockfd,buffer,255);
			if (n < 0)
			error("ERROR reading from socket");
			printf("%s\n",buffer);
		}
		return 0;
	}


}

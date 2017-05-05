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
#define h_addr h_addr_list[0]

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
void get_config(struct config *cfg, char *filename){
	printf("opening %s\n",filename);
	FILE *file = fopen (filename, "r");
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
				memcpy(cfg->server_ip,cfline,strlen(cfline));
				// printf("%s\n",configstruct->server_ip);
			} else if (i == 1){
				memcpy(cfg->port,cfline,strlen(cfline));
				// printf("%s\n",configstruct->port);
			} else if (i == 2){
				memcpy(cfg->chunk_size,cfline,strlen(cfline));
				// printf("%s\n",configstruct->chunk_size);
			} else if (i == 3){
				memcpy(cfg->image_type,cfline,strlen(cfline));
				// printf("%s\n",configstruct->image_type);
			}
			i++;
		} // End while
		fclose(file);
	} else {
		fprintf(stderr, "Error opening config file: %s\n", strerror(errno));
		exit(1);
	}
	// End if file
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
		fprintf(stderr, "Error opening catalog: %s\n", strerror( errnum ));
		exit(1);
	}
	// End if file
	return catalogstruct;
}

void getfilefromcatalog(int filenumber, char filetosend[]){
	FILE *f = fopen("catalog.csv", "r");
	if ( f != NULL ){
		int i = filenumber;
		int count = 0;
		char line[512]; /* or other suitable maximum line size */
		while (fgets(line, sizeof line, f) != NULL){
			if (count == i){
				char* settonull = strchr(line, ',');

				*settonull = '\0';

				//use line or in a function return it
				strcpy(filetosend, line);

				fclose(f);

				return;
				//in case of a return first close the file with "fclose(file);"
			}
			else{
				count++;
			}
		}
		fclose(f);
	}
}

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	struct config *configstruct = malloc(sizeof(struct config));
	get_config(configstruct, argv[1]);
	// struct catalog catalogstruct;
	// catalogstruct = get_catalog_name("catalog.csv");

	if (strlen(configstruct->image_type) == 0){
		/*
		* Code for Interactive mode
		*/



		int sockfd, portno, n;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		char buffer[256];
		if (argc < 2) {
			fprintf(stderr,"usage %s <filename.config>\n", argv[0]);
			exit(0);
		}
		portno = atoi(configstruct->port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		error("ERROR opening socket");
		server = gethostbyname(configstruct->server_ip);
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

		int sockfd, portno;
		struct sockaddr_in serv_addr;
		struct hostent *server;
		if (argc < 2) {
			fprintf(stderr,"usage %s <filename.config>\n", argv[0]);
			exit(0);
		}
		portno = atoi(configstruct->port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		error("ERROR opening socket");
		server = gethostbyname(configstruct->server_ip);
		if (server == NULL) {
			fprintf(stderr,"ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(portno);
		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
			error("ERROR connecting");
		}
		int n;
		printf("Connected to server.\n");
		char buffer[256];
		bzero(buffer,256);
		n = write(sockfd,configstruct->chunk_size,strlen(configstruct->chunk_size));
		if (n < 0)
		error("ERROR writing to socket");
		int blocksize = atoi(configstruct->chunk_size);
		bzero(buffer,256);
		// Incoming file of size x
		n = read(sockfd,buffer,256);
		int incomingfilesize = atoi(buffer);
		FILE *fp;
		fp = fopen("catalog.csv", "wb");
		if(fp == NULL){
			fprintf(stderr, "Error opening file: %s\n", strerror(errno));
			return 1;
		}
		// Receive catalog
		char vsb[blocksize];
		memset(vsb, '\0', blocksize);
		int bytesRead = 0;
		int result;
		while (bytesRead < incomingfilesize)
		{
			int read_result = read(sockfd, vsb, blocksize);
			if (result < 1 )
			{ exit(1); }
			result = fwrite(vsb, 1, blocksize, fp);
			if (result < 1 )
			{ exit(1); }
			bytesRead += read_result;
		}
		printf("downloading catalog.csv @ %d bytes\n", bytesRead);
		fclose(fp);
		memset(vsb, '\0', blocksize);
		while(1){
			memset(buffer, '\0', 256);
			// Send selection to server
			char buffer[256];
			struct catalog catalogstruct;
			catalogstruct = get_catalog_name("catalog.csv");
			printf("===============================\n");
			printf("Connecting server at %s, port %s\n", configstruct->server_ip, configstruct->port);
			printf("Chunk size is %s bytes. No image type found.\n", configstruct->chunk_size);
			printf("===============================\n");
			for(int i=0; i<catalogstruct.index; i++){
				printf("[%d]%s\n", i+1, catalogstruct.filelogs[i]);
			}
			printf("===============================\n");
			int selection = -1;
			while(selection == -1 || selection > catalogstruct.index){
				if(selection > catalogstruct.index){
					printf("File must be in the range 1 to %d.\n", catalogstruct.index);
				}
				printf("Please select a file (0 will exit):");
				scanf("%s", buffer);
				selection = atoi(buffer);
				if(selection == 0){
					return 0;
				}
				n = write(sockfd, buffer, 256);
			}

			// Collect file from server
			memset(buffer, '\0', 256);
			// Incoming file of size x
			n = read(sockfd,buffer,256);
			int incomingfilesize = atoi(buffer);
			char filename[256];
			char fileanddir[264];

			getfilefromcatalog(selection, filename);
			sprintf(fileanddir, "images/%s", filename);
			printf("FNAME: %s\n",fileanddir);
			FILE *fp;
			fp = fopen(fileanddir, "wb");
			if(fp == NULL){
				fprintf(stderr, "Error opening file: %s\n", strerror(errno));
				return 1;
			}
			// Receive catalog
			char vsb[blocksize];
			int bytesRead = 0;
			int result;
			printf("IMAGE SIZE %d\n", incomingfilesize);
			while (bytesRead < incomingfilesize)
			{
				memset(vsb, '\0', blocksize);
				result = read(sockfd, vsb, blocksize);
				if (result < 1 )
				{ exit(1); }
				result = fwrite(vsb, 1, blocksize, fp);
				if (result < 1 )
				{ exit(1); }
				bytesRead += result;
			}
		}
		memset(vsb, '\0', blocksize);
		close(sockfd);
		fclose(fp);
	}
	return 0;
}

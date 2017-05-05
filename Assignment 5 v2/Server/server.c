/* A simple server in the internet domain using TCP */
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include "md5sum.h"
// #define FILENAME "server.config"
#define MAXBUF 1024
#define DELIM "="

struct config{
  char port[MAXBUF];
  char dir[MAXBUF];
};
/*
* get_config() will read the server.config file
*/
void get_config(struct config *cfg, char *filename){
  FILE *file = fopen (filename, "r");
  if (file != NULL){
    char line[MAXBUF];
    int i = 0;
    while(fgets(line, sizeof(line), file) != NULL){
      char *cfline;
      cfline = strstr((char *)line,DELIM);
      cfline = cfline + strlen(DELIM);

      if (i == 0){
        memcpy(cfg->port,cfline,strlen(cfline));
        //printf("%s",configstruct.port);
      } else if (i == 1){
        char* pos;
        if((pos = strchr(cfline, '\n')) != NULL){
          *pos = '\0';
        }
        if(cfline[0] == ' '){
          cfline++;
        }
        memcpy(cfg->dir,cfline,strlen(cfline));
        //printf("%s",configstruct.dir);
      }
      i++;
    } // End while
    fclose(file);
  } else {
    fprintf(stderr, "Error opening configuration file: %s\n", strerror(errno));
    exit(1);
  }
  // End if file
}

void error(char *msg){
  perror(msg);
  exit(1);
}

void gencata(char* dir){
  printf("Generating the catalog.\n");
  printf("DIR %s\n", dir);
  char cata[] = "catalog.csv";
  FILE *f = fopen(cata, "w");
  if(f != NULL){
    fputs("filename, size, checksum\n",f);
    DIR *pdir               = opendir(dir);
    if(pdir != NULL){
      struct dirent *entry    = readdir(pdir);
      while (entry != NULL){
        if((entry->d_type == DT_DIR) && \
        (strcmp(entry->d_name, ".") != 0) && \
        (strcmp(entry->d_name,"..") != 0)){
          // This item is a directory
        } else {
          // The item is not a directory and is a file
          char ext[16];

          strcpy(ext, strrchr(entry->d_name, '.')+1);
          if(!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "gif") || !strcmp(ext, "tiff")){
            int filesize;
            unsigned char* checksum;
            checksum = malloc(sizeof(char)*MD5_DIGEST_LENGTH);
            int fpsize = strlen(entry->d_name) + strlen(dir) + 2;
            char* fileandpath = malloc(sizeof(char)*fpsize);
            snprintf(fileandpath, fpsize,"%s/%s", dir, entry->d_name);
            md5sum(fileandpath, checksum);

            struct stat st;
            if (stat(fileandpath, &st)) {
              error(fileandpath);
            } else {
              filesize = st.st_size;
            }
            char cs[MD5_DIGEST_LENGTH * 2 + 1] = "";
            int i;
            for(i = 0; i < MD5_DIGEST_LENGTH; i++){
              char temp[3];
              sprintf(temp, "%02x", checksum[i]);
              strcat(cs, temp);
            }
            cs[MD5_DIGEST_LENGTH * 2] = '\0';
            char name[strlen(entry->d_name)+1];
            strcpy(name, entry->d_name);
            fprintf(f, "%s,%d,%s\n", entry->d_name, filesize, cs);
          }
        }
        // Advance to the next item
        entry = readdir(pdir);
      }

      closedir(pdir);
    } else {
      fprintf(stderr, "Error opening folder: %s\n", strerror(errno));
      exit(1);
    }
    fclose(f);
  } else {
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    exit(1);
  }
}

void getfilefromcatalog(int filenumber,char filetosend[]){
  FILE *f = fopen("catalog.csv", "r");
  if ( f != NULL ){
    filenumber++;
    int count = 0;
    char line[512]; /* or other suitable maximum line size */
    char filename[256];
    while (fgets(line, sizeof line, f) != NULL){
      if (count == filenumber){
        char* settonull = strchr(line, ',');
        *settonull = '\0';
        //use line or in a function return it
        strcpy(filetosend, filename);
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


int getnumfiles(char *dir){
  DIR *pdir               = opendir(dir);
  struct dirent *entry    = readdir(pdir);
  int num = 0;
  if(pdir != NULL){
    while (entry != NULL){
      char ext[16];
      strcpy(ext, strrchr(entry->d_name, '.')+1);
      if(!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "gif") || !strcmp(ext, "tiff")){
        num++;
      }
      // This item is a directory
      // Advance to the next item
      entry = readdir(pdir);
    }
    closedir(pdir);
  }
  return num;
}

int main(int argc, char *argv[]){
  struct config *configstruct = malloc(sizeof(struct config));
  get_config(configstruct, argv[1]);
  int sockfd, newsockfd, portno;
  unsigned int clilen;
  struct sockaddr_in serv_addr, cli_addr;
  gencata(configstruct->dir);
  if (argc < 2) {
    fprintf(stderr,"ERROR, no config provided\n");
    exit(1);
  }
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(configstruct->port);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
  sizeof(serv_addr)) < 0)
  error("ERROR on binding");
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  char buffer[256];
  int n;
  if (newsockfd < 0)
  error("ERROR on accept");
  bzero(buffer,256);
  n = read(newsockfd,buffer,255);
  if (n < 0) error("ERROR reading from socket");
  int blocksize = atoi(buffer);
  printf("Blocksize of %d established by client\n", blocksize);
  // Now that blocksize has been established, we need to send the catalog.
  FILE *fd = fopen("catalog.csv", "rb");
  if (fd == NULL){
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    exit(1);
  }
  struct stat file_stat;
  // Get file size
  if (fstat(fileno(fd), &file_stat) < 0){
    fprintf(stderr, "Error getting file stats: %s", strerror(errno));
    exit(1);
  }
  // Send the client the filesize
  char fsize[32];
  sprintf(fsize, "%lu", file_stat.st_size);
  n = write(newsockfd, fsize, sizeof(fsize));
  if (n < 0) error("ERROR writing to socket");

  // Send the file the catalog
  off_t offset = 0;
  int bytes_to_send = file_stat.st_size;
  int bytes_left = bytes_to_send;
  int bytes_sent;
  /* Sending file data */

  while (((bytes_sent = sendfile(newsockfd, fileno(fd), &offset, blocksize)) > 0) && (bytes_left > 0)){
    bytes_left -= bytes_sent;
    fprintf(stdout, "Server sent %d bytes from file's data, offset is now : %li and remaining data = %d\n", bytes_sent, offset, bytes_left);
  }
  
  while(1){
    n = recv(newsockfd, buffer, 255, 0);
    printf("From client: %s", buffer);
  }


  // Go again?
  close(newsockfd);
  close(sockfd);
  return 0;
}

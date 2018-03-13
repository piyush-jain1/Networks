#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <fcntl.h>

#define buff_size 1024


int main(int argc, char *argv[])
{
    struct sockaddr_in address,serv_addr;
    int sockfd = 0;
    char buffer[buff_size] = {0};
    char filename[buff_size] = {0};

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        perror("\n inet_pton error occured\n");
        return 1;
    }
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       perror("\n Error : Connect Failed \n");
       return 1;
    }

    char msg[1024];
    int port = 7743;
    sprintf(msg, "0$%d", port);
    int msg_len = strlen(msg);

    printf("msg string : %s\n",msg);
    
    if(send(sockfd,msg,msg_len,0) != msg_len){
        perror("message not sent");
        exit(-1);
    }
    printf("0 sent\n");

    // PHASE 3
    close(sockfd);

    sockfd = 0;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("\n Socket creation error \n");
        return -1;
    }

    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    printf("bncbs port : %d\n", port);
    if(sockfd == -1){
        perror("Socket Error\n");
        exit(-1);
    }

    if(bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Unable to bind!");
        exit(-1);
    }

    listen(sockfd,10);
    printf("Port : %d\n", ntohs(address.sin_port));
    
    // int clientid;
    int address_len = sizeof(address);
    
    // memset(buffer,'\0',sizeof(buffer));
    
    // int filecount = 2;
    // char* filename[filecount];
    while(1)
    {   
        
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        int clientid = accept(sockfd,  (struct sockaddr *)&client_addr , (socklen_t *)&client_addr_len);
        if(clientid < 0)
        {
            perror("Accept Error");
            exit(-1);
        }
        if(recv(clientid,filename,buff_size,0) < 0)
        {
            perror("Receive error");
            exit(-1);
        }
        printf("filename : %s\n", filename);
        int fd =open(filename,O_RDONLY);
        // File not available
        char fs[buff_size];
        if(fd==-1)
        {
            sprintf(fs, "0$");
            int fs_len = strlen(fs);
            
            if(send(clientid,fs,fs_len,0) != fs_len){
                perror("message not sent");
                exit(-1);
            }
            continue;
        }

        // Sending File
        struct stat file_stat;

        if (fstat(fd, &file_stat) < 0)
        {
                fprintf(stderr, "Error fstat --> %s", strerror(errno));

                exit(EXIT_FAILURE);
        }

        fprintf(stdout, "File Size: %ld bytes\n", file_stat.st_size);

        sprintf(fs, "1$%ld",file_stat.st_size);
        int fs_len = strlen(fs);
        printf("fs : %s\n", fs);
        printf("fs_len : %d\n", fs_len);
        // sprintf(fs, "1$%ld", file_stat.st_size);
  
        int p = send(clientid,fs,fs_len,MSG_NOSIGNAL);
        printf("p = %d\n",p );
        if(p != fs_len){
            printf("message not sent\n");
            exit(-1);
        }
        
        off_t offset = 0;
        long int sent_bytes=0,remain_data = file_stat.st_size;
        printf("bhjcbbcjk\n");
        /* Sending file data */
        while ((remain_data > 0) && ((sent_bytes = sendfile(clientid, fd, &offset, BUFSIZ)) > 0))
        {   
            remain_data -= sent_bytes;
            fprintf(stdout, "1. Server sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", sent_bytes, offset, remain_data);
        }
        printf("completed\n");
    }

    close(sockfd);
    
    return 0;
}

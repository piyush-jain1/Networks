#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#define PORT 6666

int main(int argc, char *argv[])
{
	struct sockaddr_in address;
	int sockfd = 0;
    int buff_size = 1024;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
    // printf("%s\n",argv[1] );
    // printf("%s\n",argv[2] );
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    // Connecting with relay sever

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }

    // Requesting Relay server

    char* msg = "1$";
    int msg_len = strlen(msg);
    if( send(sockfd, msg, msg_len,0)!=msg_len){
        perror("Send error");
        exit(-1);
    }

    // Response received from Relay sever

    int peer_node_count = 0;
    if (recv(sockfd, buffer, 1024, 0) < 0){
            perror("Receive Error");
            exit(-1);
    }

    char* peer_node_ip[peer_node_count];
    char* peer_node_port[peer_node_count];
    // printf("%s\n", buffer);
    char *temp = strtok(buffer, "$");
    
    if(temp != NULL)
    {
        peer_node_count = atoi(temp);
        printf("PeerNode Count : %d\n",peer_node_count);
    }
    
    temp = strtok(NULL, ":");
    struct sockaddr_in peer_node_addr[peer_node_count];
    int i = 0;
    
    while(temp != NULL)
    {   
        peer_node_addr[i].sin_family = AF_INET;
        // printf("%s\n",temp);
        if(inet_pton(AF_INET, temp, &(peer_node_addr[i].sin_addr.s_addr)) < 0) 
        {
            printf("Invalid Address\n");
            
            exit(-1);
        }
        temp = strtok(NULL, "$");    
        // printf("%s\n",temp);
        peer_node_addr[i].sin_port = htons(atoi(temp));
        temp = strtok(NULL, ":");
        i++;
    }
    // for(i=0; i< peer_node_count;i++)
    // {   
    //     char temp2[1024];
    //     printf("PeerNode IP: %s\n",inet_ntop(AF_INET, &(peer_node_addr[i].sin_addr), temp2, 1024));
    //     printf("PeerNode port: %d\n",peer_node_addr[i].sin_port);
    // }

    close(sockfd);

    // Phase 3
    
    char filename[1024];
    printf("Enter the name of the file: ");
    scanf("%s",filename);
    int flag=0;
    for(i=0;i<peer_node_count;i++){

        char temp_buffer[1024];
        printf("Peer node port: %d\n",ntohs(peer_node_addr[i].sin_port));
        printf("Peer node IP: %s\n",inet_ntop(AF_INET, &(peer_node_addr[i].sin_addr), temp_buffer, buff_size));

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        // Connecting with peer_nodes

        if( connect(sockfd, (struct sockaddr *)&(peer_node_addr[i]), sizeof(peer_node_addr[i])) < 0)
        {
           printf("\n Error : Connect Failed \n");
           return -1;
        }

        // printf("socket creation done\n");

        // serv_addr.sin_family = AF_INET;
        // inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
        // serv_addr.sin_port = htons(4400);

        // if( connect(sockfd, (struct sockaddr *)&(serv_addr), sizeof(serv_addr)) < 0)
        // {
        //    printf("\n Error : Connect Failed \n");
        //    return -1;
        // }

        // printf("connection done\n");


        // send filename to peer_node
        if( send(sockfd, filename, strlen(filename),0)!=strlen(filename))
        {
            perror("Send error");
            exit(-1);
        }

        memset(buffer,'\0',buff_size);

        if (recv(sockfd, buffer, 1024, 0) < 0)
        {
            perror("Receive Error");
            exit(-1);
        }

        printf("buffer = %s\n",buffer);

        if(buffer[0]=='0')
        {
            close(sockfd);
            continue;
        }

        flag = 1;
        // if (recv(sockfd, buffer, 1024, 0) < 0)
        // {
        //     perror("Receive Error");
        //     exit(-1);
        // }

        printf("buffer = %s\n",buffer);

        int temp_int;

        char *temp = strtok(buffer, "$");
        char* val = temp;

        printf("val  = %c\n",*val);

        if(temp != NULL){
            temp = strtok(NULL,"$");
            temp_int = atoi(temp);
        }

        long int filesize= temp_int;
        long int remain_data=filesize;

        printf("file size = %ld \n",filesize);

        FILE *received_file=fopen(filename,"w");
        int len;
        while( remain_data>0 && ( (len=recv(sockfd, buffer, 1024, 0)) > 0))
        {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
            printf("In buffer: %s \n",buffer);
            printf("Receive %d bytes and we hope : - %ld bytes \n",len, remain_data);
        }

        // printf("file transfer completed\n");

        fclose(received_file);
        close(sockfd);
    }
    if(flag==0)
    {
        printf("file not found in all peer_nodes\n");
    }
    return 0;
}
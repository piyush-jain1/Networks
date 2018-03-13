#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int toInt(char *x){
	int res = 0;

	int i = 0;
	while(x[i] != '\0'){
		res = 10*res + (x[i] - '0');
		i++;
	}

	return res;
}


int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Usuage: ./relay_server <server_port_number>\n");
		return -1;
	}

	int server_port = toInt(argv[1]);
	// printf("server port : %d \n",server_port);
	struct sockaddr_in serv_addr,client_addr;

	int sockid = socket(AF_INET, SOCK_STREAM, 0), clientid;
	int client_addr_len = sizeof(client_addr);
	int buff_size = 1024;
	char buffer[buff_size];
	char temp[buff_size];
	int peer_node_count = 0;
	int peer_node_id[buff_size];
	struct sockaddr_in peer_node_addr[buff_size];


	memset(buffer,'\0',sizeof(buffer));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(server_port);

	if(sockid == -1){
		perror("Socket Error\n");
		exit(-1);
	}

	if(bind(sockid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Unable to bind!");
		exit(-1);
	}

	listen(sockid,10);

	printf("bind done\n");

	printf("port: %d \n",ntohs(serv_addr.sin_port));

	int pni = 0;
	while(1){

		memset(buffer,'\0',sizeof(buffer));

		clientid = accept(sockid, (struct sockaddr *)&client_addr , (socklen_t *)&client_addr_len);
		if(clientid < 0){
			perror("Accept Error");
			exit(-1);
		}

		printf("Client port: %d\n",client_addr.sin_port);
		printf("Client IP: %s\n",inet_ntop(AF_INET, &(client_addr.sin_addr), temp, buff_size));


		printf("accept done \n");

		// char *msg = "Hello from server!";
		// int msg_len = strlen(msg);

		// if(send(clientid,msg,msg_len,0) != msg_len){
		// 	perror("message not sent");
		// 	exit(-1);
		// }

		if(recv(clientid,buffer,buff_size,0) < 0){
			perror("Receive error");
			exit(-1);
		}

		printf("buffer = %s\n",buffer);
		// printf("buf0 = %c\n",buffer[0]);

		// int one = (buffer[0] == '1');
		// int zero = (buffer[0] == '0');
		// printf("one = %d zero = %d\n",one,zero);

		int temp_port;

		char *temp = strtok(buffer, "$");
		char* val = temp;

		printf("val	= %c\n",*val);

		printf("temp = %s\n",temp);

		temp = strtok(NULL,"$");

		if(temp != NULL){
			printf("temp = %s\n",temp);
			temp_port = atoi(temp);
		}

		if(*val == '0'){
			// Peer node
			peer_node_count++;
			peer_node_id[pni] = clientid;
			peer_node_addr[pni].sin_family = client_addr.sin_family;
			peer_node_addr[pni].sin_port = temp_port;
			peer_node_addr[pni].sin_addr.s_addr = client_addr.sin_addr.s_addr;

			printf("Peer node port: %d\n",temp_port);
			printf("Peer node IP: %s\n",inet_ntop(AF_INET, &(client_addr.sin_addr), temp, buff_size));

			pni++;
		}
		else if(*val == '1'){
			// Peer Client
			
			printf("Peer client\n");

			// char pnodes[50];
			// sprintf(pnodes,"%d",peer_node_count);

			// if( send(clientid,pnodes,sizeof(pnodes),0) != sizeof(pnodes)){
			// 		perror("message not sent");
			// 		exit(-1);
			// }

			char msg[buff_size];
			sprintf(msg,"%d",peer_node_count);

			printf("pnc = %d\n",peer_node_count );

			for(int i=0;i<peer_node_count;i++){
				printf("i=%d\n",i);

				char temp2[buff_size];

				inet_ntop(AF_INET, &(peer_node_addr[i].sin_addr), temp2, buff_size);
				sprintf(msg+strlen(msg),"$%s:%d",temp2,peer_node_addr[i].sin_port);
			}

			printf("dfdf\n");

			if( send(clientid,msg ,strlen(msg),0) != strlen(msg) ){
				perror("message not sent");
				exit(-1);
			}
		}
		else{
			perror("Illegal Client");
			exit(-1);
		}

		// printf("%s\n",buffer);
		printf("\n");
	}
	close(sockid);
}
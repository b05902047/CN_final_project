#include <functional>
#include <iostream>
#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common_app_protocol.hpp"


#define MAX_CLIENT 1024
#define STRING_MAX 1024
#define ID_MAX 64

using namespace std;

typedef struct{
	int  fd = 0;
	string id = "\0";
	char status = 'L';//L = login, R = register, M = main, C = chat, F = file
	string dest_id = "\0";
}User_log;

int server_send(int fd, int op, int data_len, char message[]){
	op = htonl(op);	
	int tmp_data_len = htonl(data_len);
	send (fd,&op,sizeof(int),0);
	send (fd,&tmp_data_len,sizeof(int),0);
	send (fd,message,data_len,0);
	return 0;
}

int Recv_Mes(User_log client_log){
	int op, data_len;
	//client disconnected
	if (recv(client_log.fd, &op, sizeof(int),0) <= 0)
		return -1;
	if (recv(client_log.fd, &data_len, sizeof(int),0) <= 0)
		return -1;
	op = ntohl(op);
	data_len = ntohl(data_len);

	char input[STRING_MAX];
	memset(input,'\0', sizeof(input));

	if (recv(client_log.fd, input, data_len, 0) <= 0)
		return -1;
	string message = input;

	//login
	if (client_log.status == 'L'){
		if (op == APP_SIGNUP){
			client_log.status = 'R';
			server_send(client_log.fd, APP_SIGNUP, 11, "Sign up ID:");
			return 1;
		}
		else if(op == APP_LOGIN){
			//check ID
			fstream file;
			file.open(input,ios::in);//path should be added.
			if (!file){
				server_send(client_log.fd, APP_ERROR, 19, "User doesn't exist.");
				return 0;
			}
			else if (file)
				server_send(client_log.fd, APP_LOGIN, 22,"Please enter password.");
			string tmpid = input;
			//check password
			if (recv(client_log.fd, &op, sizeof(int),0) <= 0){
				file.close();
				return -1;
			}
			if (recv(client_log.fd, &data_len, sizeof(int),0) <= 0){
				file.close();
				return -1;
			}
			op = ntohl(op);
			data_len = ntohl(data_len);
			memset(input,'\0',sizeof(input));
			if (recv(client_log.fd,input,data_len,0) <= 0){
				file.close();
				return -1;
			}
			if(op == APP_LOGIN){
				char buffer [STRING_MAX];
				char password [STRING_MAX];
				file.getline(buffer,sizeof(buffer),' ');
				if (strcmp(buffer,"hash:") != 0){
					printf ("ERROR, buffer = %s\n",buffer);
					file.close();
					return -2;
				}
				file.getline(password,sizeof(password),' ');
				if (strcmp(input,password) != 0){
					server_send(client_log.fd, APP_ERROR, 35,"Wrong Password, please login again.");
					file.close();
					return 0;
				}
				if (strcmp(input,password) == 0){
					server_send(client_log.fd, APP_LOGIN, 8,"Welcome.");
					client_log.status = 'M';
					client_log.id = tmpid;
					file.close();
					return 0;
				}
			}
		}
	}
	//Login end
	//reg
	else if (client_log.status == 'R'){
		if(op == APP_SIGNUP){
			fstream file;
			file.open(input,ios::in);//path should be added
			if (file.is_open() == 1){
				server_send(client_log.fd, APP_ERROR, 17,"ID already exist.");
				file.close();
				return 0;
			}
			else if (!file.is_open()){
				char path[STRING_MAX];
				strcpy(path,input);
				file.open(path,ios::out);
				server_send(client_log.fd, APP_SIGNUP, 21,"Please enter password");
				
				if (recv(client_log.fd, &op, sizeof(int),0) <= 0){
					file.close();
					remove(path);
					return -1;
				}
				if (recv(client_log.fd, &data_len, sizeof(int),0) <= 0){
					file.close();
					remove(path);
					return -1;
				}
				op = ntohl(op);
				data_len = ntohl(data_len);
				memset(input,'\0',sizeof(input));
				if (recv(client_log.fd,input,data_len,0) <= 0){
					file.close();
					remove(path);
					return -1;
				}
				if(op == APP_SIGNUP){
					file.write("hash: ",6);
					file.write(input,strlen(input));
					server_send(client_log.fd, APP_SIGNUP, 16,"Sign up success!");
					client_log.status = 'L';
					file.close();
					return 1;
				}
			}
		}
	}
	//Reg end
	//Main
	else if (client_log.status == 'M'){
		if (op == APP_CHAT){
			fstream file;
			file.open(input,ios::in);
			if (!file.is_open())
				server_send(client_log.fd, APP_ERROR, 19,"User doesn't exist.");
			else {
				file.close();
				ifstream fin;
				string path = "../message/";
				path = path + client_log.id + "/" + message;
				fin.open(path,ifstream::in);
				fin.seekg(0,ios::end);
				long long data_len = fin.tellg();
				char history[data_len];
				fin.read(history,data_len);
				server_send(client_log.fd, APP_CHAT, data_len, history);
				client_log.status = 'C';
				client_log.dest_id = input;
				fin.close();
				return 0;
			}
		}
	}
	//Main end
	//Chat
	else if (client_log.status == 'C'){
		
	}
	//Chat end
	else {
		int tmpOp = APP_ERROR, tmpData_len = 14;
		tmpOp = htonl(tmpOp);
		tmpData_len = htonl(tmpData_len);
		send (client_log.fd,&tmpOp,sizeof(int),0);
		send (client_log.fd,&tmpData_len,sizeof(int),0);
		send (client_log.fd,"ERROR message.",14,0);
		printf ("ERROR message: %s\n",input);
		return -2;
	}
	//printf ("recv from[%s:%d]\n",inet_ntoa(client_info[i].sin_addr),ntohs(client_info[i].sin_port));
	//send(client_log[i].fd,message,strlen(message),0);
	return 0;
}

int main(int argc, char *argv[]){
	//open socket
	int server_fd = 0;
	int port = atoi(argv[1]);
	struct sockaddr_in info;
	
	server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_fd == 0){
		printf("Fail to create a socket.");
	}
	bzero(&info,sizeof(info));

	//set sock info
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = INADDR_ANY;
	info.sin_port = htons(port);
	
	//bind
	if (::bind(server_fd, (struct sockaddr *)&info, sizeof(info)) < 0){
		perror("bind failed");   
		exit(0);
	}

	if (listen(server_fd,MAX_CLIENT) < 0){
		perror("listen failed");
		exit(0);
	}
	//setup parameters that select() used
	struct timeval tv;
	fd_set rdfds;
	fd_set master;

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	FD_ZERO(&master);
	FD_SET(server_fd, &master);
	int maxfd = server_fd;

	User_log 	client_log[MAX_CLIENT];
	int	 		fd;
	int			client_num = 0;
	char 		input[STRING_MAX];
	struct		sockaddr_in client_info[MAX_CLIENT];
	int 		addrlen = sizeof(info);
	while (1)
	{
		printf ("looping\n");
		FD_ZERO(&rdfds);
		memcpy(&rdfds, &master, sizeof(rdfds));
		struct timeval tmp_tv;
		memcpy(&tmp_tv, &tv, sizeof(tmp_tv));
		select (maxfd+1,&rdfds,NULL,NULL,&tmp_tv);
		
		//new client
		if (FD_ISSET(server_fd,&rdfds)){
			printf ("Get new client!\n");
			//find a available client_num
			for (int k = 0; k < MAX_CLIENT; k++)
				if (client_log[k].fd == 0)
					client_num = k;
			if ((client_log[client_num].fd = accept(server_fd,(struct sockaddr *)&client_info[client_num], (socklen_t*)&addrlen))<0){
				perror("accepted failed");
			}
			//printf ("Accepted\n");
			getpeername(server_fd, (struct sockaddr *)&client_info[client_num], (socklen_t*)sizeof(client_info[client_num]));
			FD_SET(client_log[client_num].fd,&master);
			maxfd = max(maxfd,client_log[client_num].fd);
			//set client log
			client_log[client_num].id = "\0";
			client_log[client_num].status = 'L';
		}

		//check client
		for (int i = 0; i < MAX_CLIENT; i++){
			if (FD_ISSET(client_log[i].fd,&rdfds)){
				int check = Recv_Mes(ref(client_log[i]));
				if (check == -1){
					FD_CLR(client_log[i].fd,&master);
					close(client_log[i].fd);
					client_log[i].fd = 0;
					continue;
				}
				
			}
		}
	}

	return 0;
}

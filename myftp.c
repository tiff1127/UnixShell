/*
 *  myftp.c  - For ICT374 A2 Project 2
 *              Tiffany Wong
 *              Last modified: 10/4/2021
 *
 *  revised:	Tiffany Wong
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>


#include "command.h"


#define SV_TCP_PORT 2023

//function to print appropriate errors if read/write fails
void wr_error(int re_val){
	if(re_val == 0){
		printf("Connection ended\n");
	}
	else if(re_val == -1){
		printf("Read/Write error\n");
	}
	else if(re_val == -2){
		printf("Message overload/More space is needed!\n");
	}
	else if(re_val == -3){
		printf("File creation failed\n");
	}
	else if(re_val == -4){
		printf("Data insertion error occured\n");
	}
	else{
		printf("No such error number\n");
	}
}

//function to perform for 'get' command
void getn(int sd, char *filename){

	/*initializing empty string for use*/
	char buf[MAX_BLOCK_SIZE];
	bzero(buf,MAX_BLOCK_SIZE);

	/*client_condition set to 1 by default(indicating it is ready to receive the file)*/
	int file_overwrite, wr_condition, client_condition = 1, svr_reply;

	/* 
	 * gets response from server either :
	 * 1 : the server is ready to transfer files over
	 * -1 : the file cannot be transfer over
	 */
	if((wr_condition = readn(sd,(char *)&svr_reply,sizeof(svr_reply))) <= 0){
		wr_error(wr_condition);
		return;
	}

	//converts the number back to integer
	svr_reply = (int) ntohs(svr_reply);

	/*if the server is ready to send the files*/
	if(svr_reply == 1){

		//checks if there is file with the same name in client
		if(fileExist(filename) == 1){

			//prompts for input if the same file name is found
			printf("Same filename detected in this directory, overwrite?(yes/no)\n");
			fgets(buf,MAX_BLOCK_SIZE,stdin);

			//change to newline to null terminator
			buf[strcspn(buf,"\n")] = '\0';

			FILE *fp;

			//gets file ready if user enters yes
			if(strcmp(buf,"yes")==0){
				fp = fopen(filename,"wb");

				//client fails to create file
				if(fp == NULL){
					file_overwrite = -1;
					printf("Cannot create file to write on current directory\n");
				}

				else{
					file_overwrite = 1;
				}

			}

			//if user enters no
			else if(strcmp(buf,"no") == 0){
				file_overwrite = -1;
			}

			//invalid command entered
			else{
				file_overwrite = -1;
				printf("Invalid command\n");
			}

			/*sends number to server
			*
			*	1  : client wants to receive file
			*	-1 : client does not want to receive file
			*
			*/
			int nb_file_overwrite = htons(file_overwrite);
			if((wr_condition = writen(sd, (char *)&nb_file_overwrite,sizeof(nb_file_overwrite))) <= 0){
				wr_error(wr_condition);
				exit(1);
			}

			//receives file from server
			if(file_overwrite == 1){
				receivefile(sd, fp);
				fclose(fp);
			}

			//if client refuses to receive file, print message
			else{
				printf("File not received\n");
			}

		}
		
		//if no file of the same name found in the directory
		else{

			FILE *fp = fopen(filename,"wb");

			//if file cannot be created
			if(fp == NULL){
				client_condition = -1;
				printf("Client is unable to create file in current directory\n");
			}

			/*sends mesage to server
			*
			*	1 = client is ready to receive file
			*	-1 = client cannot receive file
			*/
			int nb_client_condition = htons(client_condition);
			writen(sd, (char *)&nb_client_condition,sizeof(nb_client_condition));

			//receives file
			if(client_condition == 1){
				receivefile(sd, fp);
			}

			fclose(fp);
		}
	}

	else{
		printf("No such file/Server is unable to send the selected file\n");
	}

	printf("Operation ended\n");

}

//function that triggers if 'put' command is set
void putn(int sd,char *filename){
	
	//initializing and clearing strings for use
	char buf[MAX_BLOCK_SIZE];
	bzero(buf,MAX_BLOCK_SIZE);

	int svr_reply, wr_condition;
	
	
	
	/* gets signal from server that the server is either :
	 *
	 * 1 : server is ready to receive file
	 * -1 : server is unable to receive file
	 * 0 : waits for another client input 
	 *
	 */
	if((wr_condition = readn(sd,(char *)&svr_reply,sizeof(svr_reply)) <= 0){
		wr_error(wr_condition);
		return;
	}

	svr_reply = ntohs(svr_reply);

	/*prints error message if server is not ready to receive file*/
	if(svr_reply == -1){
		printf("Server can't accept file %s\n",filename);
	}

	/*server is ready to receive files*/
	else if(svr_reply == 1){

		/*client send files*/
		sendfile(sd,filename);
	}

	/*Server wants verification from client, whether to overwrite the file in server*/
	else{
		int file_overwrite = 0, nb_file_overwrite;
		printf("File present in server.\nDo you want to overwrite %s ? (yes/no)\n",filename);

		/*getting input from user*/
		fgets(buf,MAX_BLOCK_SIZE,stdin);

		/*removes newline charcter generated by fgets*/
		buf[strcspn(buf,"\n")] = '\0';

		/*
		 * number assigned according to user input
		 * yes : 1
		 * no : 0
		 * other than the two above : -1
		 */
		if(strcmp(buf,"yes")==0){
			file_overwrite = 1;
		}
		else if(strcmp(buf,"no")==0){
			file_overwrite = -1;
		}
		else{
			file_overwrite = -1;
			printf("Invalid command\n");
		}

		FILE *fp = fopen(filename,"r");

		if(fp == NULL){
			file_overwrite = -1;
			printf("Can't create file in client's side");
		}

		int nb_file_overwrite = htons(file_overwrite);

		/*sends number to server*/
		/*error writing*/
		if(wr_condition = writen(sd,(char *)&nb_file_overwrite,sizeof(nb_file_overwrite))<=0){
			wr_error(wr_condition);
			return;
		}

		/*client intends to overwrite file*/
		if(file_overwrite == 1){

			int transfer;

			/*get signal from server when it is ready to receive file*/
			if((wr_condition = readn(sd,(char *)&transfer,sizeof(transfer))) > 0){
				transfer = ntohs(transfer);

				//server is ready to receive file
				if( transfer == 1 ){
					sendfile(sd,fp);

					printf("Transfer complete\n");
				}

				//server cannot receive file
				else{
					printf("Server error occured\n");
				}

			}

			//if read error
			else{
				wr_error(wr_condition);
				exit(1);
			}
		}

		/*Clients chooses not to overwrite*/
		else {
			printf("File not uploaded\n");
		}

		printf("Operation ended\n");
		return;
	}


}


int main(int argc,char*argv[]){

	char host[60];

	//if no IP address/host name is entered assume that the client is trying to connect to server that is hosting in the loaclhost
	if(argc==2){
		strcpy(host,argv[1]);
	}

	//only accepts one format
	else{
		printf("Usage : %s [IP Address/HostName]\n",argv[0]);
		exit(1);
	}

	//declare server port to default port number : 2023
	unsigned short port = SV_TCP_PORT;
	
	int sd, wr;
	struct sockaddr_in server_addr;
	struct hostent  *host_details;
	char buf[MAX_BLOCK_SIZE];


	//erases all data starting from pointer at ser_addr by sizeof(ser_addr)
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	//if host name is not recognized
	if((host_details = gethostbyname(host))==NULL){
		printf("host %s not found\n",host);
		exit(1);
	}

	//h_addr : first element in h_addr_list which is a char pointer to a char array
	//derefrence the char array and binds to a pointer to u_long
	server_addr.sin_addr.s_addr = *(u_long *) host_details -> h_addr;

	//TCP socket
	sd = socket(PF_INET, SOCK_STREAM, 0);

	//fd of new socket's file descriptor will be returned, otherwise returns -1
	if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		perror("Client connect");
		exit(1);
	}

	//infinite loop
	while(1){

		bzero(buf,MAX_BLOCK_SIZE);

		//prints '>' and wait for input from user
		fputs(">",stdout);
		
		//gets input from user
		fgets(buf,MAX_BLOCK_SIZE,stdin);
		buf[strcspn(buf, "\n")] = '\0';
	
		int controller;

		//prevents to alter the original string from user input
		char cmd_line[MAX_BLOCK_SIZE];
		strcpy(cmd_line,buf);
		char* cmd = strtok(cmd_line," ");

		//different processes for different command

		//command "pwd"
		if(strcmp(cmd,"pwd") == 0){

			//writes command to server
			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}

			//clears 'buf'
			bzero(buf,MAX_BLOCK_SIZE);

			int pwd_process;

			/*reads from server :
			 * 1 : the current working directory is obtained successfully
			 * -1 : the current working directory cannot be obtained
			 */
			if((controller = readn(sd,(char *)&pwd_process,sizeof(pwd_process))) <= 0){
				wr_error(controller);
				break;
			}

			pwd_process = ntohs(pwd_process);
			
			//if server can get it's directory
			if(pwd_process == 1){

				bzero(buf,MAX_BLOCK_SIZE);

				//gets directory string from server
				if((controller = readn(sd,buf,MAX_BLOCK_SIZE)) <= 0){
					wr_error(controller);
					break;
				}

				//prints current directory of server
				else{
					printf("%s\n",buf);
				}

			}

			//server cannot obtain it's directory
			else{
				printf("Fail to obtain current directory of server\n");
			}


		}

		//command "lpwd"
		else if(strcmp(cmd,"lpwd") == 0){

			//clears input
			bzero(buf,MAX_BLOCK_SIZE);

			//can't obtain current directory
			if(pwdn(buf) == -1){
				printf("Failed to obtain current working directory\n");
			}

			//prints current directory if obtained
			else{
				printf("%s\n", buf);
			}
		}

		//command "dir"
		else if(strcmp(cmd,"dir") == 0){

			int svr_response;

			//writes command to server
			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}

			//
			if((controller = readn(sd,(char *)&svr_response,sizeof(svr_response))) <= 0){
				wr_error(controller);
				break;
			}

			svr_response = ntohs(svr_response);

			bzero(buf,MAX_BLOCK_SIZE);

			if((svr_response == 1){
				if((controller = readn(sd,buf,MAX_BLOCK_SIZE)) <= 0){
					wr_error(controller);
					break;
				}

				printf("%s\n", buf);
			}

			else if(svr_response == -1){
				printf("Cannot retrieve file names in server's current directory\n");
			}

			else{
				printf("Buffer too small for input\n");
			}

		}

		//command "ldir"
		else if(strcmp(cmd,"ldir") == 0){
			bzero(buf,MAX_BLOCK_SIZE);
			int dir_control;

			if((dir_control = dirn(buf,MAX_BLOCK_SIZE)) < 0){
				if(dir_control == -1){
					printf("Failed to obtain files in current directory\n");	
				}
				else{
					printf("Buffer too small\n");
				}
			}

			else{
				printf("%s\n", buf);
			}
		}

		//command "cd"
		else if(strcmp(cmd,"cd") == 0){

			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}

			bzero(buf,MAX_BLOCK_SIZE);

			int directory_change;

			if((controller = readn(sd,(char *)&directory_change,sizeof(directory_change))) <= 0){
				wr_error(controller);
				printf("Can't change server's current directory\n");
			}

			directory_change = ntohs(directory_change);

			else{
				if(directory_change == 1){
					printf("Server directory changed\n");
				}
				else{
					printf("Can't change server's current directory\n");
				}
			}

		}

		//command "lcd"
		else if(strcmp(cmd,"lcd") == 0){
			char * directory = strtok(NULL," ");

			bzero(buf,MAX_BLOCK_SIZE);

			if(cdn(directory) < 0){
				printf("Failed to change client current directory\n");
			}
			else{
				printf("Client dierectory changed\n");
			}

		}

		else if(strcmp(cmd,"get") == 0){
			//get file name
			char *filename;
			filename = strtok(NULL," ");

			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}
		
			getn(sd,filename);
			
		}

		else if(strcmp(cmd,"put") == 0){
			char *filename;
			filename = strtok(NULL," ");
					
			//sends command to server
			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}

			putn(sd,filename);
		}


		//quits the program if the input is quit
		else if(strcmp(cmd,"quit") == 0){
			if((controller = writen(sd,buf,strlen(buf))) <= 0){
				wr_error(controller);
				break;
			}

			printf("quit\n");

			exit(0);
		}

		else{
			printf("Invalid command\n");
		}

	}
}

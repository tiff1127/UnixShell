/*
 *  myftpd.c  - For ICT374 A2 Project 2
 *              Lee Yong Jian
 *              Last modified: 10/4/2021
 *
 *  revised:	Tiffany Wong
 */


#include  <unistd.h>
#include  <sys/stat.h>
#include  <stdlib.h>     
#include  <stdio.h>      
#include  <string.h>     
#include  <errno.h>      
#include  <signal.h>     
#include  <syslog.h>
#include  <sys/types.h>  
#include  <sys/socket.h> 
#include  <sys/wait.h>   
#include  <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h>


#include "command.h"


#define  SV_TCP_PORT   2023   /* default server listening port : 2023*/

void claim_children()
{
     pid_t pid=1;
     
     while (pid>0) { 
         pid = waitpid(0, (int *)0, WNOHANG); 
     } 
}


void daemon_init(void)
{       
     pid_t   pid;
     struct sigaction act;

     if ( (pid = fork()) < 0) {
          perror("fork"); exit(1); 
     } else if (pid > 0) {
          printf("Server PID: %d\n", pid);
          exit(0);
     }

   
     setsid();
     umask(0);

     /* catch SIGCHLD to remove zombies from system */
     act.sa_handler = claim_children; 
     sigemptyset(&act.sa_mask);       
     act.sa_flags   = SA_NOCLDSTOP;   
     sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
     
}

//checking for type of command
int checkwadcommand(const char *str) { 
	
    if (str[0] == 'p' && str[1] == 'w' && str[2] == 'd' && strlen(str)== 3) 
    return 0; // pwd = 0
    
    else if (str[0] == 'd' && str[1] == 'i' && str[2] == 'r' && strlen(str)== 3)
    return 1; // dir = 1
    
    else if(str[0] == 'c' && str[1] =='d')
    return 2; // cd = 2
    
    else if (str[0] == 'g' && str[1] =='e' && str[2] == 't')
    return 3; // get = 3
    
    else if (str[0] == 'p' && str[1] =='u' && str[2] == 't')
    return 4; // put = 4
    
    else if (strcmp(str,"quit")==0)
    return 5;
    
    else 
    return -1; // other inputs
}


void getn(int sd,char *filename)
{
	int pass = 1;
	FILE *getfile = fopen(filename,"r");// tries to read the filename given by the client.

	if(getfile != NULL) // Successfully read the file .
	{
		int ctn;

		pass = htons(pass);
		writen(sd,(char *) &pass, sizeof(pass));

		readn(sd,(char*) &ctn,sizeof(ctn));
		ctn = ntohs(ctn);
		
		if(ctn==1)
		{
			sendfile(sd,getfile);
		}
		else
			return;
	}

	else 
	{
		pass = -1;
		pass = htons(pass);
		writen(sd,(char *) &pass, sizeof(pass)); // File can't be read send -1 code to client
	}
		
		

}

void putn(int sd,char *filename)
{
    
    int pass = 1;
    int found = fileExist(filename);
    if(found == 0)
    {
        FILE* putfile  = fopen(filename,"wb");

        if(putfile != NULL)
        {

            pass = htons(pass);
            writen(sd,(char *) &pass, sizeof(pass));
            
            receivefile(sd,putfile);
        }

        else 
        {
            pass = -1;
            pass = htons(pass);

            writen(sd,(char *) &pass, sizeof(pass)); // File not create send -1 code to client
        }   
        
    }
    else 
    {

        int ctn;

        pass = 0;
        pass = htons(pass);
        

        writen(sd,(char *) &pass, sizeof(pass)); // existing file found.

        readn(sd,(char*) &ctn,sizeof(ctn)); // read if client wants to overwrite.
        ctn = ntohs(ctn);
        
        if(ctn==1)
        {
            FILE* putfile  = fopen(filename,"wb");
            if(putfile != NULL)
            {
                pass = 1;
                pass = htons(pass);

                writen(sd,(char *) &pass, sizeof(pass));

                receivefile(sd,putfile);
            }
            else 
            {
                pass = -1;
                pass = htons(pass);

                writen(sd,(char *) &pass, sizeof(pass)); // File not create send -1 code to client
            }   
            
        }
        else 
            return;
    }

}

void serve_a_client(int sd)
{   
    int nr, nw,rtrn,count;
    char buf[MAX_BLOCK_SIZE];
    char newbuf[MAX_BLOCK_SIZE];

    while (1){
        
         if ((nr = readn(sd, buf, sizeof(buf))) <= 0) 
             return;   

         
         if (checkwadcommand(buf) == 0) // if command = pwd
         {
            bzero(buf,MAX_BLOCK_SIZE);

         	rtrn = pwdn(buf);

         	if(rtrn == 1)
         	{
                rtrn = htons(rtrn);
         		writen(sd,(char *)&rtrn, sizeof(rtrn));
                writen(sd,buf,strlen(buf));
         	}

         	else
            {
                rtrn = htons(rtrn);
         		writen(sd,(char *)&rtrn,sizeof(rtrn));
            }
         	
         }

         else if (checkwadcommand(buf) == 1) //  if command = dir
         {
         	rtrn = dirn(buf, sizeof(buf));

         	if(rtrn == 1)
         	{
                rtrn = htons(rtrn);

         		writen(sd,(char *)&rtrn, sizeof(rtrn));
                writen(sd,buf,strlen(buf));
         	}

         	else
            {
         		rtrn = htons(rtrn);
         		writen(sd,(char *)&rtrn,sizeof(rtrn));
            }
         }

         else if (checkwadcommand(buf) == 2) //  if command = cd
         {
         	memcpy(newbuf,&buf[3],strlen(buf)-3);
         	rtrn = cdn(newbuf);

         	if(rtrn == 1)
         	{
                rtrn = htons(rtrn);
         		writen(sd,(char *) &rtrn,sizeof(rtrn));
         	}

         	else
            {
         		rtrn = htons(rtrn);
         		writen(sd,(char *) &rtrn,sizeof(rtrn));
            }
         }

         else if (checkwadcommand(buf) == 3) // if command = get
         {

            //gets 'get' but we do not need it
         	strtok(buf," ");
         		
         	char *fname = strtok(NULL," ");
         	
         	getn(sd,fname);
         	
         }

         else if (checkwadcommand(buf) == 4 )// if command = put
         {
         	
         	strtok(buf," ");
         	
         	char * fname = strtok(NULL," ");

         	putn(sd,fname);
         	
         	
         }

         else// if command = quit
         {
         	break;
         }
         
    } 
}

int main(int argc, char *argv[])
{
     int sd, nsd, n;  
     pid_t pid;
     unsigned short port;   // server listening port
     socklen_t cli_addrlen;  
     struct sockaddr_in ser_addr, cli_addr; 
     
     if (argc == 2) {
     	if(chdir(argv[1]) == -1){
     		printf("The directory %s does not exist\n", argv[1]);
     		exit(1);
     	}
     }
     else {
          printf("Usage: %s [ initial current directory ]\n", argv[0]);     
          exit(1);
     }

     /* turn the program into a daemon */
     daemon_init(); 

     /* set up listening socket sd */
     if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
           perror("server:socket"); exit(1);
     } 

     /* build server Internet socket address */
     bzero((char *)&ser_addr, sizeof(ser_addr));
     ser_addr.sin_family = AF_INET;
     ser_addr.sin_port = htons(port);
     ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
     /* note: accept client request sent to any one of the
        network interface(s) on this host. 
     */

     /* bind server address to socket sd */
     if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
           perror("server bind"); exit(1);
     }

     /* become a listening socket */
     listen(sd, 5);

     while (1) {

          /* wait to accept a client request for connection */
          cli_addrlen = sizeof(cli_addr);
          nsd = accept(sd, (struct sockaddr *) &cli_addr, (socklen_t *)&cli_addrlen); // nsd = new socket descriptor
          if (nsd < 0) {
               if (errno == EINTR)   /* if interrupted by SIGCHLD */
                    continue;
               perror("server:accept"); exit(1);
          }

          /* create a child process to handle this client */
          if ((pid=fork()) <0) {
              perror("fork"); exit(1);
          } else if (pid > 0) { 
              close(nsd);
              continue; /* parent to wait for next client */
          }

          close(sd); 
          serve_a_client(nsd);
          exit(0);
     }
}


#define MAX_BLOCK_SIZE (1024*5)

/*
*
*	-1 = read error
*	-2 = buffer too small
*	>0 = bytes read
*	0 = no bytes read(connection ended)
* 	
*/

int readn(int fd, char *buf, int bufsize);

/*
*	
*	-2 = buffer too large to write into socket
*	-1 = write error
*	0 = connection ended
*	>0 = bytes written
*
*/
int writen(int fd, char *buf, int nbytes);

/*
*
*	-1 = failed to get current directory
*	1 = get succesfully(the outcome string is at output)
*
*/
int pwdn(char *output);

/*
*
*	-1 = cannot open current directory
*	-2 = the resulting data names from dirn is too large
* 	1 = get successfully
*
*
*/
int dirn(char *output, int outsize);

/*
*	-1 = unable to change to the stated directory
*	1 = the directory changed successfully
*
*
*/
int cdn(char *newdir);

/*	
*	0 = socket connection ended
*	1 = operation performed succesfully
*	-1 = write fail
*	-2 = message overload
*	-3 = unable to open file
*	-4 = error reading from file
*	
*
*/
int sendfile(int sd,char *filename);

/*
*	0  = connection closed
*	1  = operation performed succesfully
*	-1 = read() error
*	-2 = buffer too small for read()
*	-3 = file cannot be created
*	-4 = bytes lost
*	
*
*/
int receivefile(int sd,char *filename);

/*
*	0 = file does not exist
*	1 = file exist
*
*
*/
int fileExist(char *filename);
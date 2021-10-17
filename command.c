#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#include "command.h"


int readn(int fd, char *buf, int bufsize){

    int data_size;
    int n, nr, len;

    //buffer too small
    if(bufsize < MAX_BLOCK_SIZE){
        return -2;
    }

    //returns read error if fails
    if( nr = read(fd, (char *)&data_size, sizeof(data_size)) <= 0 ){
        //-1 if read error/0 if connection ended
        return nr;
    }

    len = (int) ntohs(data_size);

    for(n = 0; n < len; n += nr){

        //read failed/bytes lost
        if(( nr = read(fd, buf+n, len-n)) <= 0 ){
            return nr;
        }

    }

    return len;
}


int writen(int fd, char *buf, int nbytes){
    int data_size = nbytes;
    int n, nw;

    //write value too large
    if(nbytes > MAX_BLOCK_SIZE){
        return -2;
    }

    data_size = htons(data_size);


    if((nw = write(fd, (char *)&data_size, sizeof(data_size))) <= 0 ){
        return nw;
    }

    for(n=0; n<nbytes; n += nw){
        if((nw = write(fd, buf+n, nbytes-n)) <= 0 ){
            return nw;
        }
    }
    return n;

}


int pwdn(char *output){
    char *temp;
    if((temp = getcwd(NULL,0)) == NULL){
        return -1;
    }
    
    strcpy(output,temp);
    free(temp);
    return 1;
}


int dirn(char *output, int outsize){
    int i = 0;
    DIR *d;
    struct dirent *dir;

    d = opendir(".");

    if(d == NULL){
        return -1;
    }

    else{
        while((dir = readdir(d)) != NULL ){
            char * temp = dir->d_name;
            if(strcmp(".",temp)!=0&&strcmp("..",temp)!=0){
                //changing the null terminator to new line character
                if(temp[strlen(temp)] == '\0'){
                    temp[strlen(temp)] = '\n';
                }

                i += strlen(temp);
               strcat(output,temp);

               if(i > outsize){
                   return -2;
               }
            }

            
        }
    }

    closedir(d);
    return 1;
}


int cdn(char *newdir){
    if(chdir(newdir) != 0){
        return -1;
    }

    return 1;
}

int sendfile(int sd,FILE *fp){
    char buf[MAX_BLOCK_SIZE];
    bzero(buf, MAX_BLOCK_SIZE);


    int w_condition, sz, read_bytes;

    //gets size of file in bytes
    if(fseek(fp,0,SEEK_END) == 1){
        return -3;
    }

    sz = (int) ftell(fp);

    if(sz == -1){
        return -3;
    }


    if((w_condition = writen(sd,(char *)&sz, sizeof(sz))) <= 0){
        return -1;
    }

    //while not end of file
    while(!feof(fp)){
        //reading characters up to MAX_BLOCK_SIZE
        read_bytes = fread(buf,sizeof(char),MAX_BLOCK_SIZE,fp);
        
        if(ferror(fp)){
            return -3;
        }
        
        //sends all the characters over
        if((w_condition = writen(sd, buf, read_bytes)) == -1){
            return -1;
        }
        else if(w_condition == 0){
            return w_condition;
        }
        else if(w_condition == -2){
            return w_condition;
        }
    }

    fclose(fp);

    return 1;
}

int receivefile(int sd,FILE *fp){
    char buf[MAX_BLOCK_SIZE] = {0};
    bzero(buf,MAX_BLOCK_SIZE);

    int bytes_read, r_condition, t_bytes = 0;
    long fsize;

    if((r_condition = readn(sd,(char *)fsize,sizeof(fsize))) <= 0){
        return -1;
    }

    while(t_bytes <= fsize){
        if((bytes_read = readn(sd, buf, MAX_BLOCK_SIZE)) <= 0){
            break;
        }

        int write_control = fwrite(buf, sizeof(char), bytes_read, fp);

        //write fail/not enough bytes is written
        if(write_control<bytes_read){
            return -3;
        }

        bzero(buf,MAX_BLOCK_SIZE);

    }

    if(bytes_read == -1){
        if(errno != EAGAIN){
            return bytes_read;
        }
    }

    else if(bytes_read == -2){
        return bytes_read;
    }

    else if(bytes_read == 0){
        return bytes_read;
    }

    else{   }

    fclose(fp);
    return 1;
}

int fileExist(char *filename){
    int found = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    while((dir = readdir(d)) != NULL){
        if(strcmp(dir->d_name,filename) == 0){
            found = 1;
            break;
        }
    }
    closedir(d);
    return found;
}

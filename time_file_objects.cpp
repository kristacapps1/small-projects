//c++ copy files
//
//
//
//
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "buffer.h"
//#include "msgqueuelog.h"
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string>
#include <climits>
#include <algorithm>

#define PERMS (S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)

typedef struct mymsg_t{
    long mtype;
    char mtext[4096];
}mymsg_t;

//////////////////////////////////////////
//Time performance of unnamed pipe
static int timepipe(int *fd,char *buff, char *buff1, int fs){
    int pid;
    long tottime;
    long avgtime=0;
    struct timeval tpstart,tpend;
    gettimeofday(&tpstart, NULL);
    pipe(fd);
    if((pid=fork())<0){
        printf("fork error");
    }
    if(pid>0){              //this is the parent
        write(fd[1],buff,fs);
    }
    else{                   //this is child
        read(fd[0],buff1,fs);
        exit(EXIT_SUCCESS);
    }
    wait(&pid);             //parent wait for child to join
    gettimeofday(&tpend, NULL);
    tottime=1000000L*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec);
    printf("copytime for unnamed pipe: %dmusec\n",tottime);
    fs=pid;
    return fs;
}

//////////////////////////////////////////
//Time performance of named pipe(fifo)
static int timefifo(int *fd,char *buff, char *buff1, int fs){
    int pid;
    int tmpfd;
    int tmpfd2;
    long tottime;
    long avgtime=0;
    struct timeval tpstart,tpend;
    gettimeofday(&tpstart, NULL);
    mkfifo("myfifo",S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if((pid=fork())<0){
        printf("fork error");
    }
    if(pid>0){              //this is the parent
        tmpfd=open("myfifo", O_WRONLY);
        write(tmpfd,buff,fs);
        close(tmpfd);
    }
    else{                   //this is the child
        tmpfd2=open("myfifo", O_RDWR);
        read(tmpfd2,buff1,fs);
        close(tmpfd2);
        exit(EXIT_SUCCESS);
    }
    wait(&pid);             //parent wait for child to join
    gettimeofday(&tpend, NULL);
    tottime=1000000L*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec);
    printf("copytime for fifo: %dmusec\n",tottime);
    fs=pid;
    return fs;
}

//////////////////////////////////////////
//Time performance of message queue
static int timemsgq(int *fd,char *buff, char *buff1, int fs){
    int pid;
    int br;
    int msgqid;
    int error=0;
    long tottime;
    long avgtime=0;
    struct timeval tpstart,tpend;
    gettimeofday(&tpstart, NULL);
    mymsg_t msg,msgrc;
    if(((msgqid)=msgget(IPC_PRIVATE,PERMS))==-1)
        perror("msgget failed");
    if((pid=fork())<0){
        printf("fork error");
    }
    if(pid>0){                  //this is parent
        strcpy(msg.mtext,buff);
        msg.mtype=1;
        if(msgsnd(msgqid,&msg,sizeof(msg.mtext),0)==-1){
            error=errno;
            perror("msgsnd failed");
        }
    }
    else{                       //this is child
        msgrc.mtype=1;
        msgrcv(msgqid,&msgrc,sizeof(msg.mtext),1,0);
        if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
            perror("Message queue could not be deleted.\n");
        }
        exit(EXIT_SUCCESS);
    }
    wait(&pid);                 //wait for child to join
    gettimeofday(&tpend, NULL);
    tottime=1000000L*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec);
    printf("copytime for message queue: %dmusec\n",tottime);
    fs=pid;
    return fs;
}

int main(int argc, char **argv){
    int fc;
    int tempi = 0;
    int err;
    int temp;
    int fd1;
    ssize_t br;
    
    //get data to transmit from file
    if(!(fd1 = open(argv[1],O_RDONLY))){
        printf("file1 couldn't be opened\n");
    }
    off_t fs = lseek(fd1,0,SEEK_END);
    char *buff=(char*)malloc(sizeof(char)*fs);
    char *buff1=(char*)malloc(sizeof(char)*fs);
    off_t fs2 = lseek(fd1,0,SEEK_SET);
    int fd[2];
    br=read(fd1,buff,fs);
    
    //timer
    long tottime;
    long avgtime=0;
    struct timeval tpstart,tpend;
    
    //for loop for unnamed pipe
    for(int i=0;i<30;i++){
       temp=timepipe(&fd1,buff,buff1,fs);
    }
    printf("\n\n");
    free(buff1);
    
    //for loop for fifo
    buff1=(char*)malloc(sizeof(char)*fs);
    for(int i=0;i<30;i++){
       temp=timefifo(&fd1,buff,buff1,fs);
    }
    printf("\n\n");
    free(buff1);
    
    //for loop for message queue
    buff1=(char*)malloc(sizeof(char)*fs);
    for(int i=0;i<30;i++){
       temp=timemsgq(&fd1,buff,buff1,fs);
    }
    printf("\n\n");
    return 0;
}
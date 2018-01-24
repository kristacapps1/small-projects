//getweb multithreaded
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <resolv.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string>
#include <climits>
#include <algorithm>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include<stdio.h>
#include<stdlib.h>

static pthread_rwlock_t rdlock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t wrlock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t  bufferlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  stdoutlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  totallock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  socklock = PTHREAD_MUTEX_INITIALIZER;
static int totbytes;

static void *copy_file(void *ptr);

struct thdata{
    char *thdl;
    char *hostname;
    int thrdfp;
    int thrdfd2;
    int clen;
    int tid;
    pthread_rwlock_t rdlock;
    pthread_rwlock_t wrlock;
};

int main(int argc, char*argv[])
{
    /*File descriptors*/
    int fd;
    int fd2;
    int thrd;
    int thrd_cnt=20;
    int i=0;
    int temp;
    int err;
    pid_t childpid = 0;
    
    /*for http requests and socket*/
    int char1;
    int sockfd;
    int port=80;
    char* host =argv[1];
    char* filename=(char*) malloc(256);
    char* clptr;
    struct hostent *host_server;
    struct sockaddr_in serv_addr;
    int tot, bytes;
    int begins, ends;
    char headerbuff[100000];
    char clbuff[1000];
    char req[1024];
    char ip[100];
    struct in_addr **al;
    int flag=0;
    int j;
    int contentlength;
    size_t strfind;

    /*//////////////////////////////////////////////////////////////////////////////////////////
        partitioning url
    //////////////////////////////////////////////////////////////////////////////////////////*/
        
    //get past initial http:// in format
    for(int im=0;host[im]!='\0';im++){
        if(host[im]=='/'){
            if(host[im+1]=='/'){
                flag=im+2;
                break;
            }
        }
    }
    
    //hostname
    for(j=flag;host[j]!='\0';j++){
        if(host[j]=='/'){
            break;
        }
    }
    
    char* hostname= (char*) malloc((j-flag)+1);
    char* dl= (char*) malloc((strlen(host)-j)+1);
    
    int m=0;
    int x;
    
    //hostname
    for(x=flag;x<j;x++){
        if(host[x]=='/'){
            break;
        }
        hostname[m]=host[x];
        m++;
    }
    m=0;
    
    //download url
    int r;
    for(r=x;host[r]!='\0';r++){
        dl[m]=host[r];
        m++;
    }
    
    //filename
    m=0;
    int g;
    //printf("r:%d\n",r);
    for(g=r;g>flag;g--){
        if(host[g]=='/'){
            break;
        }
    }
    
    for(int fln=g+1;fln<r;fln++){
        //printf("fnchar: %c\n",host[fln]);
        filename[m]=host[fln];
        m++;
    }
    
    /*///////////////////////////////////////////////////////////////////////////////////////////
        opening socket connection
    ///////////////////////////////////////////////////////////////////////////////////////////*/
    
    hostname[j-flag]='\0';
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
        perror("Failed to create socket");
    
    host_server=gethostbyname(hostname);
    if (host_server == NULL) perror("ERROR, no such host");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    al=(struct in_addr**)host_server->h_addr_list;
    strcpy(ip , inet_ntoa(*al[0]));
    bcopy((char *)host_server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          host_server->h_length);
    
    //printf ("Name: %s\n", host_server->h_name);
    //printf ("IP: %s\n", ip);
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(80);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        perror("ERROR with connection");
    
    /*//////////////////////////////////////////////////////////////////////////////////////////
        Get header
    //////////////////////////////////////////////////////////////////////////////////////////*/
    
     sprintf(req, "HEAD %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", dl, hostname);
     if(send(sockfd,req,strlen(req),0)<0)
     perror("ERROR request failed");
    
     bzero(headerbuff,sizeof(headerbuff));
     bytes=recv(sockfd,headerbuff,sizeof(headerbuff)-1,0);
     clptr=strstr(headerbuff,"Content-Length: ");
     clptr+=16;
     m=0;
     for(char* htmp=clptr;*htmp!='\n';htmp++){
     clbuff[m]=*htmp;
     m++;
     }
     contentlength=atoi(clbuff);
    
    /*////////////////////////////////////////////////////////////////////////////
        Opening and partitioning file and Thread handling
    ////////////////////////////////////////////////////////////////////////////*/
    
    char *fn2 = filename;
    fd2 = open(fn2, O_WRONLY | O_CREAT ,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    temp = contentlength/thrd_cnt;
    int leftover = contentlength%thrd_cnt;
    begins=0;
    ends=temp;
    
    /*thread initializations*/
    pthread_t threads[thrd_cnt];
    thdata data1[thrd_cnt];
        for(int th1=0;th1<thrd_cnt;th1++){
            data1[th1].rdlock=rdlock;
            data1[th1].wrlock=wrlock;
            data1[th1].thrdfp=thrd;
            data1[th1].thdl=dl;
            data1[th1].hostname=hostname;
            data1[th1].thrdfd2=fd2;
            data1[th1].tid=th1;
            data1[th1].clen=contentlength;
            err = pthread_create(&threads[th1], NULL, copy_file,(void *) &data1[th1]);
            if(err){
                 printf("Error creating thread %d\n",th1);
            }
        }
    /*retrieve threads*/
    for(int thct=0;thct<thrd_cnt;thct++){
        pthread_join(threads[thct], NULL);
    }
    return 0;
}


static void *copy_file(void *ptr){
    thdata *data;
    data = (thdata *) ptr;

    /*lock file for read*/
    ssize_t br;
    int sockfd;
    int bytes,rdbytes;
    int flag2=0;
    int m;
    int tmpbytes;
    int tmpstart;
    int tmpend;
    int error;
    char req[1024];
    char ip[100];
    char buff[5120];
    char bufftmp[1024];
    char buff2[2048];

    struct hostent *host_server;
    struct sockaddr_in serv_addr;
    struct in_addr **al;
    
    /*reuse thread from thread pool*/
    if ((error = pthread_mutex_lock(&bufferlock))!=0){
        printf("returning error\n");
        //return error;
    }
    
    /*read data until we have specified data segment size determined by number of threads*/
    while(totbytes<data->clen){
        /*debug statements to track thread lock operations*/
        //printf("locking lock1 for thread %d\n",data->tid);
        //printf("THREAD ID:%d\n\n",data->tid);
        tmpbytes=totbytes;
        if(tmpbytes==0){
            totbytes+=5119;
        }
        else{
            totbytes+=5120;
        }
        if ((error = pthread_mutex_unlock(&bufferlock))!=0){
            printf("returning error\n");
            //return error;
        }
        /*debug statements to track thread lock operations*/
        //printf("unlocking lock1 for thread %d\n",data->tid);
        //printf("\nTHREAD ID:%d\n\n",data->tid);

    /*create socket and retrieve server information*/
        if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
            perror("Failed to create socket\n");
        host_server=gethostbyname(data->hostname);
        if (host_server == NULL) perror("ERROR, no such host\n");
    
        bzero((char *) &serv_addr, sizeof(serv_addr));
        al=(struct in_addr**)host_server->h_addr_list;
        strcpy(ip , inet_ntoa(*al[0]));
        bcopy((char *)host_server->h_addr,
             (char *)&serv_addr.sin_addr.s_addr,
             host_server->h_length);
        
    /*open port and connect*/
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_port=htons(80);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            perror("ERROR with connection");
    
    /*download data*/
        if(tmpbytes==0){
            tmpstart=tmpbytes;
            tmpend=tmpbytes+5119;
        }
        else{
            tmpstart=tmpbytes+1;
            tmpend=tmpbytes+5120;
        }

        sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nRange: bytes=%d-%d\r\n\r\n", data->thdl, data->hostname,tmpstart,tmpend);
        if(send(sockfd,req,strlen(req),0)<0){perror("ERROR request failed");}
        bzero(bufftmp,sizeof(bufftmp));
        char* clptr;
        rdbytes=0;
        flag2=0;
    /*recieve data from socket into buffer*/
    while((bytes=recv(sockfd,bufftmp,sizeof(bufftmp),0))>0){        
        usleep(10000);
        if(flag2==0){    //get rid of header for first rcv
            int findcl;
            for(findcl=0;bufftmp[findcl]!='\0';findcl++){
                if(bufftmp[findcl]=='\r'){
                    if((bufftmp[findcl+1]=='\n')&&(bufftmp[findcl+2]=='\r')&&(bufftmp[findcl+3]=='\n')){
                        flag2 = 1;
                        findcl+=4;
                        break;
                    }
                }
            }
            m=0;
            int rdtmp;
            int tmp=findcl;
            for(rdtmp=0;tmp<sizeof(bufftmp);rdtmp++){
                buff[rdtmp]=bufftmp[tmp];
                tmp++;
                m++;
            }
            rdbytes+=rdtmp;
        }
        else{
            int elem=0;
            for(int rdtmp=rdbytes;rdtmp<=(rdbytes+bytes);rdtmp++){
                buff[rdtmp]=bufftmp[elem];
                elem++;
            }
            rdbytes+=bytes;
            bzero(bufftmp,sizeof(bufftmp));
        }
    }

    /* lock for file write operation */
    if ((error = pthread_mutex_lock(&totallock))!=0){
        printf("returning error\n");
    }
        if(lseek(data->thrdfd2,tmpstart,SEEK_SET)<0){
            perror("error in lseek");
        }
        if((br = write(data->thrdfd2,buff,rdbytes))<=0){
            printf("[%d]error writing to file, or nothing written",data->tid);
        }
        
    if ((error = pthread_mutex_unlock(&totallock))!=0){
        printf("returning error\n");
    }
    close(sockfd);
    /* unlock file after write finishes */
    memset(buff2,0,sizeof(buff2));
    memset(buff,0,sizeof(buff));
    bzero(buff,sizeof(buff));
    bzero(buff2,sizeof(buff2));
        if ((error = pthread_mutex_lock(&bufferlock))!=0){
            printf("returning error\n");
            //return error;
        }
    }
    if ((error = pthread_mutex_unlock(&bufferlock))!=0){
        printf("returning error\n");
        //return error;
    }
    /* End of thread task*/
}



//c++ copy files
//
//
//
//
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <climits>
#include <algorithm>
#include <signal.h>
#include <errno.h>
#include "buffer.h"
#include <iostream>

static const int BUFSIZE=256;
static buffer_t buffer[BUFSIZE];
static pthread_mutex_t  bufferlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  totallock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  stdoutlock = PTHREAD_MUTEX_INITIALIZER;
static int bufin = 0;
static int bufout = 0;
static int doneflag = 0;
static pthread_cond_t items = PTHREAD_COND_INITIALIZER;
static pthread_cond_t slots = PTHREAD_COND_INITIALIZER;
static int totalitems = 0;

static void *copy_file(void *ptr);
static void *producer(void *ptr);

int getitem(buffer_t *itemp);
int putitem(buffer_t item);
int getdone(int *flag);
int setdone(void);


struct thdata{
};

struct producerData{
    char* sourcedir;
    char* destdir;
};

int main(int argc, char*argv[]){
    if(argc != 4){
        fprintf(stderr,"Incorrect usage\nCorrect usage: copyfile1 dir1 dir2 numberofConsumers\n");
    }
    //files
    thdata data1;
    producerData data2;
    int fc;
    int tempi = 0;
    int err;
    int temp = 0;
    int numConsumers=atoi(argv[3]);
    
    //multithreaded initializations
    fc = 0;
    data2.sourcedir=argv[1];
    data2.destdir=argv[2];
    std::string tempcstr(argv[2],std::find(argv[2],argv[2] + INT_MAX,'\0'));
    std::string tempcstra(argv[1],std::find(argv[1],argv[1] + INT_MAX,'\0'));
    tempcstr = tempcstr + "/";
    tempcstra = tempcstra + "/";
    pthread_t threads[numConsumers];
    pthread_t producerThrd;
    
    //timer
    long tottime;
    struct timeval tpstart,tpend;
    gettimeofday(&tpstart, NULL);
    
    //create producer thread
    err = pthread_create(&producerThrd, NULL, producer,(void *) &data2);
    if(err){
        printf("Error creating thread producer");
    }
    sleep(1);
    for(int i =0;i<numConsumers;i++){
            err = pthread_create(&threads[i], NULL, copy_file,(void *) &data1);
            if(err){
                printf("Error creating thread %d\n",i);
            }
    }
    pthread_join(producerThrd, NULL);
    for(int i=0;i<numConsumers;i++){
       //join pthreads consumers
       pthread_join(threads[i], NULL);
       sleep(1);
        
    }
    //total time
    gettimeofday(&tpend, NULL);
    tottime=1000000L*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_usec-tpstart.tv_usec);
    printf("total copytime: %dmusec\n",tottime);

    return 0;
}

static void *copy_file(void *ptr){
    //copy file
        ssize_t br;
        int sz;
        thdata *data;
        buffer_t nextitem;
        buffer_t* addritem=&nextitem;
        data = (thdata *) ptr;
        int char1;
        int *tmpflag;
    
        pthread_mutex_lock(&totallock);
        while((totalitems>0)&&(bufout!=totalitems)){
            pthread_mutex_unlock(&totallock);
            sleep(1);
            
            pthread_mutex_lock(&totallock);
            //check there are still items in buffer
            if(totalitems==0){break;}
            pthread_mutex_unlock(&totallock);
            
            //get item from buffer
            int error=getitem(addritem);
            off_t fs = lseek(nextitem.infd,0,SEEK_END);
            char *buff=(char*)malloc(sizeof(char)*fs);
            off_t fs2 = lseek(nextitem.infd,0,SEEK_SET);
            pthread_mutex_lock(&stdoutlock);
            std::cout<<"completing copying "<<nextitem.filename<<" to new directory...\n";
            pthread_mutex_unlock(&stdoutlock);
            br=read(nextitem.infd,buff,fs);
            br=write(nextitem.outfd,buff,fs);
            free(buff);         //deallocate temporary buffer
        }
        pthread_mutex_unlock(&totallock);
}

 static void *producer(void *ptr){
    producerData *data1;
    data1 = (producerData *) ptr;
    DIR *dir1;
    DIR *dir2;
    struct dirent *dstr1;
    struct dirent *dstr2;
    int file1;
    int file2;
    int err1;
    buffer_t tempbuf;
    if((dir1=opendir(data1->sourcedir))==NULL){
        fprintf(stderr,"Cannot open %s\n",(data1->sourcedir));
    }
    if((dir2=opendir(data1->destdir))==NULL){
        fprintf(stderr,"Cannot open %s\n",(data1->destdir));
    }
     //get directory names
     std::string tempcstr(data1->destdir,std::find(data1->destdir,data1->destdir + INT_MAX,'\0'));
     std::string tempcstra(data1->sourcedir,std::find(data1->sourcedir,data1->sourcedir + INT_MAX,'\0'));
     tempcstr = tempcstr + "/";     //destination
     tempcstra = tempcstra + "/";   //source
     
     //read directory
          while((dstr1 = readdir(dir1))!= NULL){
              if( !strcmp(dstr1->d_name, ".") || !strcmp(dstr1->d_name,"..")){/*printf("is . or ..\n");*/}
              else{
                  //put in buffer
                  if(!(file1 = open(((tempcstra)+(dstr1->d_name)).c_str(),O_RDONLY))){
                      printf("file1 couldn't be opened\n");
                  }
                  if(!(file2 = open(((tempcstr)+(dstr1->d_name)).c_str(),O_WRONLY | O_CREAT | O_APPEND,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))){
                      printf("file1 couldn't be opened\n");
                  }
                  std::string tempfn(dstr1->d_name,std::find(dstr1->d_name,dstr1->d_name + INT_MAX,'\0'));
                  tempbuf.filename=tempfn;
                  pthread_mutex_lock(&stdoutlock);
                  std::cout<<"completing putting "<<tempfn<<" in buffer...\n";
                  pthread_mutex_unlock(&stdoutlock);
                  tempbuf.infd=file1;
                  tempbuf.outfd=file2;
                  err1=putitem(tempbuf);
                  if(err1!=0){
                      printf("error putting item in buffer");
                  }
              }
          }
     err1=setdone();    //done putting all files in buffer
 }
          
//buffer management functions
          
          int getitem(buffer_t *itemp){  //get from buffer
              int error;
              if ((error = pthread_mutex_lock(&bufferlock))!=0){
                  printf("returning error\n");
                  return error;
              }
              while ((totalitems <= 0) && !error && !doneflag){
                  error = pthread_cond_wait (&items, &bufferlock);
              }
              if (error) {
                  pthread_mutex_unlock(&bufferlock);
                  return error;
              }
              if (doneflag && (totalitems <= 0)) {
                  pthread_mutex_unlock(&bufferlock);
                  return ECANCELED;
              }
              //read buffer
              *itemp = buffer[bufout];
              bufout = (bufout + 1) % BUFSIZE;
              totalitems--;
              if (error = pthread_cond_signal(&slots)) {
                  pthread_mutex_unlock(&bufferlock);
                  return error;
              }
              return pthread_mutex_unlock(&bufferlock);
          }
          
          int putitem(buffer_t item){
              int error;
              if (error = pthread_mutex_lock(&bufferlock))
                  return error;
              while ((totalitems >= BUFSIZE) && !error && !doneflag)
                  error = pthread_cond_wait (&slots, &bufferlock);
              if (error) {
                  pthread_mutex_unlock(&bufferlock);
                  return error;
              }
              if (doneflag) {               //consumers maybe gone
                  pthread_mutex_unlock(&bufferlock);
                  return ECANCELED;
              }
              buffer[bufin] = item;
              bufin = (bufin + 1) % BUFSIZE;
              totalitems++;
              if (error = pthread_cond_signal(&items)) {
                  pthread_mutex_unlock(&bufferlock);
                  return error;
              }
              return pthread_mutex_unlock(&bufferlock);
          }
          
          int getdone(int *flag) {
              int error;
              if (error = pthread_mutex_lock(&bufferlock))
                  return error;
              *flag = doneflag;
              return pthread_mutex_unlock(&bufferlock);
          }
          
          int setdone(void){ //set flag=done, inform all threads
              int error1;
              int error2;
              int error3;
              if (error1 = pthread_mutex_lock(&bufferlock))
                  return error1;
              doneflag = 1;
              error1 = pthread_cond_broadcast(&items);
              error2 = pthread_cond_broadcast(&slots);
              error3 = pthread_mutex_unlock(&bufferlock);
              if (error1)
                  return error1;
              if (error2)
                  return error2;
              if (error3)
                  return error3;
              return 0;
          }

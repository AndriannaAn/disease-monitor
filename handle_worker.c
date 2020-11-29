#include "entries.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	
#include "server.h"

void handle_worker(workerArguments *args){
	int connection=args->fd;
	serverCountryInfo *countryInfoList=args->countryInfoList;
	pthread_mutex_t *mutex_workerUpdate=args->mutex_workerUpdate;
	struct sockaddr_in *sa = args->sa;
	char buffer[100];
	int r,bufferSize=100;
	int len=0;
	short workerPort;
	r=read(connection,&workerPort,sizeof(short));
	if(r==0){
		close(connection);
		return;
	}
	read(connection,&len,sizeof(int));
	len=ntohl(len);
	char countryName[32];
	countryName[0]='\0';
	while(len>0){
		if(len>=bufferSize){
			r=read(connection,buffer,bufferSize);
		}
		else{
			r=read(connection,buffer,len);
		}
		strncat(countryName,buffer,r);
		len=len-r;
	}
	if(len!=-2){
		pthread_mutex_lock(mutex_workerUpdate);
		if(countryInfoList->workerPort==0){
			strcpy(countryInfoList->country,countryName);
			countryInfoList->workerPort=workerPort;
			countryInfoList->workerIp=*sa;
			free(sa);
			countryInfoList->next=NULL;
		}
		else{
			serverCountryInfo *new;
			new=(serverCountryInfo*)malloc(sizeof(serverCountryInfo));
			new->next=countryInfoList->next;
			strcpy(new->country,countryName);
			new->workerPort=workerPort;
			new->workerIp=*sa;
			free(sa);
			countryInfoList->next=new;
		}
		pthread_mutex_unlock(mutex_workerUpdate);
	}
	else{
		close(connection);
		return;
	}
			
	read(connection,&len,sizeof(int));
	len=ntohl(len);
	while(len!=-2){
		char date[11];
		date[0]='\0';
		while(len>0){
			if(len>=bufferSize){
				r=read(connection,buffer,bufferSize);
			}
			else{
				r=read(connection,buffer,len);
			}					
			strncat(date,buffer,r);
			len=len-r;
		}
		//printf("%s\n%s\n",date,countryName);

		read(connection,&len,sizeof(int));	
		len=ntohl(len);
		while(len!=-1){
			char disease[32];
			disease[0]='\0';
			while(len>0){
				if(len>=bufferSize){
					r=read(connection,buffer,bufferSize);
				}
				else{
					r=read(connection,buffer,len);
				}
				strncat(disease,buffer,r);
				len=len-r;
			}
			read(connection,&len,sizeof(int));
			len=ntohl(len);
			int groupA=len;
			read(connection,&len,sizeof(int));
			len=ntohl(len);
			int groupB=len;
			read(connection,&len,sizeof(int));
			len=ntohl(len);
			int groupC=len;
			read(connection,&len,sizeof(int));
			len=ntohl(len);
			int groupD=len;

			//printf("%s\nAge range 0-20 years: %d cases\nAge range 21-40 years: %d cases\nAge range 41-60 years: %d cases\nAge range 60+ years: %d cases\n\n",disease,groupA,groupB,groupC,groupD);
			read(connection,&len,sizeof(int));
			len=ntohl(len);
		}
		read(connection,&len,sizeof(int));
		len=ntohl(len);
	}
	close(connection);
}
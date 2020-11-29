#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "scheduler.h"
#include <signal.h>

volatile sig_atomic_t glbflag=1;

void f(int signum){		//set signal again and change flag
	signal(signum,f);
	glbflag=0;
}


int main( int argc , char* argv[]){
	if(argc!=9){
		fprintf(stderr,"error\n");
		return -1;
	}
	int numThreads,bufferSize,i,statPort,queryPort;
	for(i=1;i<9;i+=2){
		if(strcmp(argv[i], "-w") == 0){
			numThreads=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-b") == 0){
			bufferSize=atoi(argv[i+1]);		
		}
		else if(strcmp(argv[i], "-s") == 0){
			statPort=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-q") == 0){
			queryPort=atoi(argv[i+1]);
		}
		else{
			fprintf(stderr,"error\n");
			return -1;
		}
	}
	/*prepare server's statistics listening socket*/
	struct sockaddr_in server;
	int lsock;
	int reuse_addr = 1;
	struct sockaddr *serverptr=(struct sockaddr *)&server;
	if((lsock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return 1;
	}
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;       /* Internet domain */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(statPort);      /* The given statPort */
	
	if(bind(lsock, serverptr, sizeof(server)) < 0){
		perror("bind");
		return 1;
	}
	if(listen(lsock, numThreads*3) < 0) {
		perror("listen");
		return 1;
	}

	/*prepare server's whoClient listening socket*/
	struct sockaddr_in serverClients;
	int clientSock;
	struct sockaddr *serverClientsptr=(struct sockaddr *)&serverClients;
	if((clientSock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return 1;
	}
	setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
	memset((char *) &serverClients, 0, sizeof(serverClients));
	serverClients.sin_family = AF_INET;       /* Internet domain */
	serverClients.sin_addr.s_addr = htonl(INADDR_ANY);
	serverClients.sin_port = htons(queryPort);      /* The given queryPort */
	
	if(bind(clientSock, serverClientsptr, sizeof(serverClients)) < 0){
		perror("bind");
		return 1;
	}
	if(listen(clientSock, numThreads*3) < 0) {
		perror("listen");
		return 1;
	}


	serverCountryInfo *countryInfoList;
	countryInfoList=(serverCountryInfo*)malloc(sizeof(serverCountryInfo));
	countryInfoList->workerPort=0;
	countryInfoList->next=NULL;
	fd_set listeningSocks;
	int highsock;
	int connection;
	JobScheduler *scheduler=Init(numThreads,bufferSize);
	signal(SIGINT,f);
	//struct sockaddr_in sa = {0};
	socklen_t socklen =sizeof(struct sockaddr_in);
	while(glbflag==1){
		FD_SET(lsock,&listeningSocks);
		FD_SET(clientSock,&listeningSocks);
		if(lsock>clientSock){
			highsock=lsock;
		}
		else{
			highsock=clientSock;
		}
		select(highsock+1,&listeningSocks,NULL,NULL,NULL);
		if(glbflag==0){
			break;
		}
		if(FD_ISSET(lsock,&listeningSocks)){
			struct sockaddr_in *sa=malloc(sizeof(struct sockaddr_in));
			connection = accept(lsock, (struct sockaddr *) sa, &socklen);

			Job job;
			job.routine=(void*)handle_worker;
			workerArguments *arg=(workerArguments*)malloc(sizeof(workerArguments));
			arg->fd=connection;
			arg->countryInfoList=countryInfoList;
			arg->mutex_workerUpdate=&(scheduler->mutex_workerUpdate);
			arg->sa=sa;
			job.arg=arg;
			Schedule(job,scheduler);
		}
		if(FD_ISSET(clientSock,&listeningSocks)){
			connection=accept(clientSock,NULL,NULL);
			Job job;
			job.routine=(void*)handle_client;
			
			clientArguments *arg=(clientArguments*)malloc(sizeof(clientArguments));
			arg->fd=connection;
			arg->countryInfoList=countryInfoList;
			job.arg=arg;
			Schedule(job,scheduler);
		}
	}
	Stop(scheduler);
	Destroy(scheduler);
	serverCountryInfo *fr=countryInfoList;
	while(fr!=NULL){
		fr=fr->next;
		free(countryInfoList);
		countryInfoList=fr;
	}

}
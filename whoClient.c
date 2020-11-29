#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <string.h>
#include <unistd.h>
#include "server.h"
#include <pthread.h>


typedef struct  client_thread{
	struct sockaddr_in server;
	char query[100];
	pthread_mutex_t *mutex_client;
}client_thread;

void* thread_client(void *args){
	client_thread *arg=args;
	int sock;
	int reuse_addr = 1;
	struct sockaddr_in server=arg->server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	char* query=arg->query;
	pthread_mutex_t *mutex_client=arg->mutex_client;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return NULL;
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
	pthread_mutex_lock(mutex_client);
	pthread_mutex_unlock(mutex_client);
	if (connect(sock, serverptr, sizeof(server)) < 0){
		perror("connect");
		return NULL;
	}
	char result[1000];
	int chars=strlen(query);
	int networkInt=htonl(chars);
	write(sock,&networkInt,sizeof(int));
	write(sock,query,chars);
	read(sock,&networkInt,sizeof(int));
	chars=ntohl(networkInt);
	
	read(sock,result,chars);
	printf("%s\n",result);
	
	close(sock);
	return NULL;
}


int main( int argc, char *argv[]){
	if(argc!=9){
		fprintf(stderr,"error\n");
		return -1;
	}
	char filename[50];
	char ips[50];
	int num_of_threads,serverPort;
	int i;
	for(i=1;i<9;i+=2){
		if(strcmp(argv[i], "-w") == 0){
			num_of_threads=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-sip") == 0){
			strcpy(ips,argv[i+1]);
		}
		else if(strcmp(argv[i], "-sp") == 0){
			serverPort=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-q") == 0){
			strcpy(filename,argv[i+1]);
		}
		else{
			fprintf(stderr,"error\n");
			return -1;
		}
	}
	struct sockaddr_in server;
	struct hostent *rem;
	if((rem = gethostbyname(ips)) == NULL) {	/* Find server address */
		herror("gethostbyname"); exit(1);
	}
	
	server.sin_family = AF_INET;       /* Internet domain */
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length); /* Find server address */
	server.sin_port = htons(serverPort);         /* Server port */
	
	int working;
	pthread_mutex_t mutex_client;
	pthread_mutex_init(&mutex_client, NULL);
	FILE *fp;
	size_t bufsize = 256;
	char *query=NULL;
	
	fp=fopen(filename,"r");
	int chars=getline(&query,&bufsize,fp)+1;
	pthread_t *id=malloc(sizeof(pthread_t)*num_of_threads);
	client_thread **arg=malloc(sizeof(client_thread*)*num_of_threads);
	for(i=0;i<num_of_threads;i++){
		arg[i]=malloc(sizeof(client_thread));
	}
	while(chars>0){
		pthread_mutex_lock(&mutex_client); //synchronize threads
		for(i=0;i<num_of_threads;i++){
			arg[i]->server=server;
			arg[i]->mutex_client=&mutex_client;
			strcpy(arg[i]->query,query);
			pthread_create(&(id[i]), NULL, thread_client, (void*)arg[i]);
			working=i+1;	//number of threads created
			chars=getline(&query,&bufsize,fp);
			if(chars<=0){
				break;
			}
		}
		pthread_mutex_unlock(&mutex_client);
		for(i=0;i<working;i++){
			pthread_join(id[i], NULL);	//wait for this batch of threads to finish
		}
	}
	//clean up
	free(id);
	free(query);
	for(i=0;i<num_of_threads;i++){
		free(arg[i]);
	}
	free(arg);
	fclose(fp);
}
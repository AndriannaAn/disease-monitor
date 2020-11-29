//Andrianna Anastasopoulou 
//sdi1300009
#include "entries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */

volatile sig_atomic_t sigusr1flag=0;

volatile sig_atomic_t sigflag=1;

void f(int signum){	//set signal again and change flag
	signal(signum,f);
	sigflag=0;
}

void user1(int signum){
	signal(signum,user1);
	sigusr1flag=1;
}

int main( int argc, char *argv[]){

	signal(SIGINT,f);
	signal(SIGQUIT,f);
	signal(SIGUSR1,user1);
	char filename[50],*buffer,del[50],dir[50];
	int bufferSize=atoi(argv[2]);
	buffer=malloc(bufferSize);
	strcpy(dir,argv[1]);
	int size,nsize;
	//creating and opening named pipes
	sprintf(filename,"%d.send",getpid());
	
	if( mkfifo(filename,0666) <0 ){
		if( errno!=EEXIST) {
			perror("fifo");
			return -1;
		}
	}
	int fd = open( filename ,  O_RDONLY );



	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
	char ips[50];
	int serverPort, sock;
	read(fd,&serverPort,sizeof(int));
	read(fd,&size,sizeof(int));
	read(fd,ips,size);
	if((rem = gethostbyname(ips)) == NULL) {	/* Find server address */
		herror("gethostbyname"); exit(1);
	}
	int reuse_addr = 1;
	/* Socket to communicate with server*/

	/* Create socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return -1;
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
	server.sin_family = AF_INET;       /* Internet domain */
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length); /* Find server address */
	server.sin_port = htons(serverPort);         /* Server port */


	/*Prepare workers listening socket*/
	struct sockaddr_in client;
	int lsock;
	struct sockaddr *clientptr=(struct sockaddr *)&client;
	if((lsock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return 1;
	}
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
	memset((char *) &client, 0, sizeof(client));
	client.sin_family = AF_INET;      /* Internet domain */
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = htons(0);      /* Assign a random available port */
	
	if(bind(lsock, clientptr, sizeof(client)) < 0){
		perror("bind");
		return 1;
	}
	if(listen(lsock, 30) < 0) {
		perror("listen");
		return 1;
	}
	socklen_t len = sizeof(client);
	if (getsockname(lsock, clientptr, &len) == -1){
		perror("getsockname");
	}
	dirlist* dirHead=NULL;
	int diseaseHashtableNumOfEntries=5,bucketSize=50;
	int total=0,success=0,fail=0;
	fd_set fds;
	int w,totalw,r;
	while(sigflag==1){
		FD_ZERO(&fds);
		FD_SET(fd,&fds);
		FD_SET(lsock,&fds);
		int highfd;
		if(fd>lsock){
			highfd=fd;
		}
		else{
			highfd=lsock;
		}
		int err=select(highfd+1, &fds, NULL, NULL, NULL); //waiting aggregator to send request
		if(err==-1 && errno!=EINTR){
			perror("select");
			return -1;
		}
		if(sigusr1flag==1){ //sigusr1 was send, check for new files
			sigusr1flag=0;
			dirlist *countries=dirHead;
			DIR * dir_ptr;
			struct dirent * direntp;
			while(countries!=NULL){
				char dirpath[100];
				sprintf(dirpath,"%s/%s",dir,countries->dir);
				dir_ptr=opendir(dirpath);
				if(dir_ptr==NULL){
					perror("opendir");
					return -1;
				}
				//reading directories and checking for new files
				while( (direntp=readdir(dir_ptr)) != NULL){
					if ( strcmp(direntp->d_name,".") == 0 || strcmp(direntp->d_name,"..") == 0){	//skip "." and ".."
						continue;
					}
					int newflag=0;
					for(int i=0;i<countries->fileCount;i++){
						if(strcmp(countries->filelist[i],direntp->d_name)==0){
							newflag=1;
							break;
						}
					}
					if(newflag==0){
						countries->fileCount+=1;
						countries->filelist=realloc(countries->filelist,countries->fileCount*sizeof(char*));
						countries->filelist[countries->fileCount-1]=malloc(11);
						strcpy(countries->filelist[countries->fileCount-1],direntp->d_name);
						readfile(countries, countries->dir,dir,countries->filelist[countries->fileCount-1],diseaseHashtableNumOfEntries,countries->diseaseHash,sock);
					}
				}
				closedir(dir_ptr);
				countries=countries->next;
			}
		}
		if(FD_ISSET(fd,&fds)!=0 && err!=-1){
			r=read(fd,buffer,1); //reading requested code
			if(r<0){
				perror("read");
				return -1;
			}
			if(buffer[0]=='d'){ //read new subdirectory/country
				if (connect(sock, serverptr, sizeof(server)) < 0){
					perror("connect");
					return -1;
				}
				read(fd,&size,sizeof(int));
				char countryName[32];
				countryName[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(fd,buffer,bufferSize);
					}
					else{
						r=read(fd,buffer,size);
						
					}
					strncat(countryName,buffer,r);
					size=size-r;
				}

				DIR * dir_ptr;
				struct dirent * direntp;
				char subdir[100];
				sprintf(subdir,"%s/%s",dir,countryName);
				dir_ptr=opendir(subdir);
				if(dir_ptr==NULL){
					perror("opendir");
					return -1;
				}
				//linked list with a country/subdirectory per node
				dirlist* findlast;
				if(dirHead==NULL){
					dirHead = malloc(sizeof(dirlist));
					dirHead->recordList=NULL;
					dirHead->next=NULL;
					strcpy(dirHead->dir,countryName);
					findlast=dirHead;
				}
				else{
					findlast=dirHead;
					while(findlast->next!=NULL){
						if(strcmp(findlast->dir,countryName)==0){
							fprintf(stderr,"already read that subdirectory\n");
						}
						findlast=findlast->next;
					}
					findlast->next=malloc(sizeof(dirlist));
					findlast=findlast->next;
					findlast->next=NULL;
					findlast->recordList=NULL;

					strcpy(findlast->dir,countryName);
				}
				int count=-2;
				while( (direntp=readdir(dir_ptr)) != NULL){ //counting files in subdirectory/country
					count++;
				}

				//allocate space for the filenames
				char **matrix;
				matrix=(char**)malloc(count*sizeof(char*));
				int i;
				for(i=0;i<count;i++){
					matrix[i]=(char*)malloc(11);
				}
				i=0;

				rewinddir(dir_ptr);

				//saving filenames
				while( (direntp=readdir(dir_ptr)) != NULL){
					if ( strcmp(direntp->d_name,".") == 0 || strcmp(direntp->d_name,"..") == 0){	//skip "." and ".."
						continue;
					}
					strcpy(matrix[i],direntp->d_name);
					i++;
				}

				quicksort(matrix,count); //sorting files in chronological order
				closedir(dir_ptr);

				findlast->filelist=matrix; //keeping file names in linked list
				findlast->fileCount=count;	//keeping file counter in linked list
				findlast->diseaseHash=readDirectory(diseaseHashtableNumOfEntries,bucketSize, countryName , dir, matrix , count ,findlast,sock,client.sin_port); //read data and keep disease hash table in linked list
			}
			else if(buffer[0]=='X'){
				close(fd);
				free(buffer);
				return 0;
			}
		}
		if(FD_ISSET(lsock,&fds)!=0 && err!=-1){
			int connection = accept(lsock,NULL, NULL);
			r=read(connection,buffer,1); //reading requested code
			if(r<0){
				perror("read");
				return -1;
			}

			if(buffer[0]=='T'){	//topk
				total++;

				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char disease[32];
				disease[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(disease,buffer,r);
					size=size-r;
				}
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date1[11];
				date1[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					
					strncat(date1,buffer,r);
					size=size-r;
				}
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date2[11];
				date2[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(date2,buffer,r);
					size=size-r;
				}
				date start_date=string_to_date(date1);
				date end_date=string_to_date(date2);
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char country[32];
				country[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(country,buffer,r);
					size=size-r;
				}
				int diseaseFlag=0;
				int countryFlag=0;
				dirlist* countries=dirHead;
				while(countries!=NULL){
					if(strcmp(country,countries->dir)==0){
						countryFlag=1;
						bucket* temp_bucket;
						entry* temp_entry;
						int i;
						for(i=0;i<diseaseHashtableNumOfEntries;i++){
							temp_bucket=countries->diseaseHash->buckets[i];
							while(temp_bucket!=NULL){
								temp_entry=temp_bucket->entries;
								while(temp_entry!=NULL){
									if(strcmp(disease,temp_entry->value)==0){
										int groupA=0,groupB=0,groupC=0,groupD=0;
										countGroupAge(temp_entry->root,&groupA,&groupB ,&groupC ,&groupD,start_date,end_date);
										int total=groupA+groupB+groupC+groupD;
										diseaseFlag=1;
										if(total==0){
											groupA=-1;
											nsize=htonl(groupA);
											write(connection,&nsize,sizeof(int));
										}
										else{
											groupA=(groupA*100)/total;
											groupB=(groupB*100)/total;
											groupC=(groupC*100)/total;
											groupD=(groupD*100)/total;
											nsize=htonl(groupA);
											write(connection,&nsize,sizeof(int));
											nsize=htonl(groupB);
											write(connection,&nsize,sizeof(int));
											nsize=htonl(groupC);
											write(connection,&nsize,sizeof(int));
											nsize=htonl(groupD);
											write(connection,&nsize,sizeof(int));
										}
										break;
									}
									temp_entry=temp_entry->next;
								}
								if(diseaseFlag==1){
									break;
								}
								temp_bucket=temp_bucket->next;
							}
						}
					}
					if(countryFlag==1){
						break;
					}
					countries=countries->next;
				}
				if(diseaseFlag==0){
					int notFound=-1;
					notFound=htonl(notFound);
					write(connection,&notFound,sizeof(int));
				}
				success++;
			}

			else if(buffer[0]=='F'){	//frequency
				total++;
				
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char disease[32];
				disease[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(disease,buffer,r);
					size=size-r;
				}

				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date1[11];
				date1[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					
					strncat(date1,buffer,r);
					size=size-r;
				}

				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date2[11];
				date2[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(date2,buffer,r);
					size=size-r;
				}
				
				date start_date=string_to_date(date1);
				date end_date=string_to_date(date2);
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				int totalresult=0;
				dirlist* countries=dirHead;
				if(size==0){
					while(countries!=NULL){
						totalresult+=diseaseFrequency(disease,countries->dir,start_date,end_date,diseaseHashtableNumOfEntries,countries->diseaseHash);

						countries=countries->next;
					}
				}
				else{
					char country[32];
					country[0]='\0';
					while(size>0){
						if(size>=bufferSize){
							r=read(connection,buffer,bufferSize);
						}
						else{
							r=read(connection,buffer,size);
							
						}
						strncat(country,buffer,r);
						size=size-r;
					}
					while(countries!=NULL){
						if(strcmp(country,countries->dir)==0){
							break;
						}
						countries=countries->next;
					}
					if(countries!=NULL){
						totalresult+=diseaseFrequency(disease,country,start_date,end_date,diseaseHashtableNumOfEntries,countries->diseaseHash);
					}
				}
				totalresult=htonl(totalresult);
				write(connection,&totalresult,sizeof(int));
				success++;
			}
			else if(buffer[0]=='R'){	//record
				total++;
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char id[32];
				id[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(id,buffer,r);
					size=size-r;
				}
				dirlist* countries=dirHead;
				int flag=0;
				while(countries!=NULL && flag==0){
					patients *records=countries->recordList;
					while(records!=NULL){
						if(strcmp(records->id,id)==0){
							flag=1;
							write(connection,"y",1); //sending to aggregator that the record was found
							
							size=strlen(records->id)+1;
							nsize=htonl(size);
							write(connection,&nsize,sizeof(int));
							totalw=0;
							while(size>0){
								w=write(connection,(records->id)+totalw,size);
								size-=w;
								totalw+=w;
							}
							size=strlen(records->firstName)+1;
							nsize=htonl(size);
							write(connection,&nsize,sizeof(int));
							totalw=0;
							while(size>0){
								w=write(connection,(records->firstName)+totalw,size);
								size-=w;
								totalw+=w;
							}
							
							size=strlen(records->lastName)+1;
							nsize=htonl(size);
							write(connection,&nsize,sizeof(int));
							totalw=0;
							while(size>0){
								w=write(connection,(records->lastName)+totalw,size);
								size-=w;
								totalw+=w;
							}
							
							size=strlen(records->disease)+1;
							nsize=htonl(size);
							write(connection,&nsize,sizeof(int));
							totalw=0;
							while(size>0){
								w=write(connection,(records->disease)+totalw,size);
								size-=w;
								totalw+=w;
							}
							nsize=htonl(records->age);
							write(connection,&nsize,sizeof(int));

							
							write(connection,&(records->entryDate),sizeof(date));
							write(connection,&(records->exitDate),sizeof(date));
							break;

						}
						
						records=records->next;
					}
					countries=countries->next;
				}
				if(flag==0){
					write(connection,"n",1); //record was not found
				}
				success++;
			}


			else if(buffer[0]=='A'){	//admissions
				total++;
				dirlist* countries=dirHead;
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char disease[32];
				disease[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(disease,buffer,r);
					size=size-r;
				}


				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date1[11];
				date1[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					
					strncat(date1,buffer,r);
					size=size-r;
				}


				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date2[11];
				date2[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(date2,buffer,r);
					size=size-r;
				}
				date start_date=string_to_date(date1);
				date end_date=string_to_date(date2);
				
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				if(size==0){
					while(countries!=NULL){
						int counter = numPatientAdmissionsAllCountries(disease,start_date,end_date, diseaseHashtableNumOfEntries, countries->diseaseHash);
						size=strlen(countries->dir)+1;
						nsize=htonl(size);
						write(connection,&nsize,sizeof(int));
						totalw=0;
						while(size>0){
							w=write(connection,(countries->dir)+totalw,size);
							size-=w;
							totalw+=w;
						}
						nsize=htonl(counter);
						write(connection,&nsize,sizeof(int));
						countries=countries->next;
					}
				}
				else{
					char country[32];
					country[0]='\0';
					while(size>0){
						if(size>=bufferSize){
							r=read(connection,buffer,bufferSize);
						}
						else{
							r=read(connection,buffer,size);
							
						}
						strncat(country,buffer,r);
						size=size-r;
					}
					
					while(countries!=NULL){
						if(strcmp(country,countries->dir)==0){
							break;
						}
						countries=countries->next;
					}
					if(countries!=NULL){
						int counter= numPatientAdmissionsAllCountries(disease,start_date,end_date, diseaseHashtableNumOfEntries, countries->diseaseHash);
						size=strlen(countries->dir)+1;
						nsize=htonl(size);
						write(connection,&nsize,sizeof(int));
						totalw=0;
						while(size>0){
							w=write(connection,(countries->dir)+totalw,size);
							size-=w;
							totalw+=w;
						}
						nsize=htonl(counter);
						write(connection,&nsize,sizeof(int));
					}
				}
				success++;
			}
			else if(buffer[0]=='E'){	//discharges
				total++;
				dirlist* countries=dirHead;
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char disease[32];
				disease[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(disease,buffer,r);
					size=size-r;
				}


				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date1[11];
				date1[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					
					strncat(date1,buffer,r);
					size=size-r;
				}


				read(connection,&size,sizeof(int));
				size=ntohl(size);
				char date2[11];
				date2[0]='\0';
				while(size>0){
					if(size>=bufferSize){
						r=read(connection,buffer,bufferSize);
					}
					else{
						r=read(connection,buffer,size);
						
					}
					strncat(date2,buffer,r);
					size=size-r;
				}
				date start_date=string_to_date(date1);
				date end_date=string_to_date(date2);
				
				read(connection,&size,sizeof(int));
				size=ntohl(size);
				if(size==0){
					while(countries!=NULL){
						int counter = discharges(disease,start_date,end_date, diseaseHashtableNumOfEntries, countries->diseaseHash);
						size=strlen(countries->dir)+1;
						nsize=htonl(size);
						write(connection,&nsize,sizeof(int));
						totalw=0;
						while(size>0){
							w=write(connection,(countries->dir)+totalw,size);
							size-=w;
							totalw+=w;
						}
						nsize=htonl(counter);
						write(connection,&nsize,sizeof(int));
						countries=countries->next;
					}
				}
				else{
					char country[32];
					country[0]='\0';
					while(size>0){
						if(size>=bufferSize){
							r=read(connection,buffer,bufferSize);
						}
						else{
							r=read(connection,buffer,size);
							
						}
						strncat(country,buffer,r);
						size=size-r;
					}
					while(countries!=NULL){
						if(strcmp(country,countries->dir)==0){
							break;
						}
						countries=countries->next;
					}
					if(countries!=NULL){
						int counter = discharges(disease,start_date,end_date, diseaseHashtableNumOfEntries, countries->diseaseHash);
						size=strlen(countries->dir)+1;
						nsize=htonl(size);
						write(connection,&nsize,sizeof(int));
						totalw=0;
						while(size>0){
							w=write(connection,(countries->dir)+totalw,size);
							size-=w;
							totalw+=w;
						}
						nsize=htonl(counter);
						write(connection,&nsize,sizeof(int));
					}
				}
				success++;
			}
			close(connection);
		}
	}
	//making the log file and freeing the allocated memory
	FILE *fp;
	sprintf(filename,"log_file.%d",getpid());
	fp=fopen(filename,"a");
	dirlist* countries;
	while(dirHead!=NULL){
		countries=dirHead;
		dirHead=dirHead->next;
		fprintf(fp,"%s\n",countries->dir);
		free_hash(diseaseHashtableNumOfEntries, countries->diseaseHash);
		for(int i=0;i<countries->fileCount;i++){
			free(countries->filelist[i]);
		}
		free(countries->filelist);
		patients *records;
		while(countries->recordList!=NULL){
			records=countries->recordList;
			countries->recordList=countries->recordList->next;
			free(records);
		}
		free(countries);
	}
	fprintf(fp,"TOTAL %d\nSUCCESS %d\nFAIL %d\n",success+fail,success,fail);
	fclose(fp);
	free(buffer);
	close(fd);
	close(sock);
	sprintf(filename,"%d.send",getpid());
	sprintf(del,"%d.rec",getpid());
	unlink(filename);
	unlink(del);
	return 0;
}
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

void handle_client(clientArguments *args){
	int connection=args->fd;
	serverCountryInfo *countryInfoList=args->countryInfoList;
	int len,nlen,bufferSize=100;
	int i;
	int r, w, totalw;
	int total=0,fail=0,success=0;
	char buffer[100];
	char query[100];
	char result[1000];
	read(connection,&nlen,sizeof(int));
	len=ntohl(nlen);

	query[0]='\0';
	while(len>0){
		if(len>=bufferSize){
			r=read(connection,buffer,bufferSize);
		}
		else{
			r=read(connection,buffer,len);
		}
		strncat(query,buffer,r);
		len=len-r;
	}
	char *extra_words;
	char delim[] = " \r\n";
	char* saveTok;
	sprintf(result,"Query:%s\n",query);
	char *command=strtok_r(query, delim, &saveTok);
	if(command==NULL){
		total++;
		sprintf(result,"%serror\n",result);
		fail++;
	}
	else if(strcmp(command,"/diseaseFrequency\0")==0 ){
		total++;
		char *virus=strtok_r(NULL, delim, &saveTok);
		char *date1=strtok_r(NULL, delim, &saveTok);
		char *date2=strtok_r(NULL, delim, &saveTok);
		char *country=strtok_r(NULL, delim, &saveTok);
		extra_words=strtok_r(NULL, delim, &saveTok);
		if(extra_words!=NULL){
			//printf("too many arguments");
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			len=strlen(result)+1;
			nlen=htonl(len);
			write(connection,&nlen,sizeof(int));
			write(connection,result,len);
			printf("%s\n",result);
			return;
		}
		if((virus!=NULL) && (date1!=NULL) && (date2!=NULL) ){
			int totalcount=0 , workerscount ;
			date start_date=string_to_date(date1);
			date end_date=string_to_date(date2);
			int compare_dates=compareNodes(start_date,end_date);
			if(compare_dates<0){
				//fprintf(stderr,"error\n");
				fail++;
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				printf("%s\n",result);
				return;
			}
			serverCountryInfo *searchCountry = countryInfoList;
			i=-1;
			int sock;
			if(country!=NULL){
				while(searchCountry!=NULL){		//searching the worker handling the requested country       
					if(strcmp(searchCountry->country,country)==0){
						i=0;
						break;
					}
					searchCountry=searchCountry->next;
				}
				if(i==-1){
					sprintf(result,"%sCountry not found\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					success++;
					return;
				}
				//connect searchCountry worker
				struct sockaddr_in worker=searchCountry->workerIp;
				struct sockaddr *workerptr = (struct sockaddr*)&worker;
				
				int reuse_addr = 1;
				/* Create socket */
				if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					perror("socket");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
				worker.sin_family = AF_INET;       /* Internet domain */
				worker.sin_port=searchCountry->workerPort;
				if (connect(sock, workerptr, sizeof(worker)) < 0){
					perror("connect");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}

				write(sock,"F",1);
				len=strlen(virus)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				write(sock,virus,len);
				len=strlen(date1)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date1+totalw,len);	
					len-=w;
					totalw+=w;
				}
				len=strlen(date2)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date2+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(country)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,country+totalw,len);
					len-=w;
					totalw+=w;
				}
			}
			else{ //if argument country is not given we send the arguments to all the workers
				serverCountryInfo *searchCountry = countryInfoList;
				int reuse_addr = 1;
				/* Create socket */
				
				while(searchCountry!=NULL && searchCountry->workerPort!=0){
					struct sockaddr_in worker=searchCountry->workerIp;
					struct sockaddr *workerptr = (struct sockaddr*)&worker;
					worker.sin_family = AF_INET;       /* Internet domain */
					worker.sin_port=searchCountry->workerPort;
					if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
					if (connect(sock, workerptr, sizeof(worker)) < 0){
						perror("connect1");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					write(sock,"F",1);
					len=strlen(virus)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					write(sock,virus,len);
					len=strlen(date1)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date1+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=strlen(date2)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date2+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=0;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					read(sock,&workerscount,sizeof(int));
					workerscount=ntohl(workerscount);
					totalcount+=workerscount;

					searchCountry=searchCountry->next;
				}
			}		
			
			if(country!=NULL){
				read(sock,&workerscount,sizeof(int));
				workerscount=ntohl(workerscount);
				totalcount+=workerscount;
				close(sock);
			}
			sprintf(result,"%s%d\n",result,totalcount);
			success++;
		}
		else{
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
		}
	}
	else if(strcmp(command,"/topk-AgeRanges\0")==0 ){
		total++;
		char *stringk=strtok_r(NULL, delim, &saveTok);
		int k=atoi(stringk);
		char *country=strtok_r(NULL, delim, &saveTok);
		char* disease=strtok_r(NULL, delim, &saveTok);
		char *date1=strtok_r(NULL, delim, &saveTok);
		char *date2=strtok_r(NULL, delim, &saveTok);
		extra_words=strtok_r(NULL, delim, &saveTok);
		if(extra_words!=NULL){
			//printf("too many arguments");
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			len=strlen(result)+1;
			nlen=htonl(len);
			write(connection,&nlen,sizeof(int));
			write(connection,result,len);
			printf("%s\n",result);
			return;
		}

		if((disease!=NULL) && (date1!=NULL) && (date2!=NULL) && (country!=NULL) ){
			date start_date=string_to_date(date1);
			date end_date=string_to_date(date2);
			int compare_dates=compareNodes(start_date,end_date);
			if(compare_dates<0){
				//fprintf(stderr,"error\n");
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				fail++;
				printf("%s\n",result);
				return;
			}
			serverCountryInfo *searchCountry = countryInfoList;
			i=-1;
			while(searchCountry!=NULL){
				if(strcmp(searchCountry->country,country)==0){
					i=0;
					break;
				}
				searchCountry=searchCountry->next;
			}
			if(i==-1){
				sprintf(result,"%sCountry not found\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				success++;
				return;
			}
			struct sockaddr_in worker=searchCountry->workerIp;
			struct sockaddr *workerptr = (struct sockaddr*)&worker;
			int sock;
			int reuse_addr = 1;
			/* Create socket */
			if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
				perror("socket");
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				return;
			}
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
			worker.sin_family = AF_INET;       /* Internet domain */
			worker.sin_port=searchCountry->workerPort;
			if (connect(sock, workerptr, sizeof(worker)) < 0){
				perror("connect");
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				return;
			}
			write(sock,"T",1);
			len=strlen(disease)+1;
			nlen=htonl(len);
			write(sock,&nlen,sizeof(int));
			totalw=0;
			while(len>0){
				w=write(sock,disease+totalw,len);
				len-=w;
				totalw+=w;
			}
			len=strlen(date1)+1;
			nlen=htonl(len);
			write(sock,&nlen,sizeof(int));
			totalw=0;
			while(len>0){
				w=write(sock,date1+totalw,len);
				len-=w;
				totalw+=w;
			}
			len=strlen(date2)+1;
			nlen=htonl(len);
			write(sock,&nlen,sizeof(int));
			totalw=0;
			while(len>0){
				w=write(sock,date2+totalw,len);
				len-=w;
				totalw+=w;
			}
			len=strlen(country)+1;
			nlen=htonl(len);
			write(sock,&nlen,sizeof(int));
			totalw=0;
			while(len>0){
				w=write(sock,country+totalw,len);
				len-=w;
				totalw+=w;
			}

			sortgroups groups[4];
			read(sock,&(groups[0].percentage),sizeof(int));
			groups[0].percentage=ntohl(groups[0].percentage);
			if(groups[0].percentage==-1){
				success++;
				sprintf(result,"%sNo entries for this disease\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				return;
			}
			strcpy(groups[0].group,"0-20");
			read(sock,&(groups[1].percentage),sizeof(int));
			groups[1].percentage=ntohl(groups[1].percentage);
			strcpy(groups[1].group,"20-40");
			read(sock,&(groups[2].percentage),sizeof(int));
			groups[2].percentage=ntohl(groups[2].percentage);
			strcpy(groups[2].group,"40-60");
			read(sock,&(groups[3].percentage),sizeof(int));
			groups[3].percentage=ntohl(groups[3].percentage);
			strcpy(groups[3].group,"60+");

			int swapflag=0;
			sortgroups temp;
			while(swapflag==0){ //sorting the age groups
				swapflag=1;
				for(i=0;i<3;i++){
					if(groups[i].percentage<groups[i+1].percentage){
						swapflag=0;
						temp=groups[i];
						groups[i]=groups[i+1];
						groups[i+1]=temp;
					}
				}
			}
			if(k>4) {
				k=4;
			}
			for(i=0;i<k;i++){
				sprintf(result,"%s%s: %d%%\n",result,groups[i].group,groups[i].percentage);
			}
			success++;
		}
		else{
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
		}
	}
	else if(strcmp(command,"/numPatientAdmissions\0")==0 ){
		total++;
		char* disease=strtok_r(NULL, delim, &saveTok);
		char *date1=strtok_r(NULL, delim, &saveTok);
		char *date2=strtok_r(NULL, delim, &saveTok);
		char *country=strtok_r(NULL, delim, &saveTok);
		extra_words=strtok_r(NULL, delim, &saveTok);
		if(extra_words!=NULL){
			//printf("too many arguments");
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			len=strlen(result)+1;
			nlen=htonl(len);
			write(connection,&nlen,sizeof(int));
			write(connection,result,len);
			printf("%s\n",result);
			return;
		}
		if((disease!=NULL) && (date1!=NULL) && (date2!=NULL) ){
			date start_date=string_to_date(date1);
			date end_date=string_to_date(date2);
			int compare_dates=compareNodes(start_date,end_date);
			if(compare_dates<0){
				//fprintf(stderr,"error\n");
				fail++;
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				printf("%s\n",result);
				return;
			}
			serverCountryInfo *searchCountry = countryInfoList;
			i=-1;
			if(country!=NULL){
				while(searchCountry!=NULL){
					if(strcmp(searchCountry->country,country)==0){
						i=0;
						break;
					}
					searchCountry=searchCountry->next;
				}
				if(i==-1){
					sprintf(result,"%sCountry not found\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					success++;
					return;
				}

				struct sockaddr_in worker=searchCountry->workerIp;
				struct sockaddr *workerptr = (struct sockaddr*)&worker;
				int sock;
				int reuse_addr = 1;
				/* Create socket */
				if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					perror("socket");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
				worker.sin_family = AF_INET;       /* Internet domain */
				worker.sin_port=searchCountry->workerPort;
				if (connect(sock, workerptr, sizeof(worker)) < 0){
					perror("connect");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				write(sock,"A",1);
				len=strlen(disease)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,disease+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(date1)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date1+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(date2)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date2+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(country)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,country+totalw,len);
					len-=w;
					totalw+=w;
				}
				read(sock,&nlen,sizeof(int));
				len=ntohl(nlen);
				char country[32];
				country[0]='\0';
				while(len>0){
					if(len>=bufferSize){
						r=read(sock,buffer,bufferSize);
					}
					else{
						r=read(sock,buffer,len);
					}
					strncat(country,buffer,r);
					len=len-r;
				}
				int admCount;
				read(sock,&admCount,sizeof(int));
				admCount=ntohl(admCount);
				sprintf(result,"%s%s %d\n",result,country,admCount);
			}
			else{
				serverCountryInfo *searchCountry = countryInfoList;
				while(searchCountry!=NULL && searchCountry->workerPort!=0){
					struct sockaddr_in worker=searchCountry->workerIp;
					struct sockaddr *workerptr = (struct sockaddr*)&worker;
					int sock;
					int reuse_addr = 1;
					/* Create socket */
					if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
					worker.sin_family = AF_INET;       /* Internet domain */
					worker.sin_port=searchCountry->workerPort;
					if (connect(sock, workerptr, sizeof(worker)) < 0){
						perror("connect");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					write(sock,"A",1);
					len=strlen(disease)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,disease+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=strlen(date1)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date1+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=strlen(date2)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date2+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=0;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char country[32];
					country[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(country,buffer,r);
						len=len-r;
					}
					int admCount;
					read(sock,&admCount,sizeof(int));
					admCount=ntohl(admCount);
					sprintf(result,"%s%s %d\n",result,country,admCount);
					searchCountry=searchCountry->next;
					close(sock);
				}
			}
			success++;
		}
		else{
			sprintf(result,"%serror\n",result);
			fail++;
		}
	}
	else if(strcmp(command,"/numPatientDischarges\0")==0 ){
		total++;
		char* disease=strtok_r(NULL, delim, &saveTok);
		char *date1=strtok_r(NULL, delim, &saveTok);
		char *date2=strtok_r(NULL, delim, &saveTok);
		char *country=strtok_r(NULL, delim, &saveTok);
		extra_words=strtok_r(NULL, delim, &saveTok);
		if(extra_words!=NULL){
			//printf("too many arguments");
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			len=strlen(result)+1;
			nlen=htonl(len);
			write(connection,&nlen,sizeof(int));
			write(connection,result,len);
			printf("%s\n",result);
			return;
		}
		if((disease!=NULL) && (date1!=NULL) && (date2!=NULL) ){
			date start_date=string_to_date(date1);
			date end_date=string_to_date(date2);
			int compare_dates=compareNodes(start_date,end_date);
			if(compare_dates<0){
				//fprintf(stderr,"error\n");
				fail++;
				sprintf(result,"%serror\n",result);
				len=strlen(result)+1;
				nlen=htonl(len);
				write(connection,&nlen,sizeof(int));
				write(connection,result,len);
				printf("%s\n",result);
				return;
			}
			serverCountryInfo *searchCountry = countryInfoList;
			i=-1;
			if(country!=NULL){
				while(searchCountry!=NULL){
					if(strcmp(searchCountry->country,country)==0){
						i=0;
						break;
					}
					searchCountry=searchCountry->next;
				}
				if(i==-1){
					sprintf(result,"%sCountry not found\n",result);
					success++;
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}

				struct sockaddr_in worker=searchCountry->workerIp;
				struct sockaddr *workerptr = (struct sockaddr*)&worker;
				int sock;
				int reuse_addr = 1;
				/* Create socket */
				if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					perror("socket");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
				worker.sin_family = AF_INET;       /* Internet domain */
				worker.sin_port=searchCountry->workerPort;
				if (connect(sock, workerptr, sizeof(worker)) < 0){
					perror("connect");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				write(sock,"E",1);
				len=strlen(disease)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,disease+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(date1)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date1+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(date2)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,date2+totalw,len);
					len-=w;
					totalw+=w;
				}
				len=strlen(country)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,country+totalw,len);
					len-=w;
					totalw+=w;
				}
				read(sock,&nlen,sizeof(int));
				len=ntohl(nlen);
				char country[32];
				country[0]='\0';
				while(len>0){
					if(len>=bufferSize){
						r=read(sock,buffer,bufferSize);
					}
					else{
						r=read(sock,buffer,len);
					}
					strncat(country,buffer,r);
					len=len-r;
				}
				int admCount;
				read(sock,&admCount,sizeof(int));
				admCount=ntohl(admCount);
				sprintf(result,"%s%s %d\n",result,country,admCount);
			}
			else{
				serverCountryInfo *searchCountry = countryInfoList;
				while(searchCountry!=NULL && searchCountry->workerPort!=0){
					struct sockaddr_in worker=searchCountry->workerIp;
					struct sockaddr *workerptr = (struct sockaddr*)&worker;
					int sock;
					int reuse_addr = 1;
					/* Create socket */
					if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
					worker.sin_family = AF_INET;       /* Internet domain */
					worker.sin_port=searchCountry->workerPort;
					if (connect(sock, workerptr, sizeof(worker)) < 0){
						perror("connect");
						sprintf(result,"%serror\n",result);
						len=strlen(result)+1;
						nlen=htonl(len);
						write(connection,&nlen,sizeof(int));
						write(connection,result,len);
						return;
					}
					write(sock,"E",1);
					len=strlen(disease)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,disease+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=strlen(date1)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date1+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=strlen(date2)+1;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(sock,date2+totalw,len);
						len-=w;
						totalw+=w;
					}
					len=0;
					nlen=htonl(len);
					write(sock,&nlen,sizeof(int));
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char country[32];
					country[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(country,buffer,r);
						len=len-r;
					}
					int admCount;
					read(sock,&admCount,sizeof(int));
					admCount=ntohl(admCount);
					sprintf(result,"%s%s %d\n",result,country,admCount);
					searchCountry=searchCountry->next;
					close(sock);
				}
			}
			success++;
		}
		else{
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			fail++;
		}
	}
	else if(strcmp(command,"/searchPatientRecord\0")==0){
		total++;
		char *recordId=strtok_r(NULL, delim, &saveTok);
		extra_words=strtok_r(NULL, delim, &saveTok);
		if(extra_words!=NULL){
			//printf("too many arguments");
			fail++;
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			len=strlen(result)+1;
			nlen=htonl(len);
			write(connection,&nlen,sizeof(int));
			write(connection,result,len);
			printf("%s\n",result);
			return;
		}
		if(recordId!=NULL){
			serverCountryInfo *searchCountry = countryInfoList;
			int flag=0;
			while(searchCountry!=NULL && searchCountry->workerPort!=0){
				struct sockaddr_in worker=searchCountry->workerIp;
				struct sockaddr *workerptr = (struct sockaddr*)&worker;
				int sock;
				int reuse_addr = 1;
				/* Create socket */
				if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					perror("socket");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
				worker.sin_family = AF_INET;       /* Internet domain */
				worker.sin_port=searchCountry->workerPort;
				if (connect(sock, workerptr, sizeof(worker)) < 0){
					perror("connect");
					sprintf(result,"%serror\n",result);
					len=strlen(result)+1;
					nlen=htonl(len);
					write(connection,&nlen,sizeof(int));
					write(connection,result,len);
					return;
				}
				write(sock,"R",1);
				len=strlen(recordId)+1;
				nlen=htonl(len);
				write(sock,&nlen,sizeof(int));
				totalw=0;
				while(len>0){
					w=write(sock,recordId+totalw,len);
					len-=w;
					totalw+=w;
				}
				read(sock,buffer,1);
				if(buffer[0]=='y'){  //worker found the requested record
					flag=1;		
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char id[32];
					id[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(id,buffer,r);
						len=len-r;
					}
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char firstname[32];
					firstname[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(firstname,buffer,r);
						len=len-r;
					}
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char lastname[32];
					lastname[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(lastname,buffer,r);
						len=len-r;
					}
					read(sock,&nlen,sizeof(int));
					len=ntohl(nlen);
					char virus[32];
					virus[0]='\0';
					while(len>0){
						if(len>=bufferSize){
							r=read(sock,buffer,bufferSize);
						}
						else{
							r=read(sock,buffer,len);
						}
						strncat(virus,buffer,r);
						len=len-r;
					}

					int age;
					read(sock,&age,sizeof(int));
					age=ntohl(age);
					date entry;
					date exit;
					read(sock,&entry,sizeof(date));
					read(sock,&exit,sizeof(date));
					if(exit.year==0){
						sprintf(result,"%s%s %s %s %s %d %d-%d-%d --\n",result,id,firstname,lastname,virus,age, (entry.day),(entry.month),(entry.year));
					}
					else{
						sprintf(result,"%s%s %s %s %s %d %d-%d-%d %d-%d-%d\n",result,id,firstname,lastname,virus,age,(entry.day),(entry.month),(entry.year),(exit.day),(exit.month),(exit.year));
					}
				}
				close(sock);
				searchCountry=searchCountry->next;
			}
			if(flag==0){
				sprintf(result,"%sRecord id not found\n",result);
			}
			success++;
		}
		else{
			//fprintf(stderr,"error\n");
			sprintf(result,"%serror\n",result);
			fail++;
		}
	}
	printf("%s\n",result);
	len=strlen(result)+1;
	nlen=htonl(len);
	write(connection,&nlen,sizeof(int));
	write(connection,result,len);
	close(connection);
}
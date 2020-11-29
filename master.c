//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "entries.h"
#include <time.h>
#include <sys/wait.h>

volatile sig_atomic_t glbflag=1;
volatile sig_atomic_t childflag=0;

void f(int signum){		//set signal again and change flag
	signal(signum,f);
	glbflag=0;
}


void child(int signum){
	signal(signum,child);
	childflag=1;
}


int main( int argc, char *argv[]){
	signal(SIGINT,f);
	signal(SIGQUIT,f);
	// READING ARGUMENTS
	if(argc!=11){
		fprintf(stderr,"error\n");
		return -1;
	}
	int numWorkers,i,port;
	char input_dir[32],stringBuffer[15];
	char hostname[50];
	for(i=1;i<11;i+=2){
		if(strcmp(argv[i], "-w") == 0){
			numWorkers=atoi(argv[i+1]);
		}
		else if(strcmp(argv[i], "-b") == 0){
			strcpy(stringBuffer,argv[i+1]);		
		}
		else if(strcmp(argv[i], "-i") == 0){
			strcpy(input_dir,argv[i+1]);
		}
		else if(strcmp(argv[i], "-s") == 0){
			strcpy(hostname,argv[i+1]);
			printf("hostname:%s\n",hostname);
		}
		else if(strcmp(argv[i], "-p") == 0){
			port=atoi(argv[i+1]);
		}
		else{
			fprintf(stderr,"error\n");
			return -1;
		}
	}


	struct stat st = {0};
		//if there is no input directory or there is input error, exit
	if(stat(input_dir, &st) == -1) {
		perror("Input directory\n");
		exit(1);
	}
	//if input is not directory exit
	else if((st.st_mode & S_IFMT ) != S_IFDIR){
		printf("%s is not a directory\n",input_dir);
		exit(1);
	}

	char filename[20];
	pid_t *workers = malloc(sizeof(pid_t)*numWorkers);
	int *fdsend = malloc(sizeof(int)*numWorkers);
	int w,totalw,len;
	// init workers and named  pipes
	for(i=0;i<numWorkers;i++){
		workers[i]=fork();
		if(workers[i]<0){
			perror("fork");
			return -1;
		}
		else if(workers[i]==0){
			execl("./worker","./worker",input_dir,stringBuffer,NULL);
			return -1;
		}
		sprintf(filename,"%d.send",workers[i]);
		if( mkfifo(filename,0666) <0 ){
			if( errno!=EEXIST) { 
				perror("fifo");
				return -1;
			}
		}
		fdsend[i] = open( filename ,  O_WRONLY );
		if(fdsend[i]<0) {
			perror("parent open");
			return -1;
		}
		write(fdsend[i],&port,sizeof(int));
		len=strlen(hostname)+1;
		write(fdsend[i],&len,sizeof(int));
		totalw=0;
		while(len>0){
			w=write(fdsend[i],hostname+totalw,len);
			len-=w;
			totalw+=w;
		}
	}

	
	DIR * dir_ptr;
	struct dirent * direntp;
	dir_ptr=opendir(input_dir);
	int count=0;
	countryInfo *countryInfoList;
	countryInfoList=NULL;
	//reading input directory and distributing countries/subdirectories to workers
	while( (direntp=readdir(dir_ptr)) != NULL){		
		if ( strcmp(direntp->d_name,".") == 0 || strcmp(direntp->d_name,"..") == 0){	//skip "." and ".."
			continue;
		}
		len=strlen(direntp->d_name)+1;
		write(fdsend[count%numWorkers],"d",1);
		write(fdsend[count%numWorkers],&len,sizeof(int));
		totalw=0;
		while(len>0){
			w=write(fdsend[count%numWorkers],(direntp->d_name)+totalw,len);
			len-=w;
			totalw+=w;
		}
		write(fdsend[count%numWorkers],direntp->d_name,len);
		if(countryInfoList==NULL){
			countryInfoList=(countryInfo*)malloc(sizeof(countryInfo));
			strcpy(countryInfoList->country,direntp->d_name);
			countryInfoList->info=count%numWorkers;
			countryInfoList->next=NULL;
		}
		else{
			countryInfo *new;
			new=(countryInfo*)malloc(sizeof(countryInfo));
			new->next=countryInfoList;
			countryInfoList=new;
			strcpy(countryInfoList->country,direntp->d_name);
			countryInfoList->info=count%numWorkers;
		}
		count++;
	}
	//closing workers that we don't need
	if(count<numWorkers){
		for(i=count;i<numWorkers;i++){
			write(fdsend[i],"X",1); 
			close(fdsend[i]);
			wait(NULL);
			sprintf(filename,"%d.send",workers[i]);
			unlink(filename);
		}
		numWorkers=count;
	}
	signal(SIGCHLD,child);
	
	// waiting for user input and statistics
	while(glbflag==1){
		pause();
		if(childflag==1){ //signal SIGCHLD came 
			//deleting previous named pipes and restarting the worker
			pid_t restart = wait(NULL);
			for(i=0;i<numWorkers;i++){
				if(restart==workers[i]){
					break;
				}
			}
			close(fdsend[i]);
			sprintf(filename,"%d.send",workers[i]);
			unlink(filename);
			workers[i]=fork();
			if(restart<0){
				perror("fork");
				return -1;
			}
			else if(workers[i]==0){
				execl("./worker","./worker",input_dir, stringBuffer,NULL);
				return -1;
			}
			sprintf(filename,"%d.send",workers[i]);
			if( mkfifo(filename,0666) <0 ){
				if( errno!=EEXIST) { 
					perror("fifo");
					return -1;
				}
			}
			fdsend[i] = open( filename ,  O_WRONLY );
			if(fdsend[i]<0) {
				perror("parent open");
				return -1;
			}
			countryInfo *findCountries = countryInfoList;
			while(findCountries!=NULL){
				if(findCountries->info==i){
					len=strlen(findCountries->country)+1;
					write(fdsend[i],"d",1);
					write(fdsend[i],&len,sizeof(int));
					totalw=0;
					while(len>0){
						w=write(fdsend[i],(findCountries->country)+totalw,len);
						len-=w;
						totalw+=w;
					}
				}
				findCountries=findCountries->next;
			}
			childflag=0;
			continue;
		}
	}
	closedir(dir_ptr);
	for(i=0;i<numWorkers;i++){		// closing workers and named pipes
		kill(workers[i],SIGKILL);
		close(fdsend[i]);
		sprintf(filename,"%d.send",workers[i]); 
		unlink(filename);
	}
	for(i=0;i<numWorkers;i++){
		wait(NULL);
	}
	//freeing the allocated memory
	countryInfo* countries;
	while(countryInfoList!=NULL){
		countries=countryInfoList;
		countryInfoList=countryInfoList->next;
		free(countries);
	}
	free(fdsend);
	free(workers);
	return 0;
}

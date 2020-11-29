//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>


int compares(char* enDate,char* exDate){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if(strcmp(enDate,"\0")==0){
		return -100;
	}
	int enDay,enMonth,enYear;
	sscanf(enDate, "%d-%d-%d", &enDay,&enMonth,&enYear);
	if(enDay<0 || enDay>31 || enMonth<1 || enMonth>12 || enYear<0 || enYear>(tm.tm_year +1901)){
		return -100;
	}
	int daysOfStay;
	if(strcmp(exDate,"\0")==0){
		return -100;
	}
	int exDay,exMonth,exYear;
	sscanf(exDate, "%d-%d-%d", &exDay,&exMonth,&exYear);
	if(exDay<0 || exDay>31 ||  exMonth<1 || exMonth>12 || exYear<0 || exYear>(tm.tm_year +1901)){
		return -100;
	}
	int daysEn= enDay+(enMonth*30)+(enYear*365);
	int daysEx= exDay+(exMonth*30)+(exYear*365);
	daysOfStay=daysEx-daysEn;

	return  daysOfStay;
}


void swap(char *ptr1, char* ptr2){

	//SWAPING VALUES
	if(ptr1==ptr2){
		return;
	}
	char temp[11];
	strcpy(temp,ptr1);
	strcpy(ptr1,ptr2);
	strcpy(ptr2,temp);

}



void quicksort(char **start1,int size){
	int smaller=0;
	char pivot[11];
	strcpy(pivot,*start1); //USING FIRST NUMBER AS THE PIVOT
	char **left=start1+1;
	char **right=start1+size-1;
	while(left<=right){
		if(compares(*left,pivot)<0){	//SEARCH KEY > PIVOT FROM LEFT TO RIGHT OR UNTUL LEFT>RIGHT
			while(compares(*right,pivot)<0 && right!=left){	//SEARCH KEY <= PIVOT FROM RIGHT TO LEFT OR UNTIL RIGHT==LEFT
				right--;
			}
			if(right==left){	//IF RIGHT==LEFT WE ARE DONE
				break;	
			}
			else{				//IF LEFT FOUND KEY>PIVOT AND RIGHT FOUND KEY<= PIVOT SWAP THEM
				swap(*left,*right);
			}
		}
		left++;
		smaller++; //COUNT OF NUMBERS SMALLER THAN THE PIVOT
	}
	swap(*(left-1),*start1); //left-1 IS THE LAST NUMBER SMALLER THAN THE PIVOT 

	//QUICKSORT THE LEFT SIDE OF THE PIVOT IF NEEDED
	if(smaller>1)
		quicksort(start1,smaller);
	//QUICKSORT THE RIGHT SIDE OF THE PIVOT IF NEEDED
	if(size-smaller-1>1){
		quicksort(left,size-(smaller+1));
	}
}

date string_to_date(char* strdate){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	date recordDate;
	if(strcmp(strdate,"-")==0){
		recordDate.day=0;
		recordDate.month=0;
		recordDate.year=0;
		return recordDate;
	}
	int chars;
	sscanf(strdate, "%d-%d-%d%n", &recordDate.day,&recordDate.month,&recordDate.year,&chars);
	if(recordDate.day<0 || recordDate.day>31 ||  recordDate.month<1 || recordDate.month>12 || recordDate.year<0 || recordDate.year>(tm.tm_year +1901) || strdate[chars]!='\0'){
		recordDate.day=-1;
		recordDate.month=-1;
		recordDate.year=-1;
	}

	return recordDate;
}




void updateRecord(patients *head,char *id,date exitDate){
	
	int done_flag=0;			
	while(head!=NULL){
		if(strcmp(head->id,id)==0){
			done_flag=1;
			if(head->exitDate.year!=0){
				done_flag=3;		//periptwsh pou to entry eixe hdh exit date
			}		
		}
		if(done_flag==1){
			head->exitDate.day=exitDate.day;
			head->exitDate.month=exitDate.month;
			head->exitDate.year=exitDate.year;						
			//printf("insertion of exit date completed\n");
			//printf("Record updated\n");
			break;
		}
		else if(done_flag==3){
			head->exitDate.day=exitDate.day;
			head->exitDate.month=exitDate.month;
			head->exitDate.year=exitDate.year;	
			//printf("Exit date already existed. Updated with new one\n");
			//printf("Record updated\n");
			break;
		}
		head=head->next;
	}
	if(done_flag==0){
		fprintf(stderr,"Error. Id not found\n");
		//printf("error updating\n");
	}
}

hash_t* readDirectory(int diseaseHashtableNumOfEntries,int bucketSize, char country[32] ,char *dir, char **matrix , int count , dirlist* curDir,int fd,unsigned short port){
	
	int numOfEntries= (bucketSize-sizeof(bucket))/sizeof(entry);
	if(numOfEntries<1){
		//printf("too small buckets, changing their size to fit one entry\n");
		numOfEntries=1;
	}
	hash_t* hashtableDisease;
	hashtableDisease=hash_t_create(diseaseHashtableNumOfEntries);	
	int k=0;		
	int size=strlen(country)+1;
	write(fd,&port,sizeof(short));
	int nsize=htonl(size);
	write(fd,&nsize,sizeof(int));
	write(fd,country,size);
	do{
		readfile(curDir, country,dir, matrix[k], diseaseHashtableNumOfEntries, hashtableDisease, fd );
		
		k++;
		if(k==count){
			break;
		}
	}while(1);
	size=-2;
	nsize=htonl(size);
	write(fd,&nsize,sizeof(int));
	return hashtableDisease;
}



void readfile(dirlist* curDir, char *country,char *dir, char *datefile,int diseaseHashtableNumOfEntries, hash_t* hashtableDisease, int fd){
	char filename[100],str[512],exDate[11]="\0",todo[10];
	sprintf(filename,"%s/%s/%s",dir,country,datefile);
	FILE *file;
	file=fopen(filename,"r");
	patients *head=NULL;
	ageStats* statsHead;
	statsHead=NULL;
	int num;
	if(curDir->recordList==NULL){
		curDir->recordList=malloc(sizeof(patients));
		head=curDir->recordList;
		if(fgets(str, 512, file)!=0){
			strcpy(head->country,country);
			strcpy(exDate,"-");
			head->entryDate=string_to_date(datefile);
			head->exitDate=string_to_date(exDate);
			sscanf(str,"%s %s %s %s %s %d",head->id,todo,head->firstName,head->lastName, head->disease,&(head->age));
			head->next=NULL;
		}
		while( strcmp(head->id,"\0")==0 || strcmp(head->firstName,"\0")==0 || strcmp(head->lastName,"\0")==0 || strcmp(head->disease,"\0")==0 || head->age < 0 || head->age > 120 || strcmp(todo,"ENTER") != 0 ){
			fprintf(stderr,"ERROR\n");
			if(fgets(str,  512, file)!=0){
				sscanf(str,"%s %s %s %s %s %d",head->id,todo,head->firstName,head->lastName, head->disease,&(head->age));
			}
			else{
				//fprintf(stderr,"There are no valid entries. Exiting\n");
				return;
			}
		}
		statsHead=malloc(sizeof(ageStats));
		strcpy(statsHead->disease,head->disease);
		statsHead->groupA=0;
		statsHead->groupB=0;
		statsHead->groupC=0;
		statsHead->groupD=0;
		statsHead->next=NULL;
		if(head->age<=20){
			statsHead->groupA++;
		}
		else if(head->age<=40){
			statsHead->groupB++;
		}
		else if(head->age<=60){
			statsHead->groupC++;								
		}
		else{
			statsHead->groupD++;
		}
		num=hash(head->disease,diseaseHashtableNumOfEntries); 
		insert_to_hash(hashtableDisease,diseaseHashtableNumOfEntries,head,num,head->disease);
	}
	patients *temp=curDir->recordList;
	while(temp->next!=NULL){
		temp=temp->next;
	}
	while(fgets(str,  512, file)!=0){
		if(strcmp(str,"\n")==0){
			break;		//an vrei keni grammi stamataei na diavazei 
		}
		patients *next=NULL;
		next=malloc(sizeof(patients));
		strcpy(next->id,"\0");strcpy(next->firstName,"\0"); strcpy(next->lastName,"\0"); strcpy(next->disease,"\0"); strcpy(next->country,country); next->age=0;
		strcpy(next->country,country);
		strcpy(exDate,"-");
		next->entryDate=string_to_date(datefile);
		next->exitDate=string_to_date(exDate);
		sscanf(str,"%s %s %s %s %s %d ",next->id,todo,next->firstName,next->lastName, next->disease,&(next->age));
		patients *headCopy=curDir->recordList;
		if(strcmp(todo,"ENTER")==0){
			while(headCopy!=NULL){
				if(strcmp(headCopy->id,next->id)==0){
					fprintf(stderr,"error duplicated id\n");
					free(next);
					next=NULL;
					break;
				}
				headCopy=headCopy->next;
			}
			if(next==NULL){
				continue;
			}
		}
		else if (strcmp(todo,"EXIT")==0){
			date tempDate;
			tempDate=string_to_date(datefile);
			updateRecord(headCopy, next->id,tempDate);
			free(next);
			continue;
		}
		else{
			fprintf(stderr,"error not EXIT or ENTER\n");
			free(next);
			continue;
		}
		if (strcmp(next->id,"\0")!=0 && strcmp(next->lastName,"\0")!=0 && strcmp(next->firstName,"\0")!=0 && strcmp(next->disease,"\0")!=0 && strcmp(next->country,"\0")!=0 && next->age>0 && next->age<120  ) {
			if(statsHead==NULL){
				statsHead=malloc(sizeof(ageStats));
				strcpy(statsHead->disease,next->disease);
				statsHead->groupA=0;
				statsHead->groupB=0;
				statsHead->groupC=0;
				statsHead->groupD=0;
				statsHead->next=NULL;
				if(next->age<=20){
					statsHead->groupA++;
				}
				else if(next->age<=40){
					statsHead->groupB++;
				}
				else if(next->age<=60){
					statsHead->groupC++;								
				}
				else{
					statsHead->groupD++;
				}
			}
			else{
				int diseaseFlag=0;
				ageStats* tempStat=statsHead;
				while(tempStat!=NULL){
					if(strcmp(tempStat->disease,next->disease)==0){
						diseaseFlag=1;
						if(next->age<=20){
							tempStat->groupA++;
						}
						else if(next->age<=40){
							tempStat->groupB++;
	
						}
						else if(next->age<=60){
							tempStat->groupC++;
						}
						else {
							tempStat->groupD++;
						}
					}
					tempStat=tempStat->next;
				}
				if(diseaseFlag==0){
					tempStat=malloc(sizeof(ageStats));
					strcpy(tempStat->disease,next->disease);
					tempStat->groupA=0;
					tempStat->groupB=0;
					tempStat->groupC=0;
					tempStat->groupD=0;
					if(next->age<=20){
						tempStat->groupA++;
					}
					else if(next->age<=40){
						tempStat->groupB++;
								
					}
					else if(next->age<=60){
						tempStat->groupC++;
					}
					else{
						tempStat->groupD++;
					}
					tempStat->next=statsHead;
					statsHead=tempStat;
				}
			}
			temp->next=next;
			temp=temp->next;
			num=hash(temp->disease,diseaseHashtableNumOfEntries); 
			//num2=hash(temp->country,countryHashtableNumOfEntries); 
			insert_to_hash(hashtableDisease,diseaseHashtableNumOfEntries,temp,num,temp->disease);
			//insert_to_hash(hashtableCountry,numOfEntries,temp,num2,temp->country);
			temp->next=NULL;
		}
		else{
			free(next);
			//printf("error   wrong dates or null value\n");
		}
	}
	ageStats* tempStat=statsHead;
	int size=strlen(datefile)+1;
	int nsize=htonl(size);
	write(fd,&nsize,sizeof(int));
	write(fd,datefile,size);
	while(tempStat!=NULL){
		//printf("%s STATSSSS: %s: groupA: %d groupB: %d groupC: %d groupD: %d\n",filename,tempStat->disease,tempStat->groupA,tempStat->groupB,tempStat->groupC,tempStat->groupD);
		size=strlen(tempStat->disease)+1;
		nsize=htonl(size);
		write(fd,&nsize,sizeof(int));
		write(fd,tempStat->disease,size);
		nsize=htonl(tempStat->groupA);
		write(fd,&nsize,sizeof(int));
		nsize=htonl(tempStat->groupB);
		write(fd,&nsize,sizeof(int));
		nsize=htonl(tempStat->groupC);
		write(fd,&nsize,sizeof(int));
		nsize=htonl(tempStat->groupD);
		write(fd,&nsize,sizeof(int));
			
		tempStat=tempStat->next;
	}
	size=-1;
	nsize=htonl(size);
	write(fd,&nsize,sizeof(int));
	fclose(file);
	while(statsHead!=NULL){
		tempStat=statsHead;
		statsHead=statsHead->next;
		free(tempStat);
	}
	statsHead=NULL;	
}
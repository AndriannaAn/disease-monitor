//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"
#include <time.h>


int compare(char* enDate,char* exDate, patients* next){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if(strcmp(enDate,"\0")==0){
		return -100;
	}
	sscanf(enDate, "%d-%d-%d", &next->entryDate.day,&next->entryDate.month,&next->entryDate.year);
	if(next->entryDate.day<0 || next->entryDate.day>31 ||  next->entryDate.month<1 || next->entryDate.month>12 || next->entryDate.year<0 || next->entryDate.year>(tm.tm_year +1901)){
		return -100;
	}
	char checkPavla;
	int daysOfStay;
	if(strcmp(exDate,"\0")==0){
		return -100;
	}
	sscanf(exDate,"%c",&checkPavla);
	
//	printf("checkpavla %c\n",checkPavla);
	if(checkPavla=='-'){			//arxikopoihsh twn day,month,year se 0 gia to exitDate twn record gia ta opoia den dinetai  
		next->exitDate.day=0;
		next->exitDate.month=0;
		next->exitDate.year=0;
		daysOfStay=0;
		
	}
	else{
		sscanf(exDate, "%d-%d-%d", &next->exitDate.day,&next->exitDate.month,&next->exitDate.year);
		if(next->exitDate.day<0 || next->exitDate.day>31 ||  next->exitDate.month<1 || next->exitDate.month>12 || next->exitDate.year<0 || next->exitDate.year>(tm.tm_year +1901)){
			return -100;
		}
		int daysEn= next->entryDate.day+(next->entryDate.month*30)+(next->entryDate.year*365);
		int daysEx= next->exitDate.day+(next->exitDate.month*30)+(next->exitDate.year*365);
		daysOfStay=daysEx-daysEn;
	}
	if(daysOfStay>=0){
		return 100;
	}
	return  daysOfStay;
}


int nodeCounter(treeNode *root,int counter){
	if(root!=NULL){
		counter++;
		//printf("%s\n",root->patient->id);
		counter=nodeCounter(root->left,counter);
		counter=nodeCounter(root->right,counter);
	}
	return counter;
}
int countDateNodes(treeNode *root,int counter,date start_date,date end_date){
	if(root!=NULL){
		int comparison1=compareNodes(root->patient->entryDate,start_date);
		int comparison2=compareNodes(root->patient->entryDate,end_date);
		if(comparison1<=0 && comparison2 >=0){
			counter++;
			//printf("%s\n",root->patient->id);
			counter=countDateNodes(root->left,counter,start_date,end_date);
			counter=countDateNodes(root->right,counter,start_date,end_date);
		}
		else if(comparison2<0){			//an entry date tou patient megaluterh tou end_date, tote sunexizoume na psaxnoume aristera sto dentro opou oi times einai mikroteres
			counter=countDateNodes(root->left,counter,start_date,end_date);
		}
		else if(comparison1>0){			//antistoixa an entry date mikroterh tou start_date, pigenoume deksia
			counter=countDateNodes(root->right,counter,start_date,end_date);			
		}
	}
	return counter;
}

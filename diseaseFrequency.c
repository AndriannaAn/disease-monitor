//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"


int discharges(char *virus, date start,date end,int diseaseHashtableNumOfEntries, hash_t* hashtable){
	int counter=0,i;
	bucket* temp_bucket;
	entry* temp_entry;
	for(i=0;i<diseaseHashtableNumOfEntries;i++){
		temp_bucket=hashtable->buckets[i];
		while(temp_bucket!=NULL){
			temp_entry=temp_bucket->entries;
			while(temp_entry!=NULL){
				if(strcmp(virus,temp_entry->value)==0){
					counter=exits(temp_entry->root,counter,start,end);	
					//printf("%s %d\n",temp_entry->value,counter);
					return counter;
				}
				temp_entry=temp_entry->next;
			}
			temp_bucket=temp_bucket->next;
		}
	}
	//printf("%s 0\n",virus);		//periptwsh pou to virus den uparxei sta data
	return 0;
}


int numPatientAdmissionsAllCountries(char *virus, date start,date end,int diseaseHashtableNumOfEntries, hash_t* hashtable){
	int counter=0,i;
	bucket* temp_bucket;
	entry* temp_entry;
	for(i=0;i<diseaseHashtableNumOfEntries;i++){
		temp_bucket=hashtable->buckets[i];
		while(temp_bucket!=NULL){
			temp_entry=temp_bucket->entries;
			while(temp_entry!=NULL){
				if(strcmp(virus,temp_entry->value)==0){
					counter=countDateNodes(temp_entry->root,counter,start,end);	
					//printf("%s %d\n",temp_entry->value,counter);
					return counter;
				}
				temp_entry=temp_entry->next;
			}
			temp_bucket=temp_bucket->next;
		}
	}
	//printf("%s 0\n",virus);		//periptwsh pou to virus den uparxei sta data
	return 0;
}


int diseaseFrequency(char *virus,char* country, date start_date, date end_date,int diseaseHashtableNumOfEntries, hash_t* hashtable){
	int counter=0,i;
	bucket* temp_bucket;
	entry* temp_entry;
	for(i=0;i<diseaseHashtableNumOfEntries;i++){
		temp_bucket=hashtable->buckets[i];
		while(temp_bucket!=NULL){
			temp_entry=temp_bucket->entries;
			while(temp_entry!=NULL){	
				if(strcmp(virus,temp_entry->value)==0){
					if(country==NULL){
						counter=countDateNodes(temp_entry->root,counter,start_date,end_date);
					}
					else{
						counter=countCountryDateNodes(temp_entry->root,counter,start_date,end_date,country);
					}
					//printf("%s %d\n",temp_entry->value,counter);
					return counter;
				}
				temp_entry=temp_entry->next;
			}
			temp_bucket=temp_bucket->next;
		}
	}
	//printf("0");		//periptwsh pou to virus den uparxei sta data
	return 0;
}



void countGroupAge(treeNode *root,int *groupA,int *groupB,int *groupC,int *groupD,date start_date,date end_date){
	if(root!=NULL){
		int comparison1=compareNodes(root->patient->entryDate,start_date);
		int comparison2=compareNodes(root->patient->entryDate,end_date);
		if(comparison1<=0 && comparison2 >=0){
			if(root->patient->age<=20){
				(*groupA)+=1;
			}
			else if(root->patient->age<=40){
				(*groupB)+=1;
			}
			else if(root->patient->age<=60){
				(*groupC)+=1;
			}
			else{
				(*groupD)+=1;
			}
			
			countGroupAge(root->left,groupA,groupB,groupC,groupD,start_date,end_date);
			countGroupAge(root->right,groupA,groupB,groupC,groupD,start_date,end_date);
		}
		else if(comparison2<0){			//an entry date tou patient megaluterh tou end_date, tote sunexizoume na psaxnoume aristera sto dentro opou oi times einai mikroteres
			countGroupAge(root->left,groupA,groupB,groupC,groupD,start_date,end_date);
		}
		else if(comparison1>0){			//antistoixa an entry date mikroterh tou start_date, pigenoume deksia
			countGroupAge(root->right,groupA,groupB,groupC,groupD,start_date,end_date);			
		}
	}
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

int countCountryDateNodes(treeNode *root,int counter,date start_date,date end_date,char*value){ 
	if(root!=NULL){
		int comparison1=compareNodes(root->patient->entryDate,start_date);
		int comparison2=compareNodes(root->patient->entryDate,end_date);
		if(comparison1<=0 && comparison2 >=0){
			if(strcmp(root->patient->country,value)==0){		//to counter anevainei mono gia to zhtoumeno country
				counter++;}
			counter=countCountryDateNodes(root->left,counter,start_date,end_date,value);
			counter=countCountryDateNodes(root->right,counter,start_date,end_date,value);
		}
		else if(comparison2<0){
			counter=countCountryDateNodes(root->left,counter,start_date,end_date,value);
		}
		else if(comparison1>0){
			counter=countCountryDateNodes(root->right,counter,start_date,end_date,value);			
		}
	}
	return counter;
}




int exits(treeNode *root, int counter ,date start_date, date end_date){
	if(root!=NULL){
		int comparison1=compareNodes(root->patient->exitDate,start_date);
		int comparison2=compareNodes(root->patient->exitDate,end_date);
		if(comparison1<=0 && comparison2 >=0){
			counter++;
		}
		
		counter=exits(root->left,counter,start_date,end_date);
		if(compareNodes(root->patient->entryDate,end_date)>0){
			counter=exits(root->right,counter,start_date,end_date);
		}
	}
	return counter;
}
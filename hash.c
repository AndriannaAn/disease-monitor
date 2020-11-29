//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"

unsigned int hash ( char* key ,int buckets){
	int j,a=1031,p=13499;
	unsigned int hash=0;
	for(j=0;j<strlen(key);j++){			//Universal hash for strings
		hash=(hash*a+key[j])%p;
	}
	hash=hash%buckets;
	return hash;
}

hash_t *hash_t_create(int size) {
    
	hash_t* hashtable;
	hashtable=malloc(sizeof(hash_t));
    hashtable->buckets = malloc(sizeof(bucket*) * size);

    int i = 0;
    for (; i < size; ++i) {
		hashtable->buckets[i]=NULL;
    }

    return hashtable;
}
void insert_to_hash(hash_t *hashtable,int numOfEntries,patients* temp, int num,char* val_entry){
			int flag;
			entry* new_entry;
			new_entry=malloc(sizeof(entry));
			new_entry->value=val_entry;
			new_entry->next=NULL;
			new_entry->root=NULL;
			if(hashtable->buckets[num]==NULL){
				bucket* new_bucket;
				new_bucket=malloc(sizeof(bucket));
				new_bucket->counter=1;
				new_bucket->entries=new_entry;
				new_bucket->entries->root=insertNode(new_bucket->entries->root,temp);
				new_bucket->next=NULL;
				hashtable->buckets[num]=new_bucket;
			}
			else{
				int flag2=0;
				bucket* temp_bucket=hashtable->buckets[num], *bucket_last;
				
				while(temp_bucket!=NULL){					
					flag=0;
					entry* temp_entry=temp_bucket->entries;
					while(temp_entry!=NULL){
						if(strcmp(temp_entry->value,new_entry->value)==0){
							flag=1;
							free(new_entry);
						//	printf("flagged1\n");
							break;
						}
						temp_entry=temp_entry->next; // 
					}
					if (flag==1){		//to val tou entry uparxei, opote apla prepei na mpei sto dentro
						flag2=1;
					//	printf("flagged2\n");
						//insert new_entry to temp_entry->tree
						temp_entry->root=insertNode(temp_entry->root,temp);

						break;
					}
					else{
						bucket_last=temp_bucket;
						temp_bucket=temp_bucket->next;
					}
				}
				
				if(flag2==0){	//h hashed value den uparxei sto hashtable, opote ginetai insert sto hashtable kai sto dentro
					if((bucket_last->counter+1)<=numOfEntries){
					//	printf(" there is space in the bucket %s\n",temp->id);
						entry* temp_entry=bucket_last->entries;
						while(temp_entry->next!=NULL){
							temp_entry=temp_entry->next;
						}
						temp_entry->next=new_entry;
						temp_entry->next->root=insertNode(temp_entry->next->root,temp);

						bucket_last->counter++;
					}
					else{
						//printf(" no space in existant buckets, new bucket needed %s\n",temp->id);
						bucket* new_bucket;
						new_bucket=malloc(sizeof(bucket));
						new_bucket->counter=1;
						new_bucket->entries=new_entry;
						new_bucket->entries->root=insertNode(new_bucket->entries->root,temp);

						new_bucket->next=NULL;
						bucket_last->next=new_bucket;
					}
				}
			}
	
	return;
}
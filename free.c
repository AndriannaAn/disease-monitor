//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"


void free_hash(int size, hash_t* hashtable){
	int i;
	for(i=0;i<size;i++){
		bucket* temp_bucket=hashtable->buckets[i];
		while(temp_bucket!=NULL){
			entry* temp_entry=temp_bucket->entries;
			while(temp_entry!=NULL){
				entry* tmp=temp_entry;
				temp_entry=temp_entry->next;
				free_node(tmp->root);
				free(tmp);
			}
			bucket* tmpb=temp_bucket;
			temp_bucket=temp_bucket->next;
			free(tmpb);
		}
	}
	free(hashtable->buckets);
	free(hashtable);
}

void free_node(treeNode* node){
	if(node!=NULL){
		free_node(node->left);
		free_node(node->right);
		free(node);
	}
}
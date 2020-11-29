//Andrianna Anastasopoulou 
//sdi1300009
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "entries.h"
#include <time.h>
int maxHeight(int heightA, int heightB){
	if(heightA>heightB){
		return heightA; 
	}
	else{
		return heightB;
	}
}

int compareNodes(date A,date B){
	int comparison;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if(A.day<1 || A.day>31 || A.month<1 || A.month>12 || A.year<0 || A.year>(tm.tm_year +1901) || B.day<1 || B.day>31 || B.month<1 || B.month>12 || B.year<0 || B.year>(tm.tm_year +1901)){
		return -100;
	}
	comparison=(B.day+(B.month*30)+(B.year*365))-(A.day+(A.month*30)+(A.year*365));
	return comparison;
	}


treeNode *rotateRight(treeNode *node)
{
	int a,a1,b,b1;
    treeNode *lchild = node->left;
	treeNode *temp = lchild->right;
	lchild->right = node;
    node->left = temp;
	if(node->left==NULL){
		a=0;		
	}
	else{
		a=node->left->height;
	}
	if(node->right==NULL){
		b=0;
	}
	else{
		b=node->right->height;
	}

  
   
    node->height = maxHeight(a,b)+1;
	
	
	if(lchild->left==NULL){
		a1=0;
	}
	else{
		a1=lchild->left->height;
	}
	if(lchild->right==NULL){
		b1=0;
		
	}
	else{
		b1=lchild->right->height;
	}
    lchild->height = maxHeight(a1, b1)+1;


    return lchild;
}

struct treeNode *rotateLeft(treeNode *node){	
	int a,a1,b,b1;
	treeNode *rchild = node->right;
	treeNode *tmp = rchild->left;
	rchild->left = node;
	node->right = tmp;
	
	if(node->left==NULL){
		a1=0;	
	}
	else{
		a1=node->left->height;
	}
	if(node->right==NULL){
		b1=0;
		
	}
	else{
		b1=node->right->height;
	}
    
	node->height = maxHeight(a1, b1)+1;
	
	if(rchild->left==NULL){
		a=0;	
	}
	else{
		a=rchild->left->height;
	}
	if(rchild->right==NULL){
		b=0;
	}
	else{
		b=rchild->right->height;
	}

	rchild->height = maxHeight(a, b)+1;

    return rchild;
}

treeNode* insertNode(treeNode* node,patients* patient){
	if (node==NULL){
		treeNode* new;
		new= malloc(sizeof(treeNode));
		new->patient=patient;
		new->left=NULL;
		new->right=NULL;
		new->height=1;
		return new;
	}
	date patient_date=patient->entryDate;
	date node_date =node->patient->entryDate;
	int comparison;
	comparison=compareNodes(patient_date,node_date);
	if(comparison>0){ //if  patient's entry date< node's date go left
		node->left=insertNode(node->left,patient);
		//printf("left\n");
	}
	else{
		node->right=insertNode(node->right,patient);
		//printf("right\n");
	}
		
 	int a,b;
	date left_date;
	date right_date;
	if(node->left==NULL){
		a=0;
		left_date.day=0;
		left_date.month=0;
		left_date.year=0;
	}
	else{
		a=node->left->height;
		left_date=node->left->patient->entryDate;
	}
	if(node->right==NULL){
		b=0;
		right_date.day=0;
		right_date.month=0;
		right_date.year=0;
	}
	else{
		b=node->right->height;
		right_date=node->right->patient->entryDate;
	}
	node->height=maxHeight(a,b)+1;
	

	int height_difference=a-b;

	if(height_difference > 1){
		if(compareNodes(patient_date,left_date)<0){
			//printf("left right\n");
			node->left = rotateLeft(node->left);
			return rotateRight(node);
		}
		else{
			//printf("right right\n");
			return rotateRight(node);
		}
	}
	else if(height_difference < -1){
		if(compareNodes(patient_date,right_date)>0){
			//printf("right left\n");
			node->right = rotateRight(node->right);
			return rotateLeft(node);
		}
		else{
			//printf("left left\n");
			return rotateLeft(node);
		}
	}

	return node;
}
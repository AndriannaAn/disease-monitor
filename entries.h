//Andrianna Anastasopoulou 
//sdi1300009
#ifndef ENTRIES_H
#define ENTRIES_H
#include <stdio.h>

typedef struct sortgroups{
	int percentage;
	char group[6];
}sortgroups;

typedef struct ageStats{
	char disease[32];
	int groupA;
	int groupB;
	int groupC;
	int groupD;
	struct ageStats* next;
}ageStats;

typedef struct date{
	int day;
	int month;
	int year;
}date;
typedef struct countryInfo{
	char country[32];
	int info;
	struct countryInfo *next;
}countryInfo;


typedef struct dirlist{
	char dir[50];
	struct dirlist* next;
	char **filelist;
	int fileCount;
	struct hash_t* diseaseHash;
	struct patients *recordList;
}dirlist;

typedef struct patients{
	char id[32];
	char firstName[32] ;
	char lastName[32] ;
	char disease[32] ;
	char country[32] ;
	date entryDate;
	date exitDate;
	struct patients *next;
	int age;
}patients;

typedef struct treeNode
{
    struct patients *patient;
    struct treeNode *left;
    struct treeNode *right;
    int height;
}treeNode;

typedef struct entry{
	char *value;
	struct treeNode *root;
	struct entry* next;
}entry;

typedef struct bucket{
	entry* entries;
	struct bucket* next;
	int counter;
}bucket;
typedef struct hash_t{
	 bucket** buckets;
	
}hash_t;


typedef struct heap_node{
	int key;
	char* value;
	struct heap_node *parent;
	struct heap_node *left;
	struct heap_node *right;
}heap_node;
typedef struct build_heap{
	int counter;
	char* value;
	struct build_heap *next;
}build_heap;
typedef struct order{
	int value;
	struct order* next;
}order;


int maxHeight(int a, int b);
int compareNodes(date A,date B);
treeNode* insertNode(treeNode* node,struct patients* patient);

unsigned int hash ( char* key ,int buckets);
hash_t *hash_t_create(int size);
void insert_to_hash(hash_t *hashtable,int numOfEntries,struct patients* temp, int num,char* val_entry);

int nodeCounter(struct treeNode *root,int counter);

int countDateNodes(treeNode *root,int counter,date start_date,date end_date);
int diseaseFrequency(char *virus,char* disease, date start,date end,int diseaseHashtableNumOfEntries, hash_t* hashtable);
int countCountryDateNodes(treeNode *root,int counter,date start_date,date end_date,char*value);
void printDateCounters(int size, hash_t *hashtableDisease,char* arg1,char*arg2);
void free_node(treeNode* node);
void free_hash(int size, hash_t* hashtable);


date string_to_date(char* strdate);
void updateRecord(patients *head,char *id,date exitDate);
hash_t* readDirectory(int diseaseHashtableNumOfEntries,int bucketSize, char country[32] ,char *dir, char **matrix , int count , dirlist* curDir,int fd,unsigned short port);
int numPatientAdmissionsAllCountries(char *virus , date start,date end,int diseaseHashtableNumOfEntries, hash_t* hashtable);

int discharges(char *virus, date start,date end,int diseaseHashtableNumOfEntries, hash_t* hashtable);
int exits(treeNode *root, int counter,date start_date, date end_date);

void countGroupAge(treeNode *root,int *groupA,int *groupB,int *groupC,int *groupD,date start_date,date end_date);

void readfile(dirlist* curDir, char *country,char *dir, char *datefile,int diseaseHashtableNumOfEntries, hash_t* hashtableDisease,int fd);

void quicksort(char **start1,int size);

#endif
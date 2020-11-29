//Andrianna Anastasopoulou 
//sdi1300009
#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>

typedef struct serverCountryInfo{
	char country[32];
	short workerPort;
	struct sockaddr_in workerIp;
	struct serverCountryInfo *next;
}serverCountryInfo;

#endif
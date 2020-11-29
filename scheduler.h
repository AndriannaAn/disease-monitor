#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>


typedef struct Job{
	void*(*routine)(void**);
	void *arg;
}Job;

typedef struct threadStarting{
	struct JobScheduler *scheduler;
	struct serverCountryInfo *workerList;
}threadStarting;

typedef struct workerArguments{
	int fd;
	struct serverCountryInfo *countryInfoList;
	pthread_mutex_t *mutex_workerUpdate;
	struct sockaddr_in *sa;
}workerArguments;

typedef struct clientArguments{
	int fd;
	struct serverCountryInfo *countryInfoList;
}clientArguments;


typedef struct JobScheduler{
	struct Job *queue;
	int num_of_threads;
	int schedulerSize;
	int head;
	int tail;
	int queuedjobs;
	int stop;
	int working;
	int barrier;
	pthread_mutex_t mutex_scheduler;
	pthread_mutex_t mutex_workerUpdate;
	pthread_cond_t cond_scheduler;
	pthread_t *id;
}JobScheduler;


JobScheduler* Init(int num_of_threads,int bufferSize);

void Destroy(JobScheduler* scheduler);

void Barrier(JobScheduler* scheduler);

void Schedule(Job job, JobScheduler* scheduler);

void Stop(JobScheduler* scheduler);

void* scheduler_thread(void* sch);

void handle_client(clientArguments *args);

void handle_worker(workerArguments *args);

#endif
#include "entries.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>	
#include "server.h"


JobScheduler* Init(int num_of_threads,int bufferSize){
	int i;
	JobScheduler *scheduler;
	scheduler=malloc(sizeof(JobScheduler));
	scheduler->num_of_threads=num_of_threads;
	scheduler->schedulerSize=bufferSize;
	scheduler->head=0;
	scheduler->tail=0;
	scheduler->queuedjobs=0;
	scheduler->stop=0;
	scheduler->working=num_of_threads;
	scheduler->barrier=0;
	scheduler->queue=malloc(sizeof(Job)*scheduler->schedulerSize);
	pthread_mutex_init(&(scheduler->mutex_scheduler), NULL);
	pthread_mutex_init(&(scheduler->mutex_workerUpdate), NULL);
	pthread_cond_init(&(scheduler->cond_scheduler),NULL);

	scheduler->id=malloc(sizeof(pthread_t)*num_of_threads);
	for(i=0;i<num_of_threads;i++){
		pthread_create(&(scheduler->id[i]), NULL, scheduler_thread, (void*)scheduler);
	}
	return scheduler;
}



void Destroy( JobScheduler* scheduler){
	pthread_mutex_destroy(&(scheduler->mutex_scheduler));
	pthread_mutex_destroy(&(scheduler->mutex_workerUpdate));
	pthread_cond_destroy(&(scheduler->cond_scheduler));
	free(scheduler->queue);
	free(scheduler->id);
	free(scheduler);
}



void Barrier(JobScheduler* scheduler){
	pthread_mutex_lock(&(scheduler->mutex_scheduler));
	scheduler->barrier=1;
	while(scheduler->working>0 || scheduler->queuedjobs>0){
		pthread_cond_wait(&(scheduler->cond_scheduler),&(scheduler->mutex_scheduler));
	}
	scheduler->barrier=0;
	pthread_mutex_unlock(&(scheduler->mutex_scheduler));
}



void Stop(JobScheduler* scheduler){
	int i;
	pthread_mutex_lock(&(scheduler->mutex_scheduler));
	scheduler->stop=1;
	pthread_cond_broadcast(&(scheduler->cond_scheduler));
	pthread_mutex_unlock(&(scheduler->mutex_scheduler));
	for(i=0;i<scheduler->num_of_threads;i++){
		pthread_join(scheduler->id[i], NULL);
		//printf("thread %ld finished\n" ,scheduler->id[i]);
	}
}


void Schedule(Job job, JobScheduler* scheduler){
	pthread_mutex_lock(&(scheduler->mutex_scheduler));
	while(scheduler->queuedjobs==scheduler->schedulerSize ){ //|| scheduler->barrier==1
		pthread_cond_wait(&(scheduler->cond_scheduler),&(scheduler->mutex_scheduler));
	}
	scheduler->queue[scheduler->tail]=job;
	scheduler->tail=(scheduler->tail+1)%scheduler->schedulerSize;
	scheduler->queuedjobs++;
	pthread_cond_broadcast(&(scheduler->cond_scheduler));
	pthread_mutex_unlock(&(scheduler->mutex_scheduler));
}





void* scheduler_thread(void* sch){
	JobScheduler* scheduler= sch;
	Job threadsjob;
	while(1){
		pthread_mutex_lock(&(scheduler->mutex_scheduler));
		scheduler->working--;
		while(scheduler->queuedjobs==0 && scheduler->stop==0){
			if(scheduler->working==0 && scheduler->barrier==1){
				pthread_cond_broadcast(&(scheduler->cond_scheduler));
			}
			pthread_cond_wait(&(scheduler->cond_scheduler),&(scheduler->mutex_scheduler));
		}
		if(scheduler->stop==1){
			pthread_mutex_unlock(&(scheduler->mutex_scheduler));
   			pthread_exit(NULL);
   			return NULL;
		}
		threadsjob.routine=scheduler->queue[scheduler->head].routine;
		threadsjob.arg=scheduler->queue[scheduler->head].arg;
		scheduler->head = (scheduler->head+1)%scheduler->schedulerSize;
		scheduler->queuedjobs-=1;
		scheduler->working++;
		pthread_mutex_unlock(&(scheduler->mutex_scheduler));
		(*(threadsjob.routine))(threadsjob.arg);
		free(threadsjob.arg);
	}
}
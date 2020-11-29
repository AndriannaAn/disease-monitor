CC = gcc
CFLAGS = -pthread -g -Wall
OBJ1 = worker.o avltree.o diseaseFrequency.o free.o hash.o readFiles.o
OBJ2 = whoServer.o diseaseFrequency.o readFiles.o hash.o avltree.o scheduler.o handle_client.o handle_worker.o
OBJ3 = master.o avltree.o diseaseFrequency.o free.o hash.o readFiles.o
HEADERS = entries.h

all: worker whoServer whoClient master

master: $(OBJ3)
	$(CC) $(CFLAGS) $(OBJ3) -o master

worker: $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o worker

whoServer: $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o whoServer

whoClient: whoClient.o
	$(CC) $(CFLAGS) whoClient.o -o whoClient

avltree.o: avltree.c $(HEADERS)
	$(CC) $(CFLAGS) -c avltree.c -o avltree.o

diseaseFrequency.o: diseaseFrequency.c $(HEADERS)
	$(CC) $(CFLAGS) -c diseaseFrequency.c -o diseaseFrequency.o

free.o: free.c $(HEADERS)
	$(CC) $(CFLAGS) -c free.c -o free.o

hash.o: hash.c $(HEADERS)
	$(CC) $(CFLAGS) -c hash.c -o hash.o
	
readFiles.o: readFiles.c $(HEADERS)
	$(CC) $(CFLAGS) -c readFiles.c -o readFiles.o

scheduler.o: scheduler.c $(HEADERS) scheduler.h
	$(CC) $(CFLAGS) -c scheduler.c -o scheduler.o

handle_client.o: handle_client.c $(HEADERS) scheduler.h server.h
	$(CC) $(CFLAGS) -c handle_client.c -o handle_client.o

handle_worker.o: handle_worker.c $(HEADERS) scheduler.h server.h
	$(CC) $(CFLAGS) -c handle_worker.c -o handle_worker.o

clean:
	-rm -f $(OBJ1) $(OBJ2) $(OBJ3) whoClient.o master whoServer worker whoClient
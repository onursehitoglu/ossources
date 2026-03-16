/*! \file messqsrv.c
	\descr Message queue example simulates an airport control
           A flight request, arrival or departure is read
           from message queue (with arrivals priority) and
		   runway is given to the flight for a random time
*/
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#define RAND(i) (random()%(i))
#define NELS(i) (sizeof(i)/sizeof(*i))

#define APPPATH "/etc/passwd"

#define LANDING 10L
#define TAKINGOFF 20L

struct Request {
	long type;
	int no;
	char name[30];
	char city[30];
	char model[20];
};

void printrequest(struct Request *r) {
	printf("Airline: %s, No: %d, Model: %s\n",
		r->name,r->no,r->model);
	if (r->type==LANDING) 
		printf("Coming from %s",r->city);
	else 
		printf("Departing to %s",r->city);
	printf(" is given the runway\n\n");
}

int fetchrequest(int key,struct Request *r) {
	int tmp;

	/* -TAKINGOFF picks the smallest type <= TAKINGOFF
       giving priority to LANDING requests since 
       LANDING < TAKINGOFF
		replace it by 0 to any request, FIFO
		replace it by LANDING for only landing requests
    */
	tmp=msgrcv(key,r,sizeof(struct Request),-TAKINGOFF,0);
	if (tmp<0) {
			return 0;
	}
	return 1;
}

int main() {
	struct Request req;
	long queueid,key;
	queueid=ftok(APPPATH,0);
	printf("opening/creating %lx\n",queueid);
	if ((key=msgget(queueid,0664|IPC_CREAT))<0) {
		perror("opening/creating queue");
		exit(1);
	}
	while (1) {
		if (fetchrequest(key,&req))
			printrequest(&req);
		sleep(RAND(10));
	}
}

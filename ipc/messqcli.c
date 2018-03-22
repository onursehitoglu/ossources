/*! \file messqcli.c
    \desc Message queue client simulates arrival of plane
          requests to an airport control. Inserts
          random flight data on message queue
*/
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

#define RAND(i) (random()%(i))
#define NELS(i) (sizeof(i)/sizeof(*i))

#define APPPATH "/etc/passwd"
#define LANDING 10L
#define TAKINGOFF 20L

long types[]={LANDING,TAKINGOFF};
char planes[][30]={"THY","Airbus","Alitalia","SwissAir","KLM","British"};
char cities[][30]={"Trabzon","Istanbul","Munich","Paris","Londra","Roma","Prag","Atina"};
char models[][20]={"A320","B747","B737","DG10","A310","B757"};

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
		printf("Coming from %s\n\n",r->city);
	else 
		printf("Departing to %s\n\n",r->city);
}

void randomrequest(struct Request *r) {
	int tmp;
	r->type=types[RAND(NELS(types))];
	strncpy(r->name,planes[RAND(NELS(planes))],30);
	strncpy(r->city,cities[RAND(NELS(cities))],30);
	strncpy(r->model,models[RAND(NELS(models))],20);
	r->no=RAND(1000);
}

int main() {
	struct Request req;
	long queueid,key;

	srand(getpid() + time(NULL));
	queueid=ftok(APPPATH,0);
	if ((key=msgget(queueid,0664))<0) {
		perror("opening queue");
		exit(1);
	}
	while (1) {
		randomrequest(&req);
		printrequest(&req);
		msgsnd(key,&req,sizeof(struct Request),0);
		sleep(RAND(10));
	}
}

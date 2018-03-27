#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/signal.h>
#include<sys/time.h>
int semkeys[5]={0,0,0,0,0};
const struct sembuf  up = {0,1,SEM_UNDO};
const struct sembuf  down = {0,-1,SEM_UNDO};
/* deadlock example with n semaphores, vulnarable to deadlock */

void clearandexit(int r)
{
	int i;
	for (i=0;i<5;i++)
		semctl(semkeys[i],1,IPC_RMID);
	exit(r);
}

void philosopher(int i,int s1, int s2) 
{
	struct sembuf sb1, sb2;
	srand(i+time(NULL));
	while (1) {
		sleep(rand()%3);
		sb1=down; semop(s1,&sb1,1);
		sb2=down; semop(s2,&sb2,1);
		printf("Philosopher %d is eating\n",i);
		sleep(rand()%10);
		printf("Philosopher %d finished eating\n",i);
		sb1=up; semop(s1,&sb1,1);
		sb2=up; semop(s2,&sb2,1);
	}
}
int main() 
{
	int i,n;
	struct sembuf sb;
	for (i=0;i<5;i++) {
		semkeys[i]=semget(IPC_PRIVATE,1,IPC_CREAT|0600);
		if (semkeys[i]<=0) {
			perror("Creating semaphore");
			clearandexit(-2);
		}
		sb=up; 
		semop(semkeys[i],&sb,1);
	}
	for (i=0;i<5;i++) {
		if (!fork()) {
			philosopher(i,semkeys[i],semkeys[(i+1)%5]);
			return 0;
		}
	}
	signal(SIGINT,clearandexit); sleep(1000);
	clearandexit(0);
}

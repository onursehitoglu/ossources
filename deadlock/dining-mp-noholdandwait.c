#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/signal.h>
#include<sys/time.h>
/*! Multiprocess dining philosopher example
    avoding deadlock by using XSI (former System V) 
    IPC, semaphore sets. Philosophers can block
    on multiple chopsticks at once, breaking
    hold and wait precondition, thus avoiding
    deadlock 
*/

int semkey;
const struct sembuf  up = {0,1,SEM_UNDO};
const struct sembuf  down = {0,-1,SEM_UNDO};

void clearandexit(int r)
{
	int i;
	semctl(semkey,5,IPC_RMID);
	exit(r);
}

void philosopher(int i,int s1, int s2) 
{
	struct sembuf sb[2];
	srand(i+time(NULL));
	while (1) {
		sleep(rand()%2+1);
		sb[0]=down;sb[1]=down;
		sb[0].sem_num=s1; sb[1].sem_num=s2;
        /* read the following as wait({s1,s2}) , wait on sem. set*/
		semop(semkey,sb,2);     /* block on both, atomically */
		printf("Philosopher %d is eating\n",i);
		sleep(rand()%4+1);
		printf("Philosopher %d finished eating\n",i);
		sb[0]=up;sb[1]=up;
		sb[0].sem_num=s1; sb[1].sem_num=s2;
		semop(semkey,sb,2);
	}
}
int main() 
{
	int i,n;
	struct sembuf sb[5];
	
	semkey=semget(IPC_PRIVATE,5,IPC_CREAT|0600);
	if (semkey<=0) {
		perror("Creating semaphore");
		clearandexit(-2);
	}
	for (i=0;i<5;i++) {
		sb[i].sem_num=i; sb[i].sem_op=1;
		sb[i].sem_flg=SEM_UNDO;
	}
	semop(semkey,sb,5);

	for (i=0;i<5;i++) {
		if (!fork()) {
			philosopher(i,i,(i+1)%5);
			return 0;
		}
	}
	signal(SIGINT,clearandexit); sleep(1000);
	clearandexit(0);
}

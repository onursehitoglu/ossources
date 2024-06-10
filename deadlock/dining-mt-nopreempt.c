#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

pthread_mutex_t forks[5];

void *philosopher(void *ip)
{
	pthread_mutex_t *left,*right;
	int i=*(int *)ip;
	int z;
    struct timespec timeout = { 1, 0}; // 1 second, 0 nano seconds

	printf("philosopher %d thread id %u started\n", i,
  			pthread_self()); 
	left=i==0?forks+4:forks+(i-1);
	right=forks+i;
	srand(i+time(NULL));
	for (z=0;z<100000;z++) {
//		usleep(rand()%10);
        while (1) {
		    pthread_mutex_lock(left);
            // trylock is non-blocking version, returns error if not available
            // timelock is timeout version, first one makes busy wait, expensive
		    /*if (pthread_mutex_trylock(right))  // fail */
		    if (pthread_mutex_timedlock(right, &timeout))  { // fail */ 
                pthread_mutex_unlock(left);
				printf("%d waited enough\n", i);
            } else
                break;  // we have both mutexes

        }
		printf("Philosopher %d is eating %d\n",i,z);
		//usleep(10000); //rand()%30000+10000);
		printf("Philosopher %d finished eating %d\n",i,z);
		pthread_mutex_unlock(left);
		pthread_mutex_unlock(right);
	}
    return NULL;
}

int main() 
{
	pthread_t phils[5];
	int phids[5]={0,1,2,3,4};
	int i,n;
	
	for (i=0;i<5;i++) {
		if (pthread_mutex_init(&(forks[i]),NULL)) {
			perror("Creating mutex");
			return(-1);
		}
	}
	for (i=0;i<5;i++) {
		if (pthread_create(&(phils[i]), NULL, 
			philosopher, (void *) &(phids[i])) )
			perror("Create failed");
	}
	for (i=0;i<5;i++) {
		pthread_join(phils[i],NULL);
	}
	sleep(1);
}

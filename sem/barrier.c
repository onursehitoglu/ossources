#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>


#define WAIT 150000

sem_t *sbarrier, *smutex;
int count = 0;
int nthreads = 0;

void waitbarrier()
{
	sem_wait(smutex);
	count++;
	if (count == nthreads) { 
	    sem_post(smutex); /* release mutex */
		printf("let the games begin\n");
		sem_post(sbarrier); /* unblock one thread */
	} else {
	    sem_post(smutex); /* release mutex */
		sem_wait(sbarrier); /* each unblocking thread unblock one other */
		sem_post(sbarrier);
	}
/*  ALTERNATIVE VERSION: signal n-1 times
	if (count == nthreads) {
	    sem_post(smutex);
		printf("let the games begin\n");
        for (int i = 0; i < nthreads-1; i++)
		    sem_post(sbarrier);
	} else {
	    sem_post(smutex);
		sem_wait(sbarrier); // no unblock here
	}
*/
}



void *barriertest(void *p) 
{
	int i;
	char *name = (char *) p;


	/* wait for a while */
	usleep((rand() % 15+1) * WAIT ); 
	printf("%s ready for waitbarrier\n", name);
	waitbarrier();
	printf("%s starts\n", name);
	usleep(5*WAIT);
	printf("%s exits\n", name);
	return 0;

}



int main() {
	pthread_t ptr,ctr;
	char *names[] = { "A", "B", "C", "D", "E","F","G"};
	pthread_t tids[4];

	int i;

    srand(time(NULL));

	sem_unlink("barrier_sem_barrier");
	sem_unlink("barrier_sem_mutex");
	/* initially barrier is not reached */
	sbarrier = sem_open("barrier_sem_barrier",O_CREAT,0600,0);
	if (sbarrier == SEM_FAILED) {
		perror("sfull");
		return -1;
	}
	smutex = sem_open("barrier_sem_mutex",O_CREAT,0600,1);

	nthreads = sizeof(names)/sizeof(names[0]);
	for (i = 0 ; i < nthreads; i++) 
		pthread_create(tids+i, NULL, barriertest, (void *) names[i]);

	for (i = 0 ; i < nthreads; i++) 
		pthread_join(tids[i], NULL);

	return 0;
}

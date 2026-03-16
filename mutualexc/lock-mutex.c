#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sched.h>

#define NTHREADS 10


void lock(pthread_mutex_t *lockvar) {
	pthread_mutex_lock(lockvar);
}


void unlock(pthread_mutex_t *lockvar) {
	pthread_mutex_unlock(lockvar);
}



void lock(pthread_mutex_t *lockvar);
void unlock(pthread_mutex_t *lockvar);

int x;
pthread_mutex_t lockvar = PTHREAD_MUTEX_INITIALIZER;

void *increment(void *p)
{
	int n = * (int *)p;
	int i;
	for (i=0; i<1000000; i++) {
		lock(&lockvar);
		x = x + 1;
		unlock(&lockvar);
	}
	printf("%ld finished\n", pthread_self());
}

int main() {
	int i;
	int p[] = {0,1,2,3,4,5};

	pthread_t t[NTHREADS];
	for (i=0; i < NTHREADS; i++) {
		pthread_create(&t[i], NULL, increment, (void *) &p[i]);
	}
	for (i=0; i < NTHREADS; i++) {
		pthread_join(t[i], NULL);
	}
	printf("%d\n", x);
	return 0;
}




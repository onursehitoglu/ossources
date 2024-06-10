#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<sched.h>

#define INCR 10000000

int cmpxchg(int *ptr, int old, int new) {
	/* compare eax (old) to *ptr
	   if same, move (new) value to *ptr
           else move *ptr to eax (old)
		read as "+a": old <-> EAX
                "+m": %ptr is %1 as rw memory
                "r": new, any register as %2 readonly
		expands: EAX = old; cmpxchgl new, *ptr ; old = EAX
	*/
		
	asm volatile("lock; cmpxchgl %2,%1"         \
			: "+a" (old), "+m" (*ptr)   \
			: "r" (new)		    \
			: "memory");         
	return old;
}

/* shorter version, uses gcc builtin */
/*
#define cmpxchg(P,O,N) __sync_val_compare_and_swap((P), (O), (N))
*/

void lock(int *lockvar) {
	/* while lockvars value is 1 
	   spinlock, until lockvar is 0,
	   then atomically set it to 1 */
	while (cmpxchg(lockvar, 0, 1)) 
		/*  nothing */ ;

}


void unlock(int *lockvar) {
	*lockvar = 0;
}



void lock(int *lockvar);
void unlock(int *lockvar);

int x;

int lockvar;

void *increment(void *p)
{
	int n = * (int *)p;
	int i;
	for (i=0; i < INCR; i++) {
		lock(&lockvar);
		x = x + 1;
		unlock(&lockvar);
	}
	printf("%d finished\n", n);
}

int main(int argc, char *argv[]) {
	int i;
	int *p;
	pthread_t *t;
	int nthreads;

	lockvar = 0;
	
	if (argc < 2)
		nthreads = 2;
	else {
		nthreads = atoi(argv[1]);
		if (nthreads < 2)
			nthreads = 2;
	}

	p = malloc(sizeof(int)*nthreads);
	t = malloc(sizeof(pthread_t)*nthreads);
	/*
	printf("ret: %d ", cmpxchg(&lockvar, 0,1));
	printf("lockvar: %d\n", lockvar);


	return 0;
	*/
	for (i=0; i < nthreads; i++) {
		p[i] = i;
		pthread_create(&t[i], NULL, increment, (void *) &p[i]);
	}
	for (i=0; i < nthreads; i++) {
		pthread_join(t[i], NULL);
	}
	free(p); free(t);
	printf("Expected: %d, Result: %d\n", nthreads*INCR, x);
	return 0;
}




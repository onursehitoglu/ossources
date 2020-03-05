#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sched.h>
/*! Filter algorithm for mutual exclusion
    Generalized version of Peterson's, for more than 2 threads
*/

#define NTHREADS 5 

void filterlock(int id);
void filterunlock(int id);

int x;

void *increment(void *p)
{
	int n = * (int *)p;
	int i;
	for (i=0; i<1000000; i++) {
		filterlock(n);
		x = x + 1;
		filterunlock(n);
	}
	printf("%u finished\n", pthread_self());
}

int level[NTHREADS];
int lasttoenter[NTHREADS];


int main() {
	int i;
	int p[NTHREADS];
	for (i = 0; i < NTHREADS; i++) {
		p[i] = i;
		level[i] = -1;
	}

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

/* There are N-1 waiting rooms for the lock
   level[id] keeps the thread id's current
   level. Each thread starts from level[0]
   up to level[N-1]. In next loop, it enters
   the critical region.

   A thread waits in level[i] as long as
   there is another thread in a higher
   level or another thread arrives at current
   level.
*/

void filterlock(int id) {
	int i;
	for (i = 0; i < NTHREADS; i++) {
		int wait = 1;
		level[id] = i; 			// mark level of id 
		lasttoenter[i] = id; 	// mark id arrived at the level
		asm("mfence");
		while (wait) {	// wait until ...
			int k, f  = 0;
			// another level arrived at level i
			if (lasttoenter[i] != id) {
				break;
			}
			wait = 0;
			// or no other thread left in a higher level
			for (k = 0; k < NTHREADS; k++) {
				if (k == id)	continue;
				if (level[k] >= i) {
					wait = 1;
					break;
				}
			}
		}
		// now ready to skip to next waiting room
	}
	// levels[i] i < N-1 are all full or no other thread left in levels[N-1] 
	// can enter critical region
}

void filterunlock(int id) {
	level[id] = -1;		// mark yourself out of any levels
}


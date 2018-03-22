#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<sched.h>

#define NTHREADS 2 

void plock(int id);
void punlock(int id);

int x;

void *increment(void *p)
{
	int n = * (int *)p;
	int i;
	for (i=0; i<1000000; i++) {
		plock(n);
		x = x + 1;
		punlock(n);
	}
	printf("%u finished\n", pthread_self());
}

int main() {
	int i;
	int p[] = {0,1};
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

int turn = 0;
int flag[] = { 0, 0};

int dummy;

void plock(int id) {
	int other = !id;
	
	flag[id] = 1;
	turn = other;
	//asm ("mfence");	// put a memory fence. instruction order is forced here
	while (flag[other] && turn == other )  {
	};
}

void punlock(int id) {
	flag[id] = 0;
}


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>


#define WAIT 100000

pthread_cond_t allhere = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int count = 0;
int nthreads = 0;

void barrierwait()
{
    pthread_mutex_lock(&mutex);
    count++;
    /* BROADCAST SIGNAL VERSION */
    /*
    if (count == nthreads) {
        pthread_cond_broadcast(&allhere);
        printf("let the games begin\n");
    }

    if (count < nthreads)
        pthread_cond_wait(&allhere, &mutex);
    */
    /* SINGLE SIGNAL VERSION */
    if (count == nthreads) {
        pthread_cond_signal(&allhere);
        printf("let the games begin\n");
    }

    if (count < nthreads) {
        pthread_cond_wait(&allhere, &mutex);
        // wake up another 
        pthread_cond_signal(&allhere);
    }
    pthread_mutex_unlock(&mutex);
}



void *barriertest(void *p) 
{
    int i;
    char *name = (char *) p;

    /* wait for a while */
    usleep((rand() % 15+1) * WAIT ); 
    printf("%s ready for barrierwait\n", name);
    barrierwait();
    printf("%s starts\n", name);
    usleep(5*WAIT);
    return 0;

}



int main() {
    pthread_t ptr,ctr;
    char *names[] = { "A", "B", "C", "D","E","F","G","H"};
    pthread_t tids[4];

    int i;

    srand(time(NULL));

    nthreads = sizeof(names)/sizeof(names[0]); 

    for (i = 0 ; i < nthreads; i++) 
        pthread_create(tids+i, NULL, barriertest, (void *) names[i]);

    for (i = 0 ; i < nthreads; i++) 
        pthread_join(tids[i], NULL);

    return 0;
}

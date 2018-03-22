#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
/* reader writer problem with POSIX condition variables
   uses cond_broadcast()  (notify all waiters) for
   simplification. Otherwise you need to keep track
   of number of waiters
*/


#define WAIT 100000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canwrite = PTHREAD_COND_INITIALIZER;
pthread_cond_t canread = PTHREAD_COND_INITIALIZER;

int readcount = 0;
int writecount = 0;

void start_read()
{
    pthread_mutex_lock(&mutex);
    while (writecount) { // there is a writer
        pthread_cond_wait(&canread, &mutex);
    }
    readcount++;
    // notify canread here if you don't use broadcast(canread) below
    pthread_mutex_unlock(&mutex);
}

void finish_read()
{
    pthread_mutex_lock(&mutex);
    readcount--;
    if (readcount == 0) // last reader going out
        pthread_cond_signal(&canwrite);
    pthread_mutex_unlock(&mutex);
}

void start_write()
{
    pthread_mutex_lock(&mutex);
    while (readcount || writecount) {
        pthread_cond_wait(&canwrite, &mutex);
    }
    writecount++;
    pthread_mutex_unlock(&mutex);
}

void finish_write()
{
    pthread_mutex_lock(&mutex);
    writecount--;
    // there might be writers and readers, inform them
    pthread_cond_broadcast(&canread);
    pthread_cond_signal(&canwrite);
    pthread_mutex_unlock(&mutex);
}


void *readerwriter(void *p) 
{
    int i;
    char *name = (char *) p;

    for (i = 0; i < 10; i++) {
        usleep(WAIT/100);    /* wait for a while */
        if (rand() % 2) {  /* reader */
            start_read();
            printf("%s started reading, %d\n", p, readcount);
            usleep(rand()% WAIT);
            finish_read();
            printf("%s finished reading, %d\n", p, readcount);
        } else {
            start_write();
            printf("%s started writing\n", p);
            usleep(rand()% WAIT);
            finish_write();
            printf("%s finished writing\n", p);
        }
    }
    return 0;

}



int main() {
    pthread_t ptr,ctr;
    char *names[] = { "A", "B", "C", "D"};
    pthread_t tids[4];

    int i;

    srand(time(NULL));

    for (i = 0 ; i < 4; i++) 
        pthread_create(tids+i, NULL, readerwriter, (void *) names[i]);

    for (i = 0 ; i < 4; i++) 
        pthread_join(tids[i], NULL);

    return 0;
}

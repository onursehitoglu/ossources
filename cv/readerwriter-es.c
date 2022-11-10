#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
/* readerwriter with POSIX condition variables 
   without notifyall(), and fair version, tries to switch
   readers to writers in alternating order  */


#define WAIT 100000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canread = PTHREAD_COND_INITIALIZER;
pthread_cond_t canwrite = PTHREAD_COND_INITIALIZER;

int readcount = 0;
int writecount = 0;
int readwait = 0;   // waiter count for read
int writewait = 0;  // waiter count for write

void start_read()
{
    pthread_mutex_lock(&mutex);
    if  (writecount || writewait) { // second for fairness
        readwait++;
        pthread_cond_wait(&canread, &mutex);
        readwait--;
    }
    readcount++;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&canread); // let other readers enter too
}



void finish_read()
{
    pthread_mutex_lock(&mutex);
    readcount--;
    if (readcount == 0)
        pthread_cond_signal(&canwrite);
    pthread_mutex_unlock(&mutex);
}

void start_write()
{
    pthread_mutex_lock(&mutex);
    if  (writecount || readcount) {   // any reader or writer inside?
        ++writewait;
        pthread_cond_wait(&canwrite, &mutex);
        --writewait;
    }
    writecount = 1;
    pthread_mutex_unlock(&mutex);
}

void finish_write()
{
    pthread_mutex_lock(&mutex);

    writecount = 0;
    if (readwait) // if readers are waiting notify them first
        pthread_cond_signal(&canread);
    else   // no readers waiting, inform writers
        pthread_cond_signal(&canwrite);
    pthread_mutex_unlock(&mutex);
}


void *readerwriter(void *p) 
{
    int i;
    char *name = (char *) p;

    for (i = 0; i < 10; i++) {
        usleep(WAIT/100);    /* wait for a while */
        if (rand() % 2) { /* reader */
            start_read();
            printf("%s started reading, %d\n", name, readcount);
            usleep(rand()% WAIT);
            finish_read();
            printf("%s finished reading, %d\n", name, readcount);
        } else {
            start_write();
            printf("%s started writing\n", name);
            usleep(rand()% WAIT);
            finish_write();
            printf("%s finished writing\n", name);
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

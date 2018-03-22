#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>


#define WAIT 100000

sem_t *swrite, *smutex;
int readcount = 0;

void start_read()
{
    sem_wait(smutex);
    readcount++;
    if (readcount == 1) // transition from 0 reader to first
        sem_wait(swrite);   // request exclusive semaphore
    // others (readcount > 1) enter directly
    sem_post(smutex); 
}

void finish_read()
{
    sem_wait(smutex);
    readcount--;
    if (readcount == 0) // transition from 1 to 0 
        sem_post(swrite);  // release exclusive access
    sem_post(smutex);
    // a writer  may enter now
}

void start_write()
{
    sem_wait(swrite);
}


void finish_write() {
    sem_post(swrite);
}


void *readerwriter(void *p) 
{
    int i;
    char *name = (char *) p;

    for (i = 0; i < 10; i++) {
        usleep(WAIT/100);    /* wait for a while */
        if (rand() % 2) {  /* reader */ 
            start_read();
            printf("%s started reading, reader:%d\n", p, readcount);
            usleep(rand()% WAIT);
            finish_read();
            printf("%s finished reading, reader:%d\n", p,readcount);
        } else {  /* writer */
            start_write();
            printf("%s started writing\n", p);
            usleep(rand()% WAIT);
            printf("%s finished writing\n", p);
            finish_write();
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

    sem_unlink("rdrwrtr_sem_write");
    sem_unlink("rdrwrtr_sem_mutex");
    swrite = sem_open("rdrwrtr_sem_write",O_CREAT,0600,1);
    if (swrite == SEM_FAILED) {
        perror("sfull");
        return -1;
    }
    smutex = sem_open("rdrwrtr_sem_mutex",O_CREAT,0600,1);
    for (i = 0 ; i < 4; i++) 
        pthread_create(tids+i, NULL, readerwriter, (void *) names[i]);

    for (i = 0 ; i < 4; i++) 
        pthread_join(tids[i], NULL);

    return 0;
}

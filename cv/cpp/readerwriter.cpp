#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
#include "monitor.h"

/* reader writer problem with POSIX condition variables
   uses notifyAll()  (notify all waiters) for
   simplification. Otherwise you need to keep track
   of number of waiters
*/


#define WAIT 100000

class ReaderWriter:public Monitor {

    Condition canwrite;
    Condition canread;
    int readcount;
    int writecount;

public:
    ReaderWriter() : canwrite(this), canread(this) {
        readcount = writecount = 0;
    }

    
    void start_read()
    {
        __synchronized__ ;

        while (writecount) { // there is a writer
            canread.wait();
        }
        readcount++;
        // notify canread here if you don't use canread.notifyAll() below
    }
    
    void finish_read()
    {
        __synchronized__ ;

        readcount--;
        if (readcount == 0) // last reader going out
            canwrite.notify();
    }
    
    void start_write()
    {
        __synchronized__ ;

        while (readcount || writecount) {
            canwrite.wait();
        }
        writecount++;
    }
    
    void finish_write()
    {
        __synchronized__ ;

        writecount--;
        // there might be writers and readers, inform them
        canread.notifyAll();
        canwrite.notify();
    }
};

ReaderWriter rwmon; // global reader writer monitor


void *readerwriter(void *p) 
{
    int i;
    char *name = (char *) p;

    for (i = 0; i < 10; i++) {
        usleep(WAIT/100);    /* wait for a while */
        if (rand() % 10 > 4) {  /* reader */
            rwmon.start_read();

            printf("%s started reading\n", p);
            usleep(rand()% WAIT);

            rwmon.finish_read();
            printf("%s finished reading\n", p);
        } else {
            rwmon.start_write();

            printf("%s started writing\n", p);
            usleep(rand()% WAIT);

            rwmon.finish_write();
            printf("%s finished writing\n", p);
        }
    }
    return 0;

}



int main() {
    pthread_t ptr,ctr;
    const char *names[] = { "A", "B", "C", "D"};
    pthread_t tids[4];

    int i;

    srand(time(NULL));

    for (i = 0 ; i < 4; i++) 
        pthread_create(tids+i, NULL, readerwriter, (void *) names[i]);

    for (i = 0 ; i < 4; i++) 
        pthread_join(tids[i], NULL);

    return 0;
}

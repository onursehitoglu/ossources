#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>

#include "monitor.h"

/* readerwriter with POSIX condition variables 
   without notifyall(), and fair version, tries to switch
   readers to writers in alternating order  */


#define WAIT 100000

class ReaderWriter:public Monitor {
    Condition canread;
    Condition canwrite;
    int readcount = 0;
    int writecount = 0;
    int readwait = 0;   // waiter count for read
    int writewait = 0;  // waiter count for write
public:
    ReaderWriter():canread(this),canwrite(this) {
        readcount = readwait = 0;
        writecount = readcount = 0;
    }

    void start_read()
    {
        __synchronized__ ;

        if  (writecount || writewait) { // second for fairness
            readwait++;
            canread.wait();
            readwait--;
        }

        readcount++;
        canread.notify(); // let other readers enter too
    }

    void finish_read()
    {
        __synchronized__ ;

        readcount--;
        if (readcount == 0)
            canwrite.notify();
    }
    
    void start_write()
    {
        __synchronized__ ;

        if  (writecount || readcount) {   // any reader or writer inside?
            ++writewait;
            canwrite.wait();
            --writewait;
        }
        writecount = 1;
    }
    
    void finish_write()
    {
        __synchronized__ ;
    
        writecount = 0;
        if (readwait) // if readers are waiting notify them first
            canread.notify();
        else   // no readers waiting, inform writers
            canwrite.notify();
    }
};


ReaderWriter rwmon;  // reader/writer object


void *readerwriter(void *p) 
{
    int i;
    char *name = (char *) p;

    for (i = 0; i < 10; i++) {
        usleep(WAIT/100);    /* wait for a while */
        if (rand() % 10 > 3) { /* reader  60%*/
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

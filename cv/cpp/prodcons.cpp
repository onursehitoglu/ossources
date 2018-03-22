#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>
#include "monitor.h"

/* edit following to change speeds of producer consumer */
#define PWAIT 100000
#define CWAIT 500000


class ProdCons:public Monitor {
    int count;
    Condition notfull, notempty;
    int max;

    struct list {
	    int val;
	    struct list *next;
    } *head , *tail;

	void addtolist(int v) {
		struct list *p = new list;
	
		p->val = v;
		p->next = NULL;
	
		if (head == NULL)
			head = tail = p;	
		else {
			tail->next = p;
			tail = p;
		}
		count++;
	} 	
	
	int getfromlist() {
		int v;
		struct list *p;
		if (head == NULL) {
			fprintf(stderr, "runtime error\n");
			exit(1);
		}
		v = head->val;
		p = head;
		head = head->next;
		delete p;
		count--;
		return v;
	}
	int empty() { return head == NULL ;}
	int full() { return count >= max ;}
	
	void dump() {
		struct list *p;
		printf("[");
		for (p = head; p != NULL; p = p->next)
			printf("%d ", p->val);
		printf("]\n");
	}

public:
    ProdCons(int m):notfull(this), notempty(this) {
        head = tail = NULL;
        count = 0;
        max = m;
    }

    void enqueue(int v) {
        __synchronized__ ;

        while ( full() ) 
            notfull.wait();
        addtolist(v);
        notempty.notify();
    }

    int dequeue() {
        __synchronized__ ;
        int t;

        while ( empty() ) 
            notempty.wait();

        t = getfromlist();

        notfull.notify();
        return t;
    }
};


void *producer(void *p) 
{
    ProdCons *pcmon = (ProdCons *) p;
	int i;

	for (i = 0; i < 20; i++) {
		usleep(PWAIT);
        pcmon->enqueue(i);
		printf("INS -> %d\n", i);
	}
	return 0;

}

void *consumer(void *p)
{
    ProdCons *pcmon = (ProdCons *) p;
	int i;

	for (i = 0; i < 20; i++) {
		usleep(CWAIT);
		printf("CONS <- %d\n", pcmon->dequeue() );
	}

	return 0;
}


int main() {
	pthread_t ptr,ctr;
    ProdCons pcmon(10);

	pthread_create(&ptr, NULL, producer, (void *) &pcmon);
	pthread_create(&ctr, NULL, consumer, (void *) &pcmon);
	pthread_join(ptr, NULL);
	pthread_join(ctr, NULL);

	return 0;
}

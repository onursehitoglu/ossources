#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>

#define MAX  10

int count = 0;

#define PWAIT 100000
#define CWAIT 500000

pthread_mutex_t mutex;
pthread_cond_t cfull, cempty;

struct list {
    int val;
    struct list *next;
} *head = NULL, *tail = NULL;

void additem(int v) {
    struct list *p = malloc(sizeof(struct list));

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

int getitem() {
    int v;
    struct list *p;
    if (head == NULL) {
        fprintf(stderr, "runtime error\n");
        exit(1);
    }
    v = head->val;
    p = head;
    head = head->next;
    free(p);
    count--;
    return v;
}

int empty() { return head == NULL ;}
int full() { return count >= MAX ;}

void dump() {
    struct list *p;
    printf("[");
    for (p = head; p != NULL; p = p->next)
        printf("%d ", p->val);
    printf("]\n");
}

void *producer(void *p) 
{
    int i;

    for (i = 0; i < 20; i++) {
        usleep(PWAIT);
        pthread_mutex_lock(&mutex);
        while (full()) 
            pthread_cond_wait(&cfull, &mutex); 
        additem(i);
        printf("INS -> %d\n", i);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cempty);
    }
    return 0;

}

void *consumer(void *p)
{
    int i;

    for (i = 0; i < 20; i++) {
        usleep(CWAIT);
        pthread_mutex_lock(&mutex);
        while (empty()) 
            pthread_cond_wait(&cempty, &mutex); 
        printf("CONS <- %d\n", getitem());
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cfull);
    }

    return 0;
}


int main() {
    pthread_t ptr,ctr;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cfull, NULL);
    pthread_cond_init(&cempty, NULL);
    pthread_create(&ptr, NULL, producer, NULL);
    pthread_create(&ctr, NULL, consumer, NULL);
    pthread_join(ptr, NULL);
    pthread_join(ctr, NULL);

    return 0;
}

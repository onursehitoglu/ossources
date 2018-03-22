#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
#include<errno.h>

#define MAX  10

int count = 0;

/* edit following to adjust fast/slow producer slow/fast consumer */
#define PWAIT 100000
#define CWAIT 500000

sem_t *sfull, *smutex, *sempty;

/* Simple linked list queue implementation below */
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

        sem_wait(sempty); // wait until there is an empty slot
        sem_wait(smutex);
        additem(i);
        printf("INS -> %d\n", i);
        sem_post(smutex);
        sem_post(sfull); // inform there is at least one full slot
    }
    return 0;

}

void *consumer(void *p)
{
    int i;

    for (i = 0; i < 20; i++) {
        usleep(CWAIT);
        sem_wait(sfull); // wait until there is a full slot
        sem_wait(smutex); 
        printf("CONS <- %d\n", getitem());
        sem_post(smutex);
        sem_post(sempty); // infor there is an empty slot
    }

    return 0;
}


int main() {
    pthread_t ptr,ctr;
    sem_unlink("prodcons_sem_full");
    sem_unlink("prodcons_sem_mutex");
    sem_unlink("prodcons_sem_empty");
    /* number of full slots is 0 initially */
    sfull = sem_open("prodcons_sem_full",O_CREAT,0600,0);
    if (sfull == SEM_FAILED) {
        perror("sfull");
        return -1;
    }
    /* used as a binary lock, it is 1 */
    smutex = sem_open("prodcons_sem_mutex",O_CREAT,0600,1);
    /* initially MAX empty slots exists in queue */
    sempty = sem_open("prodcons_sem_empty",O_CREAT,0600,MAX);
    pthread_create(&ptr, NULL, producer, NULL);
    pthread_create(&ctr, NULL, consumer, NULL);
    pthread_join(ptr, NULL);
    pthread_join(ctr, NULL);
    sem_close(sfull);
    sem_close(smutex);
    sem_close(sempty);

    return 0;
}

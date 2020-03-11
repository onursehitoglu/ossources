#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

/*
Two different kind of operation:
default: customers keep comming and when seats are full, leave shop
ifdef CUSTBLOCK: generator blocks until a seat is free

#define CUSTBLOCK

*/

int simended = 0;   /* if simulation ended */

int nbarbers;
int nseats;
int tnewcust, tcut;

int *seatstate, *barberstate;

int freeseat(const int *seatstate)
{
    int i;
    for (i = 0; i < nseats; i++) 
        if (seatstate[i] == 0)
            return i;
    return -1;
}

void intervalwait(int t, int w)
/* wait for a random val in interval [t-w,t+w] *10 miliseconds*/
{
    int min = (t <= w) ? 0 : t-w;
    int p = t + w - min;
    usleep( (min + rand() % p ) * 100000);
}


struct clist {
    int cid; /* customer id */
    char seat;
    struct clist *next;
} *seats = NULL;

struct clist *lastseat = NULL;

int onseats = 0;
pthread_mutex_t mut_seats = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_custav = PTHREAD_COND_INITIALIZER;

#ifdef CUSTBLOCK
pthread_cond_t cond_seatfree = PTHREAD_COND_INITIALIZER;
#endif

pthread_mutex_t mut_out = PTHREAD_MUTEX_INITIALIZER;

/* following macro is for mutually exclusive output of independent threads */
#define PRINT(...) pthread_mutex_lock(&mut_out); \
               printf(__VA_ARGS__); \
           pthread_mutex_unlock(&mut_out);

void newcustomer(int id) 
/* add a new customer to end of queue, 
   thread should hold mut_seats before calling this function
*/
{
    if (onseats >= nseats) {     /* seats are full, customer drops */
        PRINT("customer %d leaves shop, no free seats\n", id);
    } else {
        int s;
        /* insert end of the queue */
        struct clist *newcust = malloc(sizeof(struct clist));
        newcust->next = NULL;
        newcust->cid = id;
        s = freeseat(seatstate);
        seatstate[s] = id;
        newcust->seat = s;
        
        if (seats == NULL) {
            seats = lastseat = newcust;
        } else {
            lastseat->next = newcust;
            lastseat = newcust;
        }
        /* increment seat count */
        onseats++;
        PRINT("Customer %d sat on seat %d\n", id, s);
        /* signal a new customer is available to barbers */
        pthread_cond_signal(&cond_custav);
    } 
}

int popcustomer()
/* pop first customer in queue,
   thread should hold mut_seats before calling this function */
{
    if (onseats == 0 || seats == NULL) { /* no customer!! ERROR */
        fprintf(stderr,"no customer waiting, this should not happen\n");
    } else {
        /* delete from start  of the queue */
        int custid;
        struct clist *p;

        p = seats;
        seats = seats->next;
        custid = p->cid;
        seatstate[p->seat] = 0;
        free(p);
        /* increment seat count */
        onseats--;
#ifdef CUSTBLOCK
        pthread_cond_signal(&cond_seatfree);
#endif
        return custid;
    } 
}

void *barber(void *p) 
{
    int id = *(int *) p;    /* id of the barber */

    while (1) {
        int custid;
        pthread_mutex_lock(&mut_seats);
        while (onseats == 0) { 
            /* no customers ready, wait... */
            if (simended) {
                PRINT("barber %d is exitting\n", id);
                pthread_mutex_unlock(&mut_seats);
                return NULL;
            }

            pthread_cond_wait(&cond_custav, &mut_seats);
        }
        custid = popcustomer();
        barberstate[id] = custid;
        pthread_mutex_unlock(&mut_seats); /* finished with seats */
        PRINT("barber %d started cutting hair of customer %d\n", id, custid);
        intervalwait(tcut, 20);
        PRINT("barber %d finished cutting hair of customer %d\n", id, custid);
        barberstate[id] = 0;
    }
}

void *generator(void *p) 
/* generate customers arriving for simulation */
{
    int cid = 100;
    while (!simended) {
        intervalwait(tnewcust,2);
        pthread_mutex_lock(&mut_seats);
#ifdef CUSTBLOCK
        while (onseats >= nseats) {
            PRINT("seats are full, customers blocked\n");
            pthread_cond_wait(&cond_seatfree, &mut_seats);
        }
#endif
        newcustomer(cid);
        pthread_mutex_unlock(&mut_seats);
        cid++;
    }
    PRINT("generator is stopping\n");
}


int main(int argc, char *argv[]) {
    pthread_t *barbers, genth;
    int *bids,i;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s nbarbers nseats tnewcustomer tcut\n",
            argv[0]);

        fprintf(stderr, "Typical run for 3 barbers 4 seats a new customer per 100ms"
            " cut takes arround 400 ms:\n"
            "%s 3 4 10 40\n", argv[0]);
        return 1;
    }
    srand(time(NULL));

    nbarbers = atoi(argv[1]);
    nseats = atoi(argv[2]);
    tnewcust = atoi(argv[3]);
    tcut = atoi(argv[4]);

    if (nbarbers <1 || nseats < 2) {
        fprintf(stderr, "Errors: should have nbabers >= 1, nseats >=2\n");
        return 2;
    }
    barbers = malloc(sizeof(pthread_t)*nbarbers);
    bids = malloc(sizeof(int)*nbarbers);
    seatstate = malloc(nseats*sizeof(int));
    barberstate = malloc(nbarbers*sizeof(int));


	
    for (i = 0; i < nseats; i++) 
		seatstate[i] = 0;

    for (i = 0; i < nbarbers; i++) {
        bids[i] = i;
        barberstate[i] = 0;
        pthread_create(&barbers[i], NULL, barber, (void *) &bids[i]);        
    }
    pthread_create(&genth, NULL, generator, NULL);

    /* run simulation for 30 seconds */
    sleep(30);

    simended = 1;

    pthread_join(genth, NULL);

    for (i = 0; i < nbarbers; i++) {
        pthread_join(barbers[i], NULL);
    }

    return 0;

}

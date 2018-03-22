#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include "monitor.h"

#include <iostream>
using namespace std;

/*
Two different kind of operation:
default: customers keep comming and when seats are full, leave shop
ifdef CUSTBLOCK: generator blocks until a seat is free

#define CUSTBLOCK

*/

void intervalwait(int t, int w)
/* wait for a random val in interval [t-w,t+w] *10 miliseconds*/
{
    int min = (t <= w) ? 0 : t-w;
    int p = t + w - min;
    usleep( (min + rand() % p ) * 100000);
}

int simended = 0;   /* if simulation ended */
int tnewcust, tcut; /* interval for new customer arrivals and time to cut */

class BarberShop:public Monitor {
    int nseats;
    int *seatstat;  // keep states of the seats
    int onseats;    // number of customer sitting (or in queue)
    int nbarbers;
    int *barbers;

    Condition custav;
    
    #ifdef CUSTBLOCK
    Condition seatfree;
    #endif

    struct clist {
        int cid; /* customer id */
        char seat;
        struct clist *next;
    };

    clist *seats, *lastseat; // queue for customer, last and first member
    
    int freeseat()
    {
        int i;
        for (i = 0; i < nseats; i++) 
            if (seatstat[i] == 0)
                return i;
        return -1;
    }
    
public:
    BarberShop(int n, int nb):custav(this)
#ifdef CUSTBLOCK
    ,seatfree(this)
#endif
    {
        seats = lastseat = NULL;  // empty queue
        nseats = n;
        onseats = 0;    // # of seating customers
        seatstat = new int[nseats];
        for (int i = 0; i < nseats; i++) seatstat[i] = 0;       
        nbarbers = nb;
        barbers = new int[nbarbers];
        for (int i = 0; i < nbarbers; i++) barbers[i] = 0;       
    }

    ~BarberShop() { delete [] seatstat; delete [] barbers;}
    
    
    /* add a new customer to end of queue */
    void newcustomer(int id)
    {
        __synchronized__ ;

        if (onseats >= nseats) {     /* seats are full, customer drops */
            #ifdef CUSTBLOCK
            cerr << "seats are full, customers blocked\n";
            while (onseats >= nseats) {
                seatfree.wait();
            }
            #else
            cerr << "customer " << id << "  leaves shop, no free seats\n";
            return;
            #endif
        }
        int s;
        /* insert end of the queue */
        clist *newcust = new clist;
        newcust->next = NULL;
        newcust->cid = id;
        s = freeseat();
        seatstat[s] = id;
        newcust->seat = s;
        
        if (seats == NULL) {
            seats = lastseat = newcust;
        } else {
            lastseat->next = newcust;
            lastseat = newcust;
        }
        /* increment seat count */
        onseats++;
        cerr <<  "Customer " << id << " sat on seat " <<  s << '\n';

        /* signal a new customer is available to barbers */
        custav.notify();
        show();
    }
    
    /* pop first customer in queue*/
    int popcustomer(int barb)
    {
        __synchronized__ ;

        while (onseats == 0) { 
            if (simended) return -1; 

            /* no customers ready, wait... */
            custav.wait();
        }

        /* delete from start  of the queue */
        int custid;
        struct clist *p;
    
        p = seats;
        seats = seats->next;
        custid = p->cid;
        seatstat[p->seat] = 0;
        delete  p;

        /* decrement sitting count */
        onseats--;

    #ifdef CUSTBLOCK
        seatfree.notify();
    #endif

        barbers[barb] = custid;
        cerr << "Barber " << barb << " started cutting hair of " << custid << '\n';

        show();
        return custid;
    } 

    /* barber finished cutting hair of customer */
    void finishcustomer(int barb) {
        __synchronized__ ;

        cerr << "Barber " << barb << " finished cutting hair of " << barbers[barb] << '\n';
        barbers[barb] = 0;
        show();
    }

    /* print current shop state */
    void show() {

        cout << "SEATS: ";
        for (int i = 0; i < nseats; i++)
            cout << " " << seatstat[i] << " ";
        cout << '\n';

        cout << "BARBERS: ";
        for (int i = 0; i < nbarbers; i++)
            cout << " " << barbers[i] << " ";
        cout << '\n';

    }
};

struct BParam {
    BarberShop *bs;
    int bid;
};

void *barber(void *p) 
{
    BParam *bp = (BParam *) p;
    BarberShop *bs = bp->bs;
    int id = bp->bid;


    while (1) {
        int custid;
        custid = bs->popcustomer(id);
        if (custid < 0) break; // simulation is ended

        intervalwait(tcut, 20); // wait a random interval

        bs->finishcustomer(id);
    }
}

void *generator(void *p) 
/* generate customers arriving for simulation */
{
    BarberShop *bs = (BarberShop *) p;

    int cid = 100;
    while (!simended) {
        intervalwait(tnewcust,2);

        bs->newcustomer(cid);

        cid++;
    }
    cerr << "Generator stopped\n";
}


int main(int argc, char *argv[]) {
    pthread_t *barbers, genth;
    int *bids;


    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " nbarbers nseats tnewcustomer tcut\n";

        cerr << "Typical run for 3 barbers 4 seats a new customer per 100ms"
            " cut takes arround 400 ms:\n" << argv[0] << " 3 4 10 40\n";
        return 1;
    }

    int nbarbers = atoi(argv[1]);
    int nseats = atoi(argv[2]);
    BParam *bparams = new BParam[nbarbers] ;
    BarberShop bs(nseats,nbarbers);


    tnewcust = atoi(argv[3]);
    tcut = atoi(argv[4]);

    srand(time(NULL));


    if (nbarbers <1 || nseats < 2) {
        cerr << "Errors: should have nbabers >= 1, nseats >=2\n";
        return 2;
    }

    barbers = new pthread_t[nbarbers];

    for (int i = 0; i < nbarbers; i++) {
        bparams[i].bid = i;
        bparams[i].bs = &bs;

        pthread_create(&barbers[i], NULL, barber, (void *) (bparams + i));
    }

    pthread_create(&genth, NULL, generator, (void *) &bs);

    /* let simulation run for 30 seconds, then mark end of simulation */
    sleep(30);
    simended = 1; // stop generator, barbers will finish remaining customers

    /* join all threads and terminate */

    for (int i = 0; i < nbarbers; i++) {
        pthread_join(barbers[i], NULL);
    }

    pthread_join(genth, NULL);

    delete [] bparams;
    delete [] barbers;

    return 0;
}

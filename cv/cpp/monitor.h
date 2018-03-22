#ifndef __MONITOR_H
#define __MONITOR_H
#include<pthread.h>

//! A base class to help deriving monitor like classes 
class Monitor {
    pthread_mutex_t  mut;   // this will protect the monitor
public:
    Monitor() {
        pthread_mutex_init(&mut, NULL);
    }
    class Condition {
        Monitor *owner;
        pthread_cond_t cond;
    public:
        Condition(Monitor *o) {     // we need monitor ptr to access the mutex
                owner = o;
                pthread_cond_init(&cond, NULL) ; 
        }
        void wait() {  pthread_cond_wait(&cond, &owner->mut);}
        void notify() { pthread_cond_signal(&cond);}
        void notifyAll() { pthread_cond_broadcast(&cond);}
    };
    class Lock {
        Monitor *owner;
    public:
        Lock(Monitor *o) { // we need monitor ptr to access the mutex
            owner = o;
            pthread_mutex_lock(&owner->mut); // lock on creation
        }
        ~Lock() { 
            pthread_mutex_unlock(&owner->mut); // unlock on destruct
        }
        void lock() { pthread_mutex_lock(&owner->mut);}
        void unlock() { pthread_mutex_unlock(&owner->mut);}
    };
};

// when following is used as a local variable the 
// method becomes a monitor method. On constructor
// lock is acquired, when function returns, automatically
// called desctructor unlocks it
#define __synchronized__ Lock mutex(this);

/* USAGE:

class A: public Monitor {   // inherit from Monitor
    Condition cv1, cv2;     // condition varibles
    ...
public:
    A() : cv1(this), cv2(this)  {   // pass "this" to cv constructors
        ...
    }
    void method1() {
        __synchronized__;

        // implement your monitor method. lock is already acquired
        // thanks to macro call above.

        while ( ...) { // I need to wait for an event
            cv1.wait();
        }
        ...

        cv2.notify();
    }  // no need to unlock here, destructor of macro variable does it

    void method2() {
        __synchronized__;

        // method1() // !!!! you should not do that.
    }
};
*/
#endif


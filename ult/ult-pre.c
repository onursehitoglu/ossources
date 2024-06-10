#include<ucontext.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

/*! \desc implements a thread list, each node contains context of a thread */
struct dllist {
	ucontext_t *context;
	int time;	
	struct dllist *prev, *next;
};

/*! \desc ready threads list */
struct dllist *tlist = NULL;

/*! \desc current thread in the list */
struct dllist *curthread;

ucontext_t mainctx;

/*! \desc add context to head of the list */
void addlist(struct dllist **lst, ucontext_t *ctx, int t)
{
	struct dllist *n = malloc(sizeof(struct dllist));
	n->context = ctx;
	n->time = t;
	n->next = *lst;
	n->prev = NULL;
	if (*lst != NULL)
		(*lst)->prev = n;
	*lst = n;
}

/*! \desc delete item from list */
void dellist(struct dllist **head, struct dllist *item)
{
	if (item->prev != NULL)
		item->prev->next = item->next;
	if (item->next != NULL)
		item->next->prev = item->prev;
	if (item == *head)
		*head = item->next;
}

#define forallitems(lst, ptr) \
	 for ( ptr = lst ; ptr != NULL; ptr = ptr->next)


void schedule() {
	struct dllist *nthread;
	ucontext_t *curctx = curthread->context;

	if (curthread->next == NULL)
		if (tlist == NULL) {
			printf("i am the only thread, continuing\n");
			return;
		} else {
			nthread = tlist;
		}
	else
			nthread = curthread->next;

	curctx = curthread->context;
	curthread = nthread;
	swapcontext(curctx, curthread->context);
}

void threadexit() {
	struct dllist *nthread = curthread->next;

	printf("exitting\n");
	if (nthread == NULL)
		if (tlist == curthread)  /* last thread go back to main  */
			nthread = NULL;
		else
			nthread = tlist;
	
	dellist(&tlist, curthread);
    free(curthread->context->uc_stack.ss_sp);
    free(curthread->context);
    free(curthread);
	curthread = nthread;
	if (curthread != NULL) 
		setcontext(curthread->context);
	else {
		signal(SIGALRM, SIG_IGN);
		ualarm(0,0);
		setcontext(&mainctx);
	}
}

ucontext_t *initthread(ucontext_t *base) {
	ucontext_t *nctx;

	nctx = malloc(sizeof(ucontext_t));
	*nctx = *base;

	nctx->uc_link = NULL;
	nctx->uc_stack.ss_sp = malloc(16*1024);
	nctx->uc_stack.ss_size = 16*1024;
	nctx->uc_stack.ss_flags = 0;

	/* add it to threads, not setup yet */
	addlist(&tlist, nctx, 0);
	return nctx;
}

/*! \desc SIGALARM handle working as a scheduler
*/
void preemption(int sig) {
	printf("preempt %p \n",curthread->context);
	schedule();
}

void sample(int id) {
	int i;
	int a; 

	a = 4;
	for (i = 0; i < 1000*1000*500 ;i++) {
		if (i % (1000*1000*100) == 1) {
			printf("id: %d, i: %d a: %d.\n", id, i, a);
		}
		if (i == 100000*id*id) {
			printf("input %d\n", id);
			scanf("%d\n", &a);
		}
		/* do something silly */
		a = (a*321141231+7+id) % 155131;
	}
	threadexit();

}

void other() {
	int i;

	for (i = 0; i < 1000*1000*700; i++) {
		if (i % (1000*1000*100) == 1) {
			printf("other ticks\n");
		}
	}
	threadexit();
}

int main() {
	int firsttime = 1;
	ucontext_t *tr;
	ucontext_t bctx;
	getcontext(&mainctx);
	getcontext(&bctx);
	if (firsttime) {
		firsttime = 0;
		/* ---- */
		tr = initthread(&bctx);
		makecontext(tr, (void (*)()) sample, 1, 1);
		/* ---- */
		tr = initthread(&bctx);
		makecontext(tr,  (void (*)())sample, 1, 2);
		/* ---- */
		tr = initthread(&bctx);
		makecontext(tr, other, 0);
		/* now start threads */
		curthread = tlist;
		signal(SIGALRM, preemption);
		ualarm(500000, 500000); /* send SIGALRM 5 times in a second */
		setcontext(tlist->context);
	}

	printf("all terminated\n");
	return 0;
}


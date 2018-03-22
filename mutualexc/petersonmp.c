#include<stdio.h>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/wait.h>


void plock(int id);
void punlock(int id);

struct shared_data {
	int turn;
	int flag[2];
	int x;
} *sd;
	

int increment(int n)
{
	int i;
	for (i=0; i<1000000; i++) {
		plock(n);
		sd->x = sd->x + 1;
		punlock(n);
	}
	printf("%d finished\n",getpid());
}

int main() {
	int i;
	
	/* create an anonymous memory map, not backed by a real file */
	sd = mmap(NULL, sizeof(struct shared_data), PROT_WRITE|PROT_READ,MAP_SHARED|MAP_ANONYMOUS,-1,0);
	if (sd == NULL)
		return 3;

	/* peterson parameters and increment variable is in mmap region*/
	sd->x = 0;
	sd->turn = 0;
	sd->flag[0] = 0;
	sd->flag[1] = 0;

	for (i=0; i < 2; i++) {
		if (! fork()) {
			return increment(i);
		}
	}
	for (i=0; i < 2; i++) {
		int st;
		wait(&st);
	}
	printf("%d\n", sd->x);
	return 0;
}


void plock(int id) {
	int other = !id;
	
	sd->flag[id] = 1;
	sd->turn = other;
	//asm("mfence");		 // put a memory barier as assembler
	//__sync_synchronize();  // same thing with GCC builtin
	while (sd->flag[other] && sd->turn == other )  {
	};
}

void punlock(int id) {
	sd->flag[id] = 0;
}


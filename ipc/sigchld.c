/*! \file sigchld.c
    \brief forks 3 children and gets their exit codes
           and reports on wait.
           uses a handler for SIGCHLD signal

		   when all children exit together it will loose
		   signals since signals are not queued.
*/
#include<signal.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdlib.h>

void child(int sno)
{
	int stat,cno;
	while ( (cno = waitpid(0, &stat, WNOHANG)) > 0)  { 
            printf("Child %d died or blocked\n",cno);
            printf("Exit code is %d\n",WEXITSTATUS(stat));
    }
}

int main() {
	double t;

	srand(time(NULL));
	if (fork() && fork() && fork() && fork() && fork()) {
		signal(SIGCHLD,child);
		printf("main process waiting forever press CTRL-C after 5 children exitted\n");
		for (;;) sleep(1);
	} else {
        srand(getpid());
		usleep(rand() % 100+100);	

		return(rand()%20);
	}
	return 0;
}

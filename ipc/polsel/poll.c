#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<poll.h>
#include<fcntl.h>
#include<errno.h>

#define PIPE(a) socketpair(AF_UNIX, SOCK_STREAM,PF_UNIX,(a))
/* POLL  VERSION */

/* purpose:
   create multiple processes, reads their
   stdout asynchronously and output */

/* rythm start period repeat message
   writes message on stdout with period
   milliseconds. First message starts after
   start milli seconds. */

char *programs[][10] = // COMMANDS to FORK/EXEC
{ {"./rythm","0","2000","10","GUM",NULL},
  {"./rythm","1000","1000","20","BOM",NULL},
  {"./rythm","1500","500","40","BILI",NULL}
};


int main() {
	int NPROC = sizeof(programs)/sizeof(programs[0]);
	int pipes[NPROC][2];
	int pids[NPROC];
	int active[NPROC]; // keep if pipe is not closed as flag
	int i;
	int activecount = NPROC;

	for (i = 0; i < NPROC; i++) {
		pipe(pipes[i]);  // create NPROC pipes
		active[i] = 1;
	}

	for (i = 0; i < NPROC; i++) {
		if (pids[i] = fork()) { // parent
			close(pipes[i][1]); // close unused end
		} else {
			dup2(pipes[i][1],1);		// dup stdout to pipe write end
			close(pipes[i][0]);			// close read end
			close(pipes[i][1]);			// close dup2()ed end
			if (execv(programs[i][0], programs[i])) {
				perror("execv");
				return -1;
			}
		}
	}

	// only master should be here
	// let us read messages from programs and print:
	while (activecount > 0) {
		int j;
		struct pollfd polls[NPROC]; // file descriptor poll info

		/* setup for poll() call. construct the set
           of file desriptors */
		for (i = 0, j = 0; i < NPROC; i++) 
			if (active[i]) {
				polls[j].fd = pipes[i][0];
				polls[j].events = POLLIN;
				polls[j].revents = 0;  // returned events clear 
				j++;
			}

		poll(polls, activecount, -1); // -1: no timeout
        /* now, polls[j].revents marked for ready descriptors
		   collect and consume all ready file descriptors 
           more than one can be ready. so keep looping for all */
		for (i = 0; i < activecount; i++) {
			int n;
			char mess[1024];
			if (polls[i].revents & POLLIN) { // this fd is ready to read
				/* BLOCKING READ. BUT WE KNOW THAT THERE IS DATA */
				n = read(polls[i].fd, mess, 1024);
				if (n <= 0) {
					active[i] = 0;
					activecount--;
				} else {
					write(1, mess, n);
				}
			}
		}
	}

	return 0;
}
/* 
it works. try with strace ./try3  to see how select works
real	0m20.008s
user	0m0.004s
sys	0m0.004s
*/


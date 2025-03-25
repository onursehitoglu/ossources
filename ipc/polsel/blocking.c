#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>

#define PIPE(a) socketpair(AF_UNIX, SOCK_STREAM,PF_UNIX,(a))
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
		for (i = 0; i < NPROC; i++) {
			int n;
			char mess[1024];
			if (active[i]) {
				/* BLOCKING READ!
                   there might be other pipes ready
                   for reading, but we have to block here*/
				n = read(pipes[i][0], mess, 1024);
				if (n <= 0) {
					active[i] = 0;
					activecount--;
					continue;
				} 
				write(1, mess, n);
			}
		}
	}

	return 0;
}
/* PROBLEM: not responsive! it cannot read ready pipes

CPU usage 0.006 seconds in total
real	0m21.512s
user	0m0.005s
sys	0m0.001s
*/


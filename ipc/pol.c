#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<poll.h>
#include<sys/wait.h>
#include<sys/time.h>
/* This C code illustrates multiplexing multiple
   pipe connection by a single thread.
  
   Server thread will block on a any of two fd's
   by using poll(). 

	message size is fixed to 40 for proper operation with
    stream data .
*/


void server(int p1[], int p2[])
{
	struct pollfd pfd[2] = {{ p1[0], POLLIN, 0}, { p2[0], POLLIN, 0}} ;
	char mess[40];
	int  r, n, i ;

	close(p1[1]); /* unused ends */
	close(p2[1]);

	/* pollfd strcture keeps: file descriptor, requested event, returned event */
	n = 2;

	while (pfd[0].fd >= 0 || pfd[1].fd >= 0) { /* one still open */
		/* clear returned event 
		for  (i = 0; i < n; i++) 
				pfd[i].revents = 0;*/

		poll(pfd, n, 0);  /* no timeout*/
		for (i = 0; i < n; i++) 
			if (pfd[i].revents && POLLIN) {
				r = read(pfd[i].fd, mess, 40);
				if (r == 0) 			/* EOF */
					pfd[i].fd = -1;   /* poll() ignores pollfd item if fd is negative */
				else
					printf("%d: %s\n", i, mess);
			}
	}

}

int child(int p[], int t, char *mess)
{
	int i;
	struct timeval now;

	gettimeofday(&now, NULL);
	srand(now.tv_sec + now.tv_usec + t);
	close(p[0]);	/* unused end */

	/* write mess to pipe with random intervals */
	for (i = 0; i < 200; i += t) {
		usleep(t*(rand()%5)*10000);
		write(p[1], mess, strlen(mess)+1);
	}
	usleep(100000);
	write(p[1],"bye",4);
	close(p[1]);
	return 0;
}


int main() {
	int pipe1[2];
	int pipe2[2];
	int w;

	pipe(pipe1);
	pipe(pipe2);
 	if (fork()) {
		if (fork()) {
			server(pipe1, pipe2);
			wait(&w);
			wait(&w);
		} else {
			return child(pipe1,10,"trololo");
		}
	} else {
			return child(pipe2,5,"lalala");
	}

	return 0;
}

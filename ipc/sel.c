#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/select.h>
#include<sys/wait.h>
#include<sys/time.h>

/* This C code illustrates multiplexing multiple
   pipe connection by a single thread.
  
   Server thread will block on a any of two fd's
   by using select. 

	message size is fixed to 40 for proper operation with
    stream data .
*/


void server(int p1[], int p2[])
{
	fd_set readset;
	char mess[40];
	int m, r;
	int open[2] = {1,1}; /* keep a flag for if pipe is still open */

	close(p1[1]); /* close unused ends */
	close(p2[1]);

	m = ((p1[0] > p2[0]) ? p1[0] : p2[0]) + 1; /* maximum of file descriptors */

	while (open[0] || open[1]) {
		/* following initializes a blocking set for select() */
		FD_ZERO(&readset);
		if (open[0]) FD_SET(p1[0],&readset);
		if (open[1]) FD_SET(p2[0],&readset);

		/* the following code will block until any of them have data to read */
		select(m, &readset, NULL,NULL,NULL);  /* no wset, eset, timeout */

		/* now readset contains the fd's with available data */
		if (FD_ISSET(p1[0], &readset)) {  /* check if first pipe is in readset */
			r = read(p1[0], mess, 40);
			if (r == 0)  	/* EOF */
				open[0] = 0;
			else
				printf("%d: %s\n", 0, mess);

		}

		/* following if is not in else since both may have data to read */
		if (FD_ISSET(p2[0], &readset)) {  /* check if second pipe is in readset */
			r = read(p2[0], mess, 40);
			if ( r == 0 )  /* EOF */
				open[1] = 0;
			else
				printf("%d: %s\n", 1, mess);
		}

	}

}

int child(int p[], int t, char *mess)
/* child() writes mess on pipe with some random intervals */
{
	int i;
	
	struct timeval now;
	gettimeofday(&now, NULL);
	srand(now.tv_sec + now.tv_usec + t); /* randomize */


	close(p[0]); /* unused end */

	for (i = 0; i < 200; i += t) {
		usleep(t*(rand()%10)*10000);
		write(p[1], mess, strlen(mess)+1);
	}
	usleep(100000);
	write(p[1],"bye",4);

	close(p[1]);   /*  On close, the other end reader get EOF */
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

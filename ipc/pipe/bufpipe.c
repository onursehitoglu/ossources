/* this example illustrates the buffering in a pipe
   A typical buffer size ranges from 4K to 60K. When
   pipe buffer is full, writes are blocked. The
   example writes pipe in chunks of given size and
   child reads line by line with delays. 

	usage: bufpipe bufsize cwait
           bufsize: size of the writes
           cwait: child read delay between line reads

   try  ./bufpipe 1024 1
        ./bufpipe 10000 10

	see man fcntl, search for "Changing the capacity of a pipe"
*/
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/fcntl.h>

/* mime.types is a large text file (70K) */
#define INFILE "/etc/mime.types"

int main(int argc, char *argv[]) {
	int fd[2];
	int bufsize;
	int p, cwait;

	if (argc != 3) {
		fprintf(stderr, "usage: %s bufsize cwait\n\tsize of the buffer for writing data to pipe\n\tcwait: child read between lines in ms\n");
		return -1;
	}
	sscanf(argv[1], "%d", &bufsize);
	sscanf(argv[2], "%d", &cwait);

	pipe(fd);
	if (p=fork()) { // parent writes messages on pipe
		char *buf = malloc(bufsize);
		int infd, nread, i = 0;
		infd = open(INFILE, O_RDONLY);
		close(fd[0]);
		while (1) {
			nread = read(infd, buf, bufsize); // read from file
			if (nread <= 0)
				break;
			write(fd[1], buf, nread);		// write to pipe
			i++;
			fprintf(stderr, "chunk %d (%d)is written\n", i, nread);
		}
		close(fd[1]);  // for end of file
		wait(&i);
		fprintf(stderr, "child returned %d\n", WEXITSTATUS(i));
	} else { 
		char line[4096];
		int i = 0, nbytes = 0;
		close(fd[1]);
		dup2(fd[0], 0); // redirect standard input
		close(fd[0]);
		while (1) {	
			usleep(cwait*1000);
			if( fgets(line, 4096, stdin) == NULL)  {// libc buffered input from standard input
				break;
			}
			i++;
			nbytes += strlen(line);
			if (i % 100 == 0) 
				fprintf(stderr, "child read line %d byte: %d\t%s", i, nbytes, line);
			// message read is just ignored
		}
		fprintf(stderr, "child read %d bytes in %d lines\n", nbytes, i);
		return i;
	}

	return 0;
}



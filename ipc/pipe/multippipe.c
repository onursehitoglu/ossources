/* this example illustrates the multireader/writer characteristics
   of pipes. All writers write to same pipe buffer and all readers
   consume from the same pipe buffer. Resulting non-deterministic
   output. Readers can read any order and writers can write in any 
   order.
   usage: multippipe nreaders nwriters

   try ./multippipe 3 1
       ./multippipe 1 3
       ./multippipe 3 3
*/
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {
	char *messages[] = { "hello", "how", "are", "you","this is a long message to read"};
	int p, i ;
	int fd[2];
	int rcount, wcount; // reader and writer count

	if (argc != 3) {
		fprintf(stderr, "usage: %s rcount wcount\n\trcount: reader count, wcount: writer count\n\n", argv[0]);
		return -1;
	}
	sscanf(argv[1], "%d", &rcount);
	sscanf(argv[2], "%d", &wcount);

	pipe(fd);

	// parent starint n writer
	for (i = 0; i < rcount ; i++) {
		char mess[1024];
		int nread, pos = 0;
		if (fork())
			continue; // parent keep looping
		// reader child code here
		close(fd[1]);
		while (1) {
			nread = read(fd[0], mess+pos, 10);
			if (nread == 0) // eof
				break;
			pos += nread;
		}
		mess[pos] = '\0';
		printf("child %d read %s\n", getpid(), mess);
		return 0;
	}
	for (i = 0; i < wcount ; i++) {
		int i;
		if (fork())
			continue; // parent keep looping
		// writer child code here
		close(fd[0]);
		for (i = 0; i < sizeof(messages)/sizeof(messages[0]); i++) {
			usleep(1000);
			write(fd[1], messages[i], strlen(messages[i]));
			printf("child %d wrote %s\n", getpid(), messages[i]);
		}
		close(fd[1]);
		return 0;
	}
	close(fd[0]);
	close(fd[1]);

	for (i = 0; i < rcount + wcount; i++)
		wait(&p);

	return 0;
}



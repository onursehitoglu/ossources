/* this example illustrates the stream characteristics of a
   pipe. If child is slow, the messages will be concatenated,
   even if it is fast, the last message is divided.
   usage: pstream pwait cwait
   pwait: millisecond wait time before writing each message
   cwait: millisecond wait time before reading a message

	try: ./pstream 5 5 ; ./pstream 5 10; ./pstream 5 50;
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
	int pwait, cwait; // wait milliseconds for parent and child before I/O

	if (argc != 3) {
		fprintf(stderr, "usage: %s pwait cwait\n\tpwait: parent wait, cwait: child wait\n\n", argv[0]);
		return -1;
	}
	sscanf(argv[1], "%d", &pwait);
	sscanf(argv[2], "%d", &cwait);

	pipe(fd);
	if (p=fork()) { // parent writes messages on pipe
		close(fd[0]);
		for (i = 0; i < sizeof(messages)/sizeof(messages[0]); i++) {
			usleep(pwait*1000);
			write(fd[1], messages[i], strlen(messages[i]));
		}
		close(fd[1]);  // for end of file
		wait(&i);
		fprintf(stderr, "child returned %d\n", WEXITSTATUS(i));
	} else { 
		char mess[10];
		i = 0;
		close(fd[1]);
		while (1) {	
			usleep(cwait*1000);
			p = read(fd[0],  mess, sizeof(mess));
			if (p <= 0)		// end of file
				break;
			i++;
			mess[p] = '\0'; // for termination
			printf("message(%d): %s\n", i, mess);
		}
		return i;
	}

	return 0;
}



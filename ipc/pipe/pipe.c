/* child: hello, i am child -> parent writes */
#include<unistd.h>
#include<stdio.h>

int main() {

	int p;
	int fd[2];
	char message[80];

	pipe(fd);
	if (p=fork()) {
		close(fd[1]);
		dup2(fd[0],0);
		close(fd[0]);
		while (fgets(message,80,stdin)!=NULL) {
			printf("CHILD: %s",message);
		}
	} else {
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		//write(1,"Hello\n",6);
		printf("Hello\n");
		printf("I'am child %d\n",getpid());
		fflush(stdout);
		close(1);
	}

	return 0;
}



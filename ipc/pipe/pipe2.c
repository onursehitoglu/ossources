
/* child1 /bin/ls | child2 tr, parent waits both
   same as /bin/ls -al | /usr/bin/tr /a-z/ /A-Z/
   on shell */
#include<unistd.h>
#include<stdio.h>

int main() {

	int p1, p2;
	int fd[2];

	pipe(fd);
	if (p1=fork()) {
		if (p2 = fork()) {		// parent
			int c;
			close(fd[0]);
			close(fd[1]);
			wait(&c);
			wait(&c);
		} else {
			close(fd[0]);
			dup2(fd[1],1);
			close(fd[1]);
			execl("/bin/ls","ls","-al",(char *)0);
		}
	} else {
		close(fd[1]);
		dup2(fd[0],0);
		close(fd[0]);
		execl("/usr/bin/tr","tr","/a-z/","/A-Z/",(char *)0);
	}

	return 0;
}



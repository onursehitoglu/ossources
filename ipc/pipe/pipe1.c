/* parent /bin/ls | child tr */
#include<unistd.h>
#include<stdio.h>

int main() {

	int p;
	int fd[2];

	pipe(fd);
	if (p=fork()) {
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		execl("/bin/ls","ls","-al",(char *)0);
	} else {
		close(fd[1]);
		dup2(fd[0],0);
		close(fd[1]);
		execl("/usr/bin/tr","tr","/a-z/","/A-Z/",(char *)0);
	}

	return 0;
}



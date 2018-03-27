/* same as pipe3.c using bidirectional pipes
   SysVR4 Unix has them as default for 
   Linux and BSD requires socketpair() call 
*/
#include<unistd.h>
#include<stdio.h>

#ifdef SYSVR4
#define PIPE(a) pipe(a)
#else
#include<sys/socket.h>
#define PIPE(a) socketpair(AF_UNIX, SOCK_STREAM,PF_UNIX,(a))
#endif

int main() {

	int p,i;
	int fd[2];
	char *messages[]={"3+4\n","5*6\n","10+100-(4*4)\n"};
	char result[100];

	PIPE(fd);
	if (p=fork()) {
		close(fd[1]);
		dup2(fd[0],0);
		dup2(fd[0],2);
		close(fd[0]);
		for (i=0;i<3;i++) {
			fprintf(stderr,messages[i]);
			fflush(stderr);
			if (fgets(result,100,stdin)) {
				printf("%s\n",result);
			}
		}

	} else {
		close(fd[0]);
		dup2(fd[1],1);
		dup2(fd[1],0);
		close(fd[1]);
		execl("/usr/bin/bc","bc","-q",(char *)0);
	}

	return 0;
}



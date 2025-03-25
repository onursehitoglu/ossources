/* parent master -> child slave (bc -q, a simple calculator)
   two pipes one for parent to child, other from child to parent
   parent -> 3+4, 5*6 * ... child bc -q results -> parent
*/
#include<unistd.h>
#include<stdio.h>

int main() {

	int p,i;
	int fd1[2],fd2[2];
	char *messages[]={"3+4\n","5*6\n","10+100-(4*4)\n"};
	char result[100];

	pipe(fd1);
	pipe(fd2);
	if (p=fork()) {
		close(fd1[1]);
		close(fd2[0]);
		dup2(fd2[1],2);
		dup2(fd1[0],0);
		close(fd2[1]);
		close(fd1[0]);
		for (i=0;i<3;i++) {
			fprintf(stderr,"%s",messages[i]);
			fflush(stderr);
			if (fgets(result,100,stdin)) {
				printf("%s\n",result);
			}
		}

	} else {
		close(fd1[0]);
		close(fd2[1]);
		dup2(fd1[1],1);
		dup2(fd2[0],0);
		close(fd1[1]);
		close(fd2[0]);
		execl("/usr/bin/bc","bc","-q",(char *)0);
	}

	return 0;
}



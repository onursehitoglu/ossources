/*! \file shmem.c
    \brief Shared Memory Example
        usage: ./shmem shmemkey nprocs

	 Multiplies two matrixes in parallel.  Max number of rows and columns is 1000.
	 First line of input gives m k n where first matrix is m x k and
	 second one is k x r. The input should follow m*k+k*r
	 floating point numbers representing content of the A and B
	 in rowwise order.

	 Parallel algorithm: Each process i calculates rows:
	 i + t * nproc where t is the iteration number.
	 In other words it calculates row i, i+nproc, i+2*nproc, ...

	 Each row i of result matrix is for j in 0 to r-1
	 c_i,j = sum(t=0..r-1) a_i,t * b_t,j

	 Matrices are kept in shared memory allowing processes
	 to work on same data.

     Note that shared memory remains persistent
     in system. use "ipcrm -M shmemkey" to remove
     it when you are done.
*/
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>


#define MAXDIM 1000
typedef double (*MATPOINTER)[MAXDIM]; /* fixed column size for simplicity */

MATPOINTER A;	/* a pointer to MAXDIM size array */
MATPOINTER B;
MATPOINTER C;



void readmatrix(MATPOINTER a, int m, int n)
{
	int i, j;
	for (i = 0; i < m; i++) 
		for (j = 0; j < n; j++) 
			scanf("%lf", &a[i][j]);
}

void printmatrix(MATPOINTER a, int m, int n)
{
	int i, j;
	for (i = 0; i < m; i++)  {
		for (j = 0; j < n; j++) 
			printf("%10.6f ", a[i][j]);
		printf("\n");
	}
}

void findrow(int k, MATPOINTER a, MATPOINTER b, MATPOINTER c, int m, 
			int n, int r)
{
	int col, t;

	for (col = 0; col < r ; col++) {
		c[k][col] = 0.0;
		for (t = 0; t < n ; t++) 
			c[k][col] += a[k][t] * b[t][col];
	}
}

int main(int argc, char *argv[]) {
	key_t shmkey;
	int shmid;
	int nproc;
	int i, j,m, n, r;	/* dimensions of matrices m x n , n x r -> m x r*/
	int shmemsize;

	if (argc != 3)  {
		fprintf(stderr, "usage: %s shmid nproc\n", argv[0]);
		return 1;
	}
	/* get shmid from first command line parameter */
	shmkey = atoi(argv[1]);
	nproc = atoi(argv[2]);

	scanf("%d %d %d", &m , &n, &r);
	/* 3 matrixes  coloumn dimension is fixed for simplicity */
	shmemsize = (m+n+m)*1000*sizeof(double);

	/* open message queue */
	shmid = shmget(shmkey, shmemsize, IPC_CREAT | 0600); /* owner can r/w */
	if (shmid < 0 ) { 	/* error */
		perror(argv[0]);
		return 2;
	}

	/* parent created the shared memory segment, now attach */

	/* A is the first matrix */
	A = (MATPOINTER) shmat(shmid, NULL, 0);	
	/* B starts m row later */
	B = A+m;
	/* C starts n row later */
	C = B+n;

	readmatrix(A, m, n);
	readmatrix(B, n, r);

	/* now create k process to calculate rows of c in parallel */
	for (i = 0; i < nproc; i++) {
		if (!fork()) {	/* child code */
			/* memory map is inherited no need to reattach */
			/* calculate rows i, i+nproc, i+2*nproc, ... */
			for (j = i; j < r ; j += nproc) 
				findrow(j, A, B, C, m, n, r);
			return 0;
		}
	}
	for (i = 0; i < nproc; i++) { /* wait for all children to term */
		int w;
		wait(&w);
	}
	printmatrix(C, m, r);
	return 0;
}

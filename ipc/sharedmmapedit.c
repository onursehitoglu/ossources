/*!	\file sharedmmapedit.c
	\descr it is a extremely simple editor
      editing the text file given in command line
	  on a mmap()'ed memory area.

      When running on multiple processes on same file
      it will be a shared editor.

      This example uses curses library. In Linux use:
         cc -o sharedmmapedit sharedit.c -lncurses
      in other Unix'es use:
         cc -o sharedmmapedit shared.c -lcurses
   
      usage:
      sharedmmapit filename

*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<curses.h>
#include<errno.h>
#define ROWS 23
#define COLS 80
#define REFRESH 1

char *area;
int row=0,col=0;

void initarea()
/*  Change all 0 bytes to space character if a new file*/
{
	int i;
	for (i=0;i<ROWS*COLS;i++)
		if (area[i]==0)
			area[i]=' ';
}

void refreshscreen()
/* Move all data in the file into window using curses function and refresh */
{
	int i;
	for (i=0;i<ROWS;i++)
		mvwaddnstr(stdscr,i,0,area+i*COLS,COLS);
	move(row,col);			/* recover curses position */
	refresh();			/* redraw the window	*/
}

void handler(int s)
/* SIGALRM handler in order to refresh the screen 
    in order to reflect changes (possibly by another process */
{
	refreshscreen();
	ualarm(REFRESH,0);
	/* some systems alarm handler is cleared, reset it */
	signal(SIGALRM,handler);	
	                                
}

int main(int argc, char *argv[])
{
	int fd;
	int key;

	if (argc != 2) {
		fprintf(stderr,"Usage: sharedit filename\n");
		return -1;
	}
	fd=open(argv[1],O_RDWR|O_CREAT,0600);
	/* open the first argument as a file, create with rw------ if
	   file does not exist */

	lseek(fd,ROWS*COLS,SEEK_SET);	/* Guarantee that file size */
	read(fd,&key,1);		/* is large enough	    */
	lseek(fd,-1,SEEK_CUR);		
	write(fd,&key,1);		
	
	if (fd<0) {
		perror(argv[0]);
		return -2;
	}
	area=mmap(0,ROWS*COLS, PROT_READ|PROT_WRITE, MAP_SHARED,fd,0);
	close(fd);

	if (area==NULL) {
		perror("mmap:");
		return -3;
	}
	initarea();

	/* Curses routines to initialize the interactive window */
	initscr(); cbreak(); noecho();
	keypad(stdscr,TRUE);
	clear();

	/* adjust alarm */
	signal(SIGALRM,handler);
	alarm(REFRESH);

	refreshscreen();
	/* infinite loop until F10 key is pressed */
	while ((key=getch())!=KEY_F(10)) {
		switch (key) {
		case KEY_LEFT:	if (col>0)
					col--;
				break;
		case KEY_RIGHT: if (col<COLS-1)
					col++;
				break;
		case KEY_UP:	if (row>0)
					row--;
				break;
		case KEY_DOWN:	if (row<ROWS-1)
					row++;
				break;
		default:	if (isprint(key)) {
					area[row*COLS+col]=key;
					addch(key);
					if (col<COLS-1) col++;
				}
		}
		/* Cursor position report in the status (bottom) line */
		mvprintw(ROWS,0,"%2d,%2d",col,row);
		move(row,col);
	}

}





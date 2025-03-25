#include<stdio.h>

int main(int argc, char *argv[]) {
	int i, start, period, repeat;
	if (argc != 5) {
		fprintf(stderr, "usage: %s start period repeat mess\n"
                        "start, period and repeat are integers in milliseconds\n");
		return -1;
	}

	if ( sscanf(argv[1], "%d", &start) < 1 || sscanf(argv[2], "%d", &period) < 1 ||
		 sscanf(argv[3], "%d", &repeat) < 1) {
		fprintf(stderr, "cannot convert one or more of the 3 arguments to integer\n");
		return -1;
	}

	usleep(start*1000);
	for (i = 0; i < repeat; i++) {
		printf("%s\n", argv[4]);
		fflush(stdout);
		usleep(period*1000);
	}
	return 0;
}

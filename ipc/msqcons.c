/*! \file msqcons.c
    \brief Message Queue Consumer Example
	reads next message and outputs as a line
        usage: ./msqcons queuekey

	note that queue remains persistent in system. remove
	it via "ipcrm -Q queuekey" after you are done.
*/
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

typedef struct struct_mess {
	long pri;
	char body[0];
} Message;

int main(int argc, char *argv[]) {
	key_t queuekey;
	int queueid;
	Message *msg;

	if (argc != 2)  {
		fprintf(stderr, "usage: %s msqid\n", argv[0]);
		return 1;
	}
	/* get queueid from first command line parameter */
	queuekey = atoi(argv[1]);
	

	/* open message queue */
	queueid = msgget(queuekey, IPC_CREAT | 0600); /* owner can r/w */
	if (queueid < 0 ) { 	/* error */
		perror(argv[0]);
		return 2;
	}

	/* allocate the large enough message struture in advance */
	msg = malloc(sizeof(long)+1024); 

	/* infinite loop to read the queue */
	while (1) {
		int ret;
		/* read next message */
		/* msgrcv(id, message, size , priority, flags) */
		ret = msgrcv(queueid, (void *) msg, 1024, 0L, 0);

		if (ret < 0) {
			perror("Receiving message");
			continue;
		} 
		
		msg->body[ret] = '\0'; /* make sure string terminated */
		/* output message */
		printf("New message: (%ld)", msg->pri);
		fputs(msg->body, stdout);
	}

	return 0;
}

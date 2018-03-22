/*! \file msqprod.c
    \brief Reads stdinp messages and inserts in message queue
      Usage: ./msqprod queuekey

      Note that message queue is  persistent. Use 
      "ipcrm -Q queuekey" to remove it when you're done
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
	int queueid, i;
	char line[1024];
	Message *msg;

	if (argc != 2)  {
		fprintf(stderr, "usage: %s msqid\n", argv[0]);
		return 1;
	}
	/* read queue key from command line parameter*/
	queuekey = atoi(argv[1]);

	/* open queue */
	queueid = msgget(queuekey, IPC_CREAT | 0600); /* owner can r/w */
	if (queueid < 0 ) { 	/* error */
		perror(argv[0]);
		return 2;
	}

	/* allocate a large enough buffer */
	msg = malloc(sizeof(long)+1024);

	/* read a line */
	while (fgets(line, 1024, stdin) != NULL) {
		long pri;
		int len, ret;
		/* first integer is the priority */
		sscanf(line,"%ld %n", &pri, &len);
		/* len is the length of integer part in string */
		msg->pri = pri;
		/* copy remaining part of line to message body */
		strncpy(msg->body, line+len, 1024);

		/* insert message on message queue */
		/* msgsnd(queueid, message, body size, flags) */
		ret = msgsnd(queueid, (void *) msg, 
				strlen(msg->body), 0);

		if (ret < 0) {
			perror("Sending message");
		}
	}
	free(msg);
	return 0;
}

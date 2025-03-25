/*! Advanced example on signals and detailed reporting of
    signals.
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<setjmp.h>
#include<string.h>
#include<sys/wait.h>

jmp_buf divzerbuf, segvbuf;

const char *signalstr(int signal)
{
	switch (signal) {
	case SIGHUP             : return "SIGHUP";              
	case SIGINT             : return "SIGINT";              
	case SIGQUIT            : return "SIGQUIT";
	case SIGILL             : return "SIGILL";              
	case SIGTRAP            : return "SIGTRAP";
	case SIGABRT            : return "SIGABRT";
	case SIGBUS             : return "SIGBUS";              
	case SIGFPE             : return "SIGFPE";              
	case SIGKILL            : return "SIGKILL";
	case SIGUSR1            : return "SIGUSR1";
	case SIGSEGV            : return "SIGSEGV";
	case SIGUSR2            : return "SIGUSR2";
	case SIGPIPE            : return "SIGPIPE";
	case SIGALRM            : return "SIGALRM";
	case SIGTERM            : return "SIGTERM";
	case SIGSTKFLT  : return "SIGSTKFLT";   
	case SIGCHLD            : return "SIGCHLD";
	case SIGCONT            : return "SIGCONT";
	case SIGSTOP            : return "SIGSTOP";
	case SIGTSTP            : return "SIGTSTP";
	case SIGTTIN            : return "SIGTTIN";
	case SIGTTOU            : return "SIGTTOU";
	case SIGURG             : return "SIGURG";              
	case SIGXCPU            : return "SIGXCPU";
	case SIGXFSZ            : return "SIGXFSZ";
	case SIGVTALRM  : return "SIGVTALRM";   
	case SIGPROF            : return "SIGPROF";
	case SIGWINCH   : return "SIGWINCH";    
	case SIGPOLL            : return "SIGPOLL";
	case SIGSYS             : return "SIGSYS";              
	default:	return "UNKNOWN";
	}
}


const char *reasonstr(int signal, int code)
{
	switch(code) {
	case SI_ASYNCNL:        return("SI_ASYNCNL");
	case SI_TKILL:  return("SI_TKILL");
	case SI_SIGIO:  return("SI_SIGIO");
	case SI_ASYNCIO:        return("SI_ASYNCIO");
	case SI_MESGQ:  return("SI_MESGQ");
	case SI_TIMER:  return("SI_TIMER");
	case SI_QUEUE:  return("SI_QUEUE");
	case SI_USER:   return("SI_USER");
	case SI_KERNEL:         return("SI_KERNEL");
	}
	if (signal == SIGILL) 
		switch(code) {
		case ILL_ILLOPC:        return("ILL_ILLOPC");
		case ILL_ILLOPN:        return("ILL_ILLOPN");
		case ILL_ILLADR:        return("ILL_ILLADR"); 
		case ILL_ILLTRP:        return("ILL_ILLTRP"); 
		case ILL_PRVOPC:        return("ILL_PRVOPC"); 
		case ILL_PRVREG:        return("ILL_PRVREG"); 
		case ILL_COPROC:        return("ILL_COPROC"); 
		case ILL_BADSTK :       return("ILL_BADSTK      "); 
		}
	else if (signal == SIGFPE)
		switch(code) {
		case FPE_INTDIV:        return("FPE_INTDIV"); 
		case FPE_INTOVF:        return("FPE_INTOVF"); 
		case FPE_FLTDIV:        return("FPE_FLTDIV"); 
		case FPE_FLTOVF:        return("FPE_FLTOVF"); 
		case FPE_FLTUND:        return("FPE_FLTUND"); 
		case FPE_FLTRES:        return("FPE_FLTRES"); 
		case FPE_FLTINV:        return("FPE_FLTINV"); 
		case FPE_FLTSUB :       return("FPE_FLTSUB"); 
		}
	else if (signal == SIGSEGV)
		switch(code) {
		case SEGV_MAPERR:       return("SEGV_MAPERR"); 
		case SEGV_ACCERR        :       return("SEGV_ACCERR"); 
		}
	else if (signal == SIGBUS)
		switch(code) {
		case BUS_ADRALN:        return("BUS_ADRALN"); 
		case BUS_ADRERR:        return("BUS_ADRERR"); 
		case BUS_OBJERR:        return("BUS_OBJERR"); 
		case BUS_MCEERR_AR:     return("BUS_MCEERR_AR"); 
		case BUS_MCEERR_AO      :       return("BUS_MCEERR_AO"); 
		}
	else if (signal == SIGCHLD)
		switch(code) {
		case CLD_EXITED:        return("CLD_EXITED"); 
		case CLD_KILLED:        return("CLD_KILLED");
		case CLD_DUMPED:        return("CLD_DUMPED");
		case CLD_TRAPPED:       return("CLD_TRAPPED");
		case CLD_STOPPED:       return("CLD_STOPPED");
		case CLD_CONTINUED:     return("CLD_CONTINUED");
		}
	else if (signal == SIGPOLL)
		switch(code) {
		case POLL_IN:   return("POLL_IN");
		case POLL_OUT:  return("POLL_OUT");
		case POLL_MSG:  return("POLL_MSG");
		case POLL_ERR:  return("POLL_ERR");
		case POLL_PRI:  return("POLL_PRI");
		case POLL_HUP   :       return("POLL_HUP");
		}
	return "unknown";
}

void handler(int signo, siginfo_t *info, void *p)
{
	
	printf("signal %s received:\n"
               "	 si_errno %d\n"    /* An errno value */
               "	 si_code %s\n"     /* Signal code */
		,signalstr(signo),
		 info->si_errno,
                 reasonstr(signo, info->si_code));
	if (info->si_code == SI_USER || signo == SIGCHLD) 
		printf(
               "	 si_pid %d\n"      /* Sending process ID */
               "	 si_uid %d\n",     /* Real user ID of sending process */
		 info->si_pid,
		 info->si_uid);
	if (signo == SIGCHLD) 
		printf(
               "	 si_status %d\n"   /* Exit value or signal */
               "	 si_utime %ld\n"    /* User time consumed */
               "	 si_stime %ld\n",   /* System time consumed */
                 info->si_status,
                 info->si_utime,
                 info->si_stime);
	if (signo == SIGFPE || signo == SIGSEGV || signo == SIGBUS ||
		signo == SIGILL) 
		printf(
               "	 si_addr %p\n",     /* Memory location which caused fault */
                 info->si_addr);

	if (signo == SIGSEGV)
		longjmp(segvbuf, 1);
	if (signo == SIGFPE)
		longjmp(divzerbuf, 1);
	if (signo == SIGCHLD) {
		int cwcode;
		wait(&cwcode);
		printf("child code %d\n", cwcode);
	}
}

int main() {
	struct sigaction act;
	char *res;
	char buf[1024];
	int chid;

	memset(&act, 0, sizeof(struct sigaction));

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART | SA_SIGINFO;
	act.sa_sigaction = handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGCHLD, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGFPE, &act, NULL);
	sigaction(SIGALRM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);

	alarm(5);

    printf("some user input to ignore. press Ctrl-D when you are done\n");
	while (res = fgets(buf, 1024, stdin)) {
		printf("%s",res);
	}

	if (!setjmp(divzerbuf)) {
		printf("DBZERO: %d\n", 10/0);
	} else {
		printf("RECOVERED DBZERO\n");
	}

	if (!setjmp(segvbuf)) {
		printf("SEGV: %d\n", * (int *) 0xdead);
	} else {
		printf("RECOVERED SEGV\n");
	}

	if (chid = fork()) {
		printf("forked a child %d\n", chid);
	} else {
		sleep(5);
		return -2;
	}

	if (chid = fork()) {
		printf("forked another child %d to kill in a second\n", chid);
		sleep(1);
		kill(chid, SIGTERM);
	} else {
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_RESTART ;
		act.sa_handler = SIG_DFL;
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGTERM, &act, NULL);
		sigaction(SIGCHLD, &act, NULL);
		sigaction(SIGPIPE, &act, NULL);
		sigaction(SIGSEGV, &act, NULL);
		sigaction(SIGFPE, &act, NULL);
		sigaction(SIGALRM, &act, NULL);
		sigaction(SIGUSR1, &act, NULL);
		sleep(5);
		return -5;
	}

	while (sleep(10)) ;
		

	return 0;

}

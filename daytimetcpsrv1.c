#include	"unp.h"
#include	<time.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<errno.h>
#include	<unistd.h>

#define POWER_OFF 0x55
#define POWER_ON 0x88
static int current_status = POWER_OFF;
static char* power_off_a13= "/sbin/rmmod sun4i_vibrator";
static char* power_on_a13 =	"/sbin/insmod /lib/modules/3.0.8+/sun4i-vibrator.ko"; 



int a13_daemon_init(void)
{
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return(-1);
	else if (pid != 0)
		exit(0);	/* parent goes bye-bye */

	/* child continues */
	setsid();		/* become session leader */

	chdir("/");		/* change working directory */

	umask(0);		/* clear our file mode creation mask */

	return(0);
}


static int a13_system_call(const char *cmdstring)	/* version without signal handling */
{
	pid_t	pid;
	int		status = 0;
	printf("a13_system_call()>>   on cmd :%s\n", cmdstring);

	if (cmdstring == NULL)
		return(1);		/* always a command processor with Unix */


	printf("a13_system_call()>> pid == 0  .. %s", cmdstring);
	status = execl("/bin/sh", "sh", "-c", cmdstring, (char *) 0);


	return(status);
}

#if 1
static int system_with_fork(const char *cmdstring)	/* version without signal handling */
{
	pid_t	pid;
	int		status;
	printf("system()>>   on cmd :%s\n", cmdstring);

	if (cmdstring == NULL)
		return(1);		/* always a command processor with Unix */

	if ( (pid = fork()) < 0) {
		status = -1;	/* probably out of processes */

	} else if (pid == 0) {				/* child */
		printf("system()>> pid == 0  .. %s", cmdstring);
		execl("/bin/sh", "sh", "-c", cmdstring, (char *) 0);
		_exit(127);		/* execl error */

	} else {							/* parent */
		printf("system()>> pid !=0 .. %s", cmdstring);
		while (waitpid(pid, &status, 0) < 0)
			if (errno != EINTR) {
				status = -1; /* error other than EINTR from waitpid() */
				break;
			}
	}

	return(status);
}
#endif

static void action(int required)
{
	if(required == POWER_ON){
		printf("action()>>  power_on\n ");
		//a13_system_call(power_on_a13);
		system_with_fork(power_on_a13);

	}else{
		printf("action()>>  power_off\n ");
		system_with_fork(power_off_a13);

	}

}

static int change_status()
{
	if(current_status == POWER_OFF){

		current_status = POWER_ON;
        action(POWER_ON);
	}else{

		current_status = POWER_OFF;
        action(POWER_OFF);
	}

	return current_status;

}


int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	socklen_t			len;
	struct sockaddr_in	servaddr, cliaddr;
	char				buff[MAXLINE];
	time_t				ticks;
	int status;
    a13_daemon_init();

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(5013);	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA *) &cliaddr, &len);
		printf("connection from %s, port %d\n",
			   Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
			   ntohs(cliaddr.sin_port));

        ticks = time(NULL);
        status = change_status();

		if(status == POWER_ON){

          snprintf(buff, sizeof(buff), "POWER ON %.24s\r\n", ctime(&ticks));
		}else{
          snprintf(buff, sizeof(buff), "POWER OFF %.24s\r\n", ctime(&ticks));

		}
        Write(connfd, buff, strlen(buff));

		Close(connfd);
	}
}

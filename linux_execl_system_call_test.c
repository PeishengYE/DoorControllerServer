#include	<time.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<errno.h>
#include	<unistd.h>
#include <stdio.h>
 static char *cmd = "/home/yep/study/C++_SAM/cpp8_soket/s8/bin/test.sh";
static int system_with_fork(const char *cmdstring)	/* version without signal handling */
{
	pid_t	pid;
	int		status = 888;
	printf("Executing cmd :%s\n", cmdstring);

	if (cmdstring == NULL)
		return(1);		/* always a command processor with Unix */

	if ( (pid = fork()) < 0) {
		status = -1;	/* probably out of processes */

	} else if (pid == 0) {				/* child */
		printf("in child process..\n");
		execl("/bin/sh", "sh", "-c", cmdstring, (char *) 0);
		_exit(123);		/* execl error */

	} else {							/* parent */
		printf("in father process..\n");
		while (waitpid(pid, &status, 0) < 0)
			if (errno != EINTR) {
				status = -1; /* error other than EINTR from waitpid() */
				break;
			}
		printf("in father process.. finishing waiting status:%d\n", status);
	}

	return(status);
}

int main()
{

//static int system_with_fork(const char *cmdstring)	/* version without signal handling */
int status;
 status = system_with_fork(cmd);	/* version without signal handling */
 printf("------\n\n status = %d\n", status);


}

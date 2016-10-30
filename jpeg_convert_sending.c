/* Test shutdown(fd,SHUT_RD) and see what happens */
#include	"unp.h"

#define CMD_CONVERT "/usr/bin/convert ./input.jpg -resize %20 output.jpg"
#define OUTPUT_FILE "output.jpg"

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
#define BUF_SIZE 1024
static void sending_image(int sockfd )
{
	FILE *fp;
	char buf[BUF_SIZE];
	int rest, number_object, file_size;

	if((fp = fopen(OUTPUT_FILE ,"r")) == NULL){
		printf("fopen error");
		return;
	}

	/* the end position */
	if(fseek(fp,0,SEEK_END) != 0)
		printf("fopen error");


	if((file_size = ftell(fp)) == 1L)
		printf("ftell error");

	printf("file_size  = 0x%x\n",file_size);


	/* how many objects we have */
	rest = file_size % BUF_SIZE;
    number_object = file_size/BUF_SIZE;

	printf("number of object  = 0x%x\n",number_object);
	printf("rest of byte  = 0x%x\n",rest);

	/* write the objects */
	rewind(fp);

	//buf_read = rand()%100;
	while((fread(buf,BUF_SIZE,1,fp)) == 1){
			Writen(sockfd, buf, BUF_SIZE);

	}

	/* write the rest data */

	if((fread(buf,rest,1,fp)) != 1){
		printf("fread2 error");
	}else{
         Writen(sockfd, buf, rest);
	}

}
#define SERVER_PORT  8099
int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: jpeg_convert_sending  <IPaddress>");

    system_with_fork(CMD_CONVERT);	/* version without signal handling */

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);		/* chargen server */
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    sending_image(sockfd);

	Close(sockfd);

	exit(0);


}

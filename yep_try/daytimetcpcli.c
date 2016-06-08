#include	"unp.h"
#define DAYTIME 5014

/*
	fclose(in);
	close(s);
	*/



int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	int BUFFER_SIZE=1024;
	FILE *in = fopen(argv[2],"r");
	int len;
	char buffer[1024];



	if (argc != 2)
		err_quit("usage: a.out <IPaddress>");

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(DAYTIME);	/* daytime server */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", argv[1]);

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			err_sys("fputs error");
	}

	/* try to send a file now */
	while((len = fread(buffer,1,sizeof(buffer),in)) > 0) {
	cout.flush(); 
	send(s,buffer,BUFFER_SIZE,0);
	}


	if (n < 0)
		err_sys("read error");

	exit(0);
}

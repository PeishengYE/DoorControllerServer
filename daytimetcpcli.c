#include	"unp.h"
#define DAYTIME 5018
#define SOCKET_NUM (5018)

#define STR_GET_TEMP "get_temp"
#define STR_GET_SWITCH_STATUS "get_switch"
#define STR_SWITCH_ON "switch_on"
#define STR_SWITCH_OFF "switch_off"


int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	if (argc != 3)
		err_quit("usage: a.out <IPaddress>, cmd");

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(SOCKET_NUM);	/* daytime server */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", argv[1]);

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

	n = write(sockfd, argv[2], strlen(argv[2]));
	if( n < 0){
		printf("write socket error");
	}else{

	}


	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			err_sys("fputs error");
	}
	if (n < 0)
		err_sys("read error");
	printf("\n");

	exit(0);
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int	errno;
extern char	*sys_erlist[];

void receiveFile(int fd);

#define QLEN	5

int main(int argc, char *argv[])
{
	struct sockaddr_in	fsin;
	char *service = "9110";
	int	 msock, ssock, alen;
	struct hostent	*phe;
	struct servent	 *pse;
	struct protoent	 *ppe;
	struct sockaddr_in	sin;
	int	 s, type;

	bzero((char *)&sin, sizeof(sin));	
	sin.sin_family = AF_INET;

	if(pse = (struct servent *)getservbyname(service, protocol))
			sin.sin_port= (u_short)pse->s_port;
	else if((sin.sin_port = htons((u_short)atoi(service))) == 0)
	printf("Can't get \"%s\" service entry\n", service);

	if(phe = (struct hostent *)gethostbyname(host))
		bcopy(phe->h_addr, (char *)&sin.sin_addr, phe->h_length);
	else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
		printf("Can't get \"%s\" protocol entry.\n", protocol);

	if((ppe = (struct protoent *)getprotobyname(protocol)) == 0)
		printf("Can't get \"%s\" protocol entry.\n", protocol);

	if(strcmp(protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket(PF_INET, type, ppe->p_proto);
		if(s < 0)
			printf("Cant create socket\n");

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		printf("Can't connect to %s.%s\n", host, service);

	ssock = accept(s, (struct sockaddr *)&sin, &alen);
	if(ssock < 0)
		printf("Accept Failed.\n");
	(void)receiveFile(ssock);
	(void)close(ssock);
}

void receiveFile(fd)
	int fd;
{
	char	buf[BUFSIZE];
	int BUFSIZE=1024;
	int	cc;
	FILE *fp=fopen("sample","w");

	do {
		cc = read(fd, buf, sizeof buf);
		fputs(fp,buf);
	}while(cc > 0);

	fclose(fp);
}





/* My Transmit Program */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	int status=0;
	int portno=atoi(argv[1]);
// Create a socket. Do a "man socket" to get details. These are
// pretty common defaults for domain, type, and protocol
	int s = socket(PF_INET,SOCK_STREAM,0);

	if (s <= 0) {
		printf("server: Socket creation failed.");
		exit(-1);
	}

// Connect to server port
	sockaddr_in address;
	int addressSize = sizeof(sockaddr_in);
	address.sin_family=AF_INET; // using INTERNET V4 domain
	address.sin_port = htons(portno);
	inet_pton(AF_INET,"127.0.0.1",&address.sin_addr); // xlate to internal format

	status = connect(s,(struct sockaddr*) &address, addressSize);

	if (status != 0) {
	printf("client:Connect operation failed, unable to connect to server port.");
	exit(-1);
	}

	int BUFFER_SIZE=1024;
	FILE *in = fopen(argv[2],"r");
	int len;
	char buffer[1024];

	while((len = fread(buffer,1,sizeof(buffer),in)) > 0) {
	cout.flush(); 
	send(s,buffer,BUFFER_SIZE,0);
	}

	fclose(in);
	close(s);
	return 0;
}


#include	"unp.h"
#include	<time.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<errno.h>
#include	<unistd.h>

#define SOCKET_NUM (5018)

#define POWER_OFF 0x55
#define POWER_ON 0x88
static int current_status = POWER_OFF;
//static char* power_off_a13= "/sbin/rmmod sun4i_vibrator";
//static char* power_on_a13 =	"/sbin/insmod /lib/modules/3.0.8+/sun4i-vibrator.ko"; 

static int connected_socket = -1;
static char temp_result[2048];
#define STR_SENSOR_1 "TMP1: "
#define STR_SENSOR_2 "TMP2: "

static char* power_off_a13= "/root/light_off.sh";
static char* power_on_a13 =	"/root/light_on.sh"; 

#define TEST_FILE  "/root/loop.sh"
static char* tmp1="/sys/bus/w1/drivers/w1_slave_driver/28-0416371170ff/w1_slave";
static char* tmp2="/sys/bus/w1/drivers/w1_slave_driver/28-0416380a7aff//w1_slave";
static char* open_error1="error on opening 28-0416380a7aff";
static char* open_error2="error on opening 28-0416371170ff";

static char* read_error1="error on reading 28-0416380a7aff";
static char* read_error2="error on reading 28-0416371170ff";

#define BUF_SIZE (1024)

#define STR_GET_TEMP "get_temp"
#define STR_GET_SWITCH_STATUS "get_switch"
#define STR_SWITCH_ON "switch_on"
#define STR_SWITCH_OFF "switch_off"

#define GET_TEMP  (0x11)
#define GET_SWITCH_STATUS  (0x12)
#define SWITCH_ON (0x13)
#define SWITCH_OFF (0x14)

#define STR_STATUS_SWITCH_ON "switch is on"
#define STR_STATUS_SWITCH_OFF "switch is off"

#define SWITCH_STATUS_FILE "/dev/heater_status"
#define SWITCH_STATUS_FILE_ON "on"
#define SWITCH_STATUS_FILE_OFF "off"
#define ERROR_ON_OPEN_SWITCH_STATUS_FILE "error on opening /dev/heater_status"

static void read_switch_status_file(char *buf)
{

	int fp;
	int read_length;
	fp = open(SWITCH_STATUS_FILE, O_RDONLY);
	if (fp<0){
		strcpy(buf, ERROR_ON_OPEN_SWITCH_STATUS_FILE);
		return;
	}
    printf("read_status file...\n");
	read_length = read(fp, buf, BUF_SIZE);
	if(read_length  < 0 ){
		strcpy(buf, read_error2);
		return;
	}
	if(fp >0 )
	  close(fp);
}


static void get_switch_status_write_socket()
{
	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
    read_switch_status_file(buf);
	char *result;

    printf("get_switch_status_write_socket()>> read status file return:  %s \n", buf);

	if(!(strncmp(buf,SWITCH_STATUS_FILE_ON, 2))){
        printf("get_switch_status_write_socket()>> switch on \n");
		result = STR_STATUS_SWITCH_ON;

	}else if(!(strncmp(buf, SWITCH_STATUS_FILE_OFF, 3))){
        printf("get_switch_status_write_socket()>> switch off \n");
		result = STR_STATUS_SWITCH_OFF;

	}else{
		result = ERROR_ON_OPEN_SWITCH_STATUS_FILE;
        printf("get_switch_status_write_socket()>> error \n");
	}

    Write(connected_socket, result, strlen(result));

}


static void read_temperature_1(char *buf)
{

	int fp;
	int read_length;
	fp = open(tmp1, O_RDONLY);
	if (fp<0){
		strcpy(buf, open_error1);
		return;
	}
    printf("read_temperature_1() read ...\n");
	read_length = read(fp, buf, BUF_SIZE);
	if(read_length  < 0 ){
		strcpy(buf, read_error1);
		return;
	}
	if(fp >0 )
	   close(fp);
}

static void read_temperature_2(char *buf)
{

	int fp;
	int read_length;
	fp = open(tmp2, O_RDONLY);
	if (fp<0){
		strcpy(buf, open_error2);
		return;
	}
    printf("read_temperature_2() read ...\n");
	read_length = read(fp, buf, BUF_SIZE);
	if(read_length  < 0 ){
		strcpy(buf, read_error2);
		return;
	}
	if(fp >0 )
	  close(fp);
}


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



static void read_temp_write_socket()
{
	char buf[BUF_SIZE];
	memset(temp_result, 0, 2048);


   printf("reading from sensor 1..\n");
   memset(buf, 0, BUF_SIZE);
   read_temperature_1(buf);
   strcat(temp_result, STR_SENSOR_1);
   strcat(temp_result, buf);


   printf("reading from sensor 2..\n");
   memset(buf, 0, BUF_SIZE);
   read_temperature_2(buf);
   strcat(temp_result, STR_SENSOR_2);
   strcat(temp_result, buf);

   Write(connected_socket, temp_result, strlen(temp_result));
}


static void change_switch_status(int required)
{
	char *result;
	if(required == POWER_ON){
		printf("change_switch_status()>>  power_on\n ");
		result = STR_STATUS_SWITCH_ON;
		//a13_system_call(power_on_a13);
		system_with_fork(power_on_a13);
        Write(connected_socket, result, strlen(result));

	}else{
		printf("change_switch_status()>>  power_off\n ");
		result = STR_STATUS_SWITCH_OFF;
		system_with_fork(power_off_a13);
        Write(connected_socket, result, strlen(result));

	}

}

static int change_status()
{
	if(current_status == POWER_OFF){

		current_status = POWER_ON;
        change_switch_status(POWER_ON);
	}else{

		current_status = POWER_OFF;
        change_switch_status(POWER_OFF);
	}

	return current_status;

}

static int  analysis_cmd(char *cmd)
{
	if(!(strcmp(cmd, STR_GET_TEMP))){
		return GET_TEMP;

	}else if(!strcmp(cmd, STR_GET_SWITCH_STATUS)){
		return GET_SWITCH_STATUS;

	}else if(!(strcmp(cmd, STR_SWITCH_ON))){
		return SWITCH_ON;

	}else if(!(strcmp(cmd, STR_SWITCH_OFF))){
		return SWITCH_OFF;
	}else{

		return -1;
	}

}


static void make_action(int cmd)
{
	switch(cmd){
       case GET_TEMP:
           read_temp_write_socket();
		   break;
       case GET_SWITCH_STATUS:
           get_switch_status_write_socket();
		   break;
       case SWITCH_ON:
           change_switch_status(POWER_ON);
		   break;
       case SWITCH_OFF:
           change_switch_status(POWER_OFF);
		   break;
	   default:
		printf(" error on reading cmd\n");
	}

}

static void  read_cmd_and_make_action()
{
	char buff[80];
	int read_size;
	int cmd_internal;


	memset(buff, 0, 80);
    read_size = read(connected_socket, buff, 79);
	if(read_size <=0){
		printf("read cmd error from connected_socket\n");
		return -1;
	}

    cmd_internal =  analysis_cmd(buff);

     make_action(cmd_internal);

}


int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	socklen_t			len;
	struct sockaddr_in	servaddr, cliaddr;
	char				buff[MAXLINE];
    a13_daemon_init();

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SOCKET_NUM);	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (SA *) &cliaddr, &len);
		printf("connection from %s, port %d\n",
			   Inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
			   ntohs(cliaddr.sin_port));

		connected_socket = connfd;

		/*
        ticks = time(NULL);
        status = change_status();

		if(status == POWER_ON){

          snprintf(buff, sizeof(buff), "POWER ON %.24s\r\n", ctime(&ticks));
		}else{
          snprintf(buff, sizeof(buff), "POWER OFF %.24s\r\n", ctime(&ticks));

		}
        Write(connfd, buff, strlen(buff));
		*/

        read_cmd_and_make_action();
		Close(connfd);
	}
}

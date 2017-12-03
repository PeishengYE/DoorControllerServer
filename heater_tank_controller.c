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

static int connected_socket = -1;
static char temp_result[2048];
#define STR_SENSOR "Water Tank Temp: "


static char* sensor="/sys/bus/w1/drivers/w1_slave_driver/28-04163711deff/w1_slave";
static int saved_temperature;
static char saved_temperature_str[10];

#define CMD_ON "8"
#define CMD_OFF "0"


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

#define STR_GPIO_SWITCH "/proc/driver/yep_gpio/GPIO_PG9"
#define STR_GPIO_SWITCH_ON "PG9 : 1"
#define STR_GPIO_SWITCH_OFF "PG9 : 0"
#define ERROR_ON_OPEN_SWITCH_STATUS_FILE "error on openning GPIO PG9"


static void read_switch_gpio_file(char *buf)
{

	int fp;
	int read_length;
	fp = open(STR_GPIO_SWITCH, O_RDONLY);
	if (fp<0){
		strcpy(buf, ERROR_ON_OPEN_SWITCH_STATUS_FILE);
		return;
	}
    printf("read_status file...\n");
	read_length = read(fp, buf, BUF_SIZE);
	if(read_length  < 0 ){
		return;
	}
	if(fp >0 )
	  close(fp);
}

static void write_switch_gpio_file(char *buf)
{

	int fp;
	fp = open(STR_GPIO_SWITCH, O_WRONLY);
	if (fp<0){
        printf("write_switch_gpio_file()>> Error on openning GPIO file...\n");
		return;
	}
    printf("write_switch_gpio_file()>> writing GPIO file...\n");
	write(fp, buf, (strlen(buf)+1) );
	close(fp);
}

static void get_switch_status_write_socket()
{
	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
    read_switch_gpio_file(buf);
	char *result;

    printf("get_switch_status_write_socket()>> read GPIO file GPIO_PG9 return:  %s \n", buf);

	if(!(strncmp(buf,STR_GPIO_SWITCH_ON, 7))){
        printf("get_switch_status_write_socket()>> switch on \n");
		result = STR_STATUS_SWITCH_ON;

	}else if(!(strncmp(buf, STR_GPIO_SWITCH_OFF, 7))){
        printf("get_switch_status_write_socket()>> switch off \n");
		result = STR_STATUS_SWITCH_OFF;

	}else{
		result = ERROR_ON_OPEN_SWITCH_STATUS_FILE;
        printf("get_switch_status_write_socket()>> error \n");
	}

    Write(connected_socket, result, strlen(result));

}


static void read_temperature(char *buf)
{

	int fp;
	int read_length;
	fp = open(sensor, O_RDONLY);
	if (fp<0){
		//strcpy(buf, open_error1);
		return;
	}
    printf("read_sensor() ...\n");
	read_length = read(fp, buf, BUF_SIZE);
	if(read_length  < 0 ){
		return;
	}
	if(fp >0 )
	   close(fp);
}

static void check_temperature_value(char *buf)
{
	char OK_STR[10] = "YES";
	//char NO_STR[10] = "NO";
	char TAG_STR[10] = "t=";
	char tmp[10] ;
	char * num;
	char *ret;
	int dec = 0, i, len;

	
	printf("check_temperature_value(()>>  string :\n%s\n", buf);
	ret = strstr(buf, OK_STR);
	if(ret != NULL){
	    ret = strstr(buf, TAG_STR);
	    printf("check_temperature_value(()>> temp :%s\n", ret);

		/* copy only the 5 character about number */
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, ret+2, 5);

		num = tmp;
		len = strlen(num);
		for(i=0; i<len; i++){
			dec = dec * 10 + ( num[i] - '0' );
		}

	    printf("check_temperature_value(()>> temp: %d\n", dec);

		/* only choice the temperatur from 4 degree to 100 degree */
		if((dec > 4000)&&(dec < 100000)){

            saved_temperature = dec;
			memset(saved_temperature_str, 0, sizeof(saved_temperature_str));
			strcpy(saved_temperature_str, tmp);
		}
    }

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
   memset(temp_result, 0, 2048);

   strcat(temp_result, STR_SENSOR);
   strcat(temp_result, saved_temperature_str);


   Write(connected_socket, temp_result, strlen(temp_result));
}

static void  read_temperature_local_internal()
{
	char value[256] ;

	while(1){
		memset(value, 0, sizeof(value));

		read_temperature(value);
		check_temperature_value(value);
		sleep(1);

	}

}
/* YEP inside */
static void read_temperature_local()
{
	    pid_t	childpid;

		if ( (childpid = Fork()) == 0) {	/* child process */
           read_temperature_local_internal();
		}
}

static void change_switch_status(int required)
{
	char *result;
	char tmp[100];
	memset(tmp, 0, sizeof(tmp));

	if(required == POWER_ON){
		printf("change_switch_status()>>  power_on\n ");
		strcpy(tmp, CMD_ON);
        write_switch_gpio_file(tmp);

		result = STR_STATUS_SWITCH_ON;
        Write(connected_socket, result, strlen(result));

	}else{
		printf("change_switch_status()>>  power_off\n ");
		strcpy(tmp, CMD_OFF);
        write_switch_gpio_file(tmp);

		result = STR_STATUS_SWITCH_OFF;
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
	}

    cmd_internal =  analysis_cmd(buff);

     make_action(cmd_internal);

}
static void
sig_chld_yep(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	socklen_t			len;
	struct sockaddr_in	servaddr, cliaddr;
	pid_t				childpid;

    a13_daemon_init();
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SOCKET_NUM);	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	Signal(SIGCHLD, sig_chld_yep);	
    read_temperature_local();
	for ( ; ; ) {

		len = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &len)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}

		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			connected_socket = connfd;
            read_cmd_and_make_action();
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}


}

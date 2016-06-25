FILE_LIST="checkopts daytimetcpcli daytimetcpsrv daytimetcpsrv1 rcvbuf rcvbufset sockopt tcpcli01 tcpcli04 tcpcli06 tcpcli07 tcpcli08 tcpserv01 tcpserv02 tcpserv03 tcpserv04 tcpservselect01"
for each in $FILE_LIST
do
	echo removing $each
	git rm  -f $each
done

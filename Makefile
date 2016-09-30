ARM_TOOLCHAIN=arm-none-linux-gnueabi-
AR=$(ARM_TOOLCHAIN)ar
CC=$(ARM_TOOLCHAIN)gcc
CFLAGS = -g -O -D_REENTRANT -Wall -D__EXTENSIONS__
LIBS= lib/libunp.a -lresolv -lrt -lnsl -lpthread

export AR CC CFLAGS 

LIBDIRS=lib
OUTPUT_DIRS=bin

TCPSERV = tcpserv01 tcpserv02 tcpserv03 tcpserv04
TCPCLI = tcpcli01 tcpcli04
SELECT = tcpcli06 tcpcli07 tcpcli08 tcpservselect01 

SO =	checkopts prdefaults rcvbuf rcvbufset sockopt

PROGS= controller 
    
    
all:$(LIBDIRS)	$(PROGS)

.PHONY : $(LIBDIRS) clean distclean

$(LIBDIRS):
	$(MAKE) -C $@

clean:
	@rm -f *.o
	$(MAKE) -C $(LIBDIRS) clean

distclean: clean
	@rm -f $(PROGS)



controller:	controller.o
	${CC} ${CFLAGS} -o $@ controller.o ${LIBS}

